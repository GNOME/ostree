/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2012 Colin Walters <walters@verbum.org>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Colin Walters <walters@verbum.org>
 */

#include "config.h"

#include "ot-main.h"
#include "ot-admin-builtins.h"
#include "ot-admin-functions.h"
#include "otutil.h"
#include "libgsystem.h"

#include <glib/gi18n.h>

static GOptionEntry options[] = {
  { NULL }
};

gboolean
ot_admin_builtin_init_fs (int argc, char **argv, GCancellable *cancellable, GError **error)
{
  GOptionContext *context;
  gs_unref_object OstreeSysroot *sysroot = NULL;
  gboolean ret = FALSE;
  gs_unref_object GFile *dir = NULL;
  gs_unref_object GFile *child = NULL;
  gs_unref_object OstreeSysroot *target_sysroot = NULL;
  guint i;
  const char *normal_toplevels[] = {"boot", "dev", "home", "proc", "run", "sys"};

  context = g_option_context_new ("PATH - Initialize a root filesystem");

  if (!ostree_admin_option_context_parse (context, options, &argc, &argv,
                                          OSTREE_ADMIN_BUILTIN_FLAG_NONE,
                                          &sysroot, cancellable, error))
    goto out;

  if (argc < 2)
    {
      ot_util_usage_error (context, "PATH must be specified", error);
      goto out;
    }

  dir = g_file_new_for_path (argv[1]);
  target_sysroot = ostree_sysroot_new (dir);

  for (i = 0; i < G_N_ELEMENTS(normal_toplevels); i++)
    {
      child = g_file_get_child (dir, normal_toplevels[i]);
      if (!gs_file_ensure_directory_mode (child, 0755, cancellable, error))
        goto out;
      g_clear_object (&child);
    }

  child = g_file_get_child (dir, "root");
  if (!gs_file_ensure_directory_mode (child, 0700, cancellable, error))
    goto out;
  g_clear_object (&child);

  child = g_file_get_child (dir, "tmp");
  if (!gs_file_ensure_directory_mode (child, 01777, cancellable, error))
    goto out;
  g_clear_object (&child);

  if (!ostree_sysroot_ensure_initialized (target_sysroot, cancellable, error))
    goto out;

  ret = TRUE;
 out:
  if (context)
    g_option_context_free (context);
  return ret;
}
