/*
 * Copyright (C) 2017 Colin Walters <walters@verbum.org>
 *
 * SPDX-License-Identifier: LGPL-2.0+
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <errno.h>
#include <gio/gunixoutputstream.h>
#include <glib-unix.h>
#include <stdio.h>
#ifdef HAVE_LIBMOUNT
#include <libmount.h>
#endif
#include "otutil.h"
#include <stdbool.h>
#include <sys/statvfs.h>

#include "ostree-cmd-private.h"
#include "ostree-core-private.h"
#include "ostree-mount-util.h"
#include "ostree-sysroot-private.h"
#include "otcore.h"

#ifdef HAVE_LIBMOUNT
typedef FILE OtLibMountFile;
G_DEFINE_AUTOPTR_CLEANUP_FUNC (OtLibMountFile, endmntent)

/* Taken from systemd path-util.c */
static bool
is_path (const char *p)
{
  return !!strchr (p, '/');
}

/* Taken from systemd path-util.c */
static char *
path_kill_slashes (char *path)
{
  char *f, *t;
  bool slash = false;

  /* Removes redundant inner and trailing slashes. Modifies the
   * passed string in-place.
   *
   * For example: ///foo///bar/ becomes /foo/bar
   */

  for (f = path, t = path; *f; f++)
    {
      if (*f == '/')
        {
          slash = true;
          continue;
        }

      if (slash)
        {
          slash = false;
          *(t++) = '/';
        }

      *(t++) = *f;
    }

  /* Special rule, if we are talking of the root directory, a
     trailing slash is good */

  if (t == path && slash)
    *(t++) = '/';

  *t = 0;
  return path;
}

#endif

/* Forcibly enable our internal units, since we detected ostree= on the kernel cmdline */
static gboolean
require_internal_units (const char *normal_dir, const char *early_dir, const char *late_dir,
                        GError **error)
{
#ifdef SYSTEM_DATA_UNIT_PATH
  GCancellable *cancellable = NULL;

  glnx_autofd int normal_dir_dfd = -1;
  if (!glnx_opendirat (AT_FDCWD, normal_dir, TRUE, &normal_dir_dfd, error))
    return FALSE;

  if (!glnx_shutil_mkdir_p_at (normal_dir_dfd, "local-fs.target.requires", 0755, cancellable,
                               error))
    return FALSE;
  if (symlinkat (SYSTEM_DATA_UNIT_PATH "/ostree-remount.service", normal_dir_dfd,
                 "local-fs.target.requires/ostree-remount.service")
      < 0)
    return glnx_throw_errno_prefix (error, "symlinkat");

  if (!glnx_shutil_mkdir_p_at (normal_dir_dfd, "multi-user.target.wants", 0755, cancellable, error))
    return FALSE;
  if (symlinkat (SYSTEM_DATA_UNIT_PATH "/ostree-finalize-staged.path", normal_dir_dfd,
                 "multi-user.target.wants/ostree-finalize-staged.path")
      < 0)
    return glnx_throw_errno_prefix (error, "symlinkat");
  if (symlinkat (SYSTEM_DATA_UNIT_PATH "/ostree-boot-complete.service", normal_dir_dfd,
                 "multi-user.target.wants/ostree-boot-complete.service")
      < 0)
    return glnx_throw_errno_prefix (error, "symlinkat");

  return TRUE;
#else
  return glnx_throw (error, "Not implemented");
#endif
}

static gboolean
write_unit_file (int dir_fd, const char *path, GCancellable *cancellable, GError **error, const char *fmt, ...)
{
  g_auto (GLnxTmpfile) tmpf = {
    0,
  };
  if (!glnx_open_tmpfile_linkable_at (dir_fd, ".", O_WRONLY | O_CLOEXEC, &tmpf, error))
    return FALSE;
  g_autoptr (GOutputStream) outstream = g_unix_output_stream_new (tmpf.fd, FALSE);
  gsize bytes_written;
  va_list args;
  va_start (args, fmt);
  const gboolean r = g_output_stream_vprintf (outstream, &bytes_written, cancellable, error, fmt, args);
  va_end (args);
  if (!r)
    return FALSE;
  if (!g_output_stream_flush (outstream, cancellable, error))
    return FALSE;
  g_clear_object (&outstream);
  /* It should be readable */
  if (!glnx_fchmod (tmpf.fd, 0644, error))
    return FALSE;
  /* Error out if somehow it already exists, that'll help us debug conflicts */
  if (!glnx_link_tmpfile_at (&tmpf, GLNX_LINK_TMPFILE_NOREPLACE, dir_fd, path,
                             error))
    return FALSE;
  return TRUE;
}

