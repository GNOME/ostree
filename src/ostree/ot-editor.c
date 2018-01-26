/*
 * Copyright (C) 2013 Stef Walter <stefw@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.0+
 *
 * Author: Stef Walter <stefw@redhat.com>
 */

#include "config.h"

#include "libglnx.h"
#include "otutil.h"
#include "ot-editor.h"

#include <sys/wait.h>
#include <string.h>

#ifndef DEFAULT_EDITOR
#define DEFAULT_EDITOR "vi"
#endif

/* Logic pulled from git */

static const char *
get_editor (void)
{
  const char *editor = g_getenv ("OSTREE_EDITOR");
  const char *terminal = g_getenv ("TERM");
  int terminal_is_dumb = !terminal || g_str_equal (terminal, "dumb");

  if (!editor && !terminal_is_dumb)
    editor = g_getenv ("VISUAL");
  if (!editor)
    editor = g_getenv ("EDITOR");

  if (!editor && terminal_is_dumb)
    return NULL;

  if (!editor)
    editor = DEFAULT_EDITOR;

  return editor;
}

char *
ot_editor_prompt (OstreeRepo *repo,
                  const char *input,
                  GCancellable *cancellable,
                  GError **error)
{
  glnx_unref_object GSubprocess *proc = NULL;
  g_autoptr(GFile) file = NULL;
  g_autoptr(GFileIOStream) io = NULL;
  GOutputStream *output;
  const char *editor;
  char *ret = NULL;
  g_autofree char *args = NULL;

  editor = get_editor ();
  if (editor == NULL)
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                           "Terminal is dumb, but EDITOR unset");
      goto out;
    }

  file = g_file_new_tmp (NULL, &io, error);
  if (file == NULL)
    goto out;

  output = g_io_stream_get_output_stream (G_IO_STREAM (io));
  if (!g_output_stream_write_all (output, input, strlen (input), NULL, cancellable, error) ||
      !g_io_stream_close (G_IO_STREAM (io), cancellable, error))
    goto out;

  {
    g_autofree char *quoted_file = g_shell_quote (gs_file_get_path_cached (file));
    args = g_strconcat (editor, " ", quoted_file, NULL);
  }

  proc = g_subprocess_new (G_SUBPROCESS_FLAGS_STDIN_INHERIT, error, 
                           "/bin/sh", "-c", args, NULL);

  if (!g_subprocess_wait_check (proc, cancellable, error))
    {
      g_prefix_error (error, "There was a problem with the editor '%s'", editor);
      goto out;
    }

  ret = glnx_file_get_contents_utf8_at (AT_FDCWD, gs_file_get_path_cached (file), NULL,
                                        cancellable, error);

out:
  if (file)
    (void )g_file_delete (file, NULL, NULL);
  return ret;
}
