/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2011,2013 Colin Walters <walters@verbum.org>
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

#include "ot-builtins.h"
#include "ot-builtins-common.h"
#include "ostree.h"
#include "otutil.h"

static gboolean opt_disable_fsync;
static gboolean opt_mirror;

#define ARG_EQ(x, y) (g_ascii_strcasecmp(x, y) == 0)
/* create a function to parse the --fsync option, and current parse it the
 * same as --disable-fsync. Allows us to add other things later, and not have
 * a double negative. */
static gboolean opt__fsync(const gchar *option_name,
			   const gchar *value,
			   gpointer data,
			   GError **error)
{
  g_assert(g_str_equal(option_name, "--fsync"));

  if (0) {}
  else if (ARG_EQ(value, "1"))
    opt_disable_fsync = 0;
  else if (ARG_EQ(value, "true"))
    opt_disable_fsync = 0;
  else if (ARG_EQ(value, "yes"))
    opt_disable_fsync = 0;
  else if (ARG_EQ(value, "0"))
    opt_disable_fsync = 1;
  else if (ARG_EQ(value, "false"))
    opt_disable_fsync = 1;
  else if (ARG_EQ(value, "none"))
    opt_disable_fsync = 1;
  else if (ARG_EQ(value, "no"))
    opt_disable_fsync = 1;
  else
    /* do we want to complain here? */
    return 0;


  return 1;
}

static GOptionEntry options[] = {
  { "disable-fsync", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_NONE, &opt_disable_fsync, "Do not invoke fsync()", NULL },
  { "fsync", 0, 0, G_OPTION_ARG_CALLBACK, opt__fsync, "Specify how to invoke fsync()", NULL },
  { "mirror", 0, 0, G_OPTION_ARG_NONE, &opt_mirror, "Write refs suitable for a mirror", NULL },
  { NULL }
};

gboolean
ostree_builtin_pull (int argc, char **argv, OstreeRepo *repo, GCancellable *cancellable, GError **error)
{
  GOptionContext *context;
  gboolean ret = FALSE;
  gs_free char *remote = NULL;
  OstreeRepoPullFlags pullflags = 0;
  GSConsole *console = NULL;
  gs_unref_ptrarray GPtrArray *refs_to_fetch = NULL;
  gs_unref_object OstreeAsyncProgress *progress = NULL;

  context = g_option_context_new ("REMOTE [BRANCH...] - Download data from remote repository");
  g_option_context_add_main_entries (context, options, NULL);

  if (!g_option_context_parse (context, &argc, &argv, error))
    goto out;

  if (argc < 2)
    {
      ot_util_usage_error (context, "REMOTE must be specified", error);
      goto out;
    }

  if (opt_disable_fsync)
    ostree_repo_set_disable_fsync (repo, TRUE);

  if (opt_mirror)
    pullflags |= OSTREE_REPO_PULL_FLAGS_MIRROR;

  if (strchr (argv[1], ':') == NULL)
    {
      remote = g_strdup (argv[1]);
      if (argc > 2)
        {
          int i;
          refs_to_fetch = g_ptr_array_new ();
          for (i = 2; i < argc; i++)
            g_ptr_array_add (refs_to_fetch, argv[i]);
          g_ptr_array_add (refs_to_fetch, NULL);
        }
    }
  else
    {
      char *ref_to_fetch;
      refs_to_fetch = g_ptr_array_new ();
      if (!ostree_parse_refspec (argv[1], &remote, &ref_to_fetch, error))
        goto out;
      /* Transfer ownership */
      g_ptr_array_add (refs_to_fetch, ref_to_fetch);
      g_ptr_array_add (refs_to_fetch, NULL);
    }

  console = gs_console_get ();
  if (console)
    {
      gs_console_begin_status_line (console, "", NULL, NULL);
      progress = ostree_async_progress_new_and_connect (ot_common_pull_progress, console);
    }

  if (!ostree_repo_pull (repo, remote, refs_to_fetch ? (char**)refs_to_fetch->pdata : NULL,
                         pullflags, progress, cancellable, error))
    goto out;

  if (progress)
    ostree_async_progress_finish (progress);

  ret = TRUE;
 out:
  if (console)
    gs_console_end_status_line (console, NULL, NULL);
 
  if (context)
    g_option_context_free (context);
  return ret;
}