/* Generate var.mount */
static gboolean
fstab_generator (const char *ostree_target, const bool is_aboot, const char *normal_dir,
                 const char *early_dir, const char *late_dir, GError **error)
{
#ifdef HAVE_LIBMOUNT
  /* Not currently cancellable, but define a var in case we care later */
  GCancellable *cancellable = NULL;
  /* Some path constants to avoid typos */
  const char *fstab_path = "/etc/fstab";
  const char *var_dst = "/var";
  const char *var_src = OTCORE_RUN_OSTREE_PRIVATE "/var";

  /* Prepare to write to the output unit dir; we use the "normal" dir
   * that overrides /usr, but not /etc.
   */
  glnx_autofd int normal_dir_dfd = -1;
  if (!glnx_opendirat (AT_FDCWD, normal_dir, TRUE, &normal_dir_dfd, error))
    return FALSE;

  /* Generate a unit to unmount var_src */
  if (!write_unit_file (normal_dir_dfd, "ostree-unmount-temp-var.service", cancellable, error,
                        "##\n# Automatically generated by ostree-system-generator\n##\n\n"
                        "[Unit]\n"
                        "Documentation=man:ostree(1)\n"
                        "ConditionPathIsMountPoint=%s\n"
                        "After=var.mount\n"
                        "\n"
                        "[Service]\n"
                        "Type=oneshot\n"
                        "ExecStart=/usr/bin/umount --lazy %s\n",
                        var_src, var_src))
    return FALSE;

  if (!glnx_shutil_mkdir_p_at (normal_dir_dfd, "local-fs.target.wants", 0755, cancellable,
                               error))
    return FALSE;
  if (symlinkat ("../ostree-unmount-temp-var.service", normal_dir_dfd,
             "local-fs.target.wants/ostree-unmount-temp-var.service") < 0)
    return glnx_throw_errno_prefix (error, "symlinkat");

  /* Load /etc/fstab if it exists, and look for a /var mount */
  g_autoptr (OtLibMountFile) fstab = setmntent (fstab_path, "re");
  gboolean found_var_mnt = FALSE;
  if (!fstab)
    {
      if (errno != ENOENT)
        return glnx_throw_errno_prefix (error, "Reading %s", fstab_path);
    }
  else
    {
      /* Parse it */
      struct mntent *me;
      while ((me = getmntent (fstab)))
        {
          g_autofree char *where = g_strdup (me->mnt_dir);
          if (is_path (where))
            path_kill_slashes (where);

          /* We're only looking for /var here */
          if (strcmp (where, var_dst) != 0)
            continue;

          found_var_mnt = TRUE;
          break;
        }
    }

  /* If we found /var, we're done */
  if (found_var_mnt)
    return TRUE;

  /* Generate our bind mount unit */
  if (!write_unit_file (normal_dir_dfd, "var.mount", cancellable, error,
                        "##\n# Automatically generated by ostree-system-generator\n##\n\n"
                        "[Unit]\n"
                        "Documentation=man:ostree(1)\n"
                        "ConditionKernelCommandLine=!systemd.volatile\n"
                        "Before=local-fs.target\n"
                        "\n"
                        "[Mount]\n"
                        "Where=%s\n"
                        "What=%s\n"
                        "Options=bind,slave,shared\n",
                        var_dst, var_src))
    return FALSE;

  /* And ensure it's required; newer systemd will auto-inject fs dependencies
   * via RequiresMountsFor and the like, but on older versions (e.g. CentOS) we
   * need this. It's what the fstab generator does.  And my mother always said,
   * listen to the fstab generator.
   */
  if (!glnx_shutil_mkdir_p_at (normal_dir_dfd, "local-fs.target.requires", 0755, cancellable,
                               error))
    return FALSE;
  if (symlinkat ("../var.mount", normal_dir_dfd, "local-fs.target.requires/var.mount") < 0)
    return glnx_throw_errno_prefix (error, "symlinkat");

  return TRUE;
#else
  return glnx_throw (error, "Not implemented");
#endif
}

/* Implementation of ostree-system-generator */
gboolean
_ostree_impl_system_generator (const char *normal_dir, const char *early_dir, const char *late_dir,
                               GError **error)
{
  /* We conflict with the magic ostree-mount-deployment-var file for ostree-prepare-root.
   * If this file is present, we have nothing to do! */
  if (unlinkat (AT_FDCWD, INITRAMFS_MOUNT_VAR, 0) == 0)
    return TRUE;

  // If we're not booted via ostree, do nothing
  if (!glnx_fstatat_allow_noent (AT_FDCWD, OTCORE_RUN_OSTREE, NULL, 0, error))
    return FALSE;
  if (errno == ENOENT)
    return TRUE;

  g_autofree char *cmdline = read_proc_cmdline ();
  if (!cmdline)
    return glnx_throw (error, "Failed to read /proc/cmdline");

  g_autofree char *ostree_target = NULL;
  gboolean is_aboot = false;
  if (!otcore_get_ostree_target (cmdline, &is_aboot, &ostree_target, error))
    return glnx_prefix_error (error, "Invalid aboot ostree target");

  /* If no `ostree=` karg exists, gracefully no-op.
   * This could happen in CoreOS live environments, where we hackily mock
   * the `ostree=` karg for `ostree-prepare-root.service` specifically, but
   * otherwise that karg doesn't exist on the real command-line. */
  if (!ostree_target)
    return TRUE;

  if (!require_internal_units (normal_dir, early_dir, late_dir, error))
    return FALSE;
  if (!fstab_generator (ostree_target, is_aboot, normal_dir, early_dir, late_dir, error))
    return FALSE;

  return TRUE;
}
