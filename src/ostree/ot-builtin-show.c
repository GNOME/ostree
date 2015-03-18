/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2011 Colin Walters <walters@verbum.org>
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
#include "ot-builtins.h"
#include "ot-dump.h"
#include "ostree.h"
#include "otutil.h"

static gboolean opt_print_related;
static char* opt_print_variant_type;
static char* opt_print_metadata_key;
static char* opt_print_detached_metadata_key;
static gboolean opt_raw;

static GOptionEntry options[] = {
  { "print-related", 0, 0, G_OPTION_ARG_NONE, &opt_print_related, "Show the \"related\" commits", NULL },
  { "print-variant-type", 0, 0, G_OPTION_ARG_STRING, &opt_print_variant_type, "Memory map OBJECT (in this case a filename) to the GVariant type string", "TYPE" },
  { "print-metadata-key", 0, 0, G_OPTION_ARG_STRING, &opt_print_metadata_key, "Print string value of metadata key", "KEY" },
  { "print-detached-metadata-key", 0, 0, G_OPTION_ARG_STRING, &opt_print_detached_metadata_key, "Print string value of detached metadata key", "KEY" },
  { "raw", 0, 0, G_OPTION_ARG_NONE, &opt_raw, "Show raw variant data" },
  { NULL }
};

static gboolean
do_print_variant_generic (const GVariantType *type,
                          const char *filename,
                          GError **error)
{
  gboolean ret = FALSE;
  gs_unref_object GFile *f = NULL;
  gs_unref_variant GVariant *variant = NULL;

  f = g_file_new_for_path (filename);

  if (!ot_util_variant_map (f, type, TRUE, &variant, error))
    goto out;

  ot_dump_variant (variant);

  ret = TRUE;
 out:
  return ret;
}

static gboolean
do_print_related (OstreeRepo  *repo,
                  const char *rev,
                  const char *resolved_rev,
                  GError **error)
{
  gboolean ret = FALSE;
  const char *name;
  gs_unref_variant GVariant *csum_v = NULL;
  gs_unref_variant GVariant *variant = NULL;
  gs_unref_variant GVariant *related = NULL;
  GVariantIter *viter = NULL;

  if (!ostree_repo_load_variant (repo, OSTREE_OBJECT_TYPE_COMMIT,
                                 resolved_rev, &variant, error))
    goto out;
      
  /* PARSE OSTREE_SERIALIZED_COMMIT_VARIANT */
  related = g_variant_get_child_value (variant, 2);
  
  viter = g_variant_iter_new (related);

  while (g_variant_iter_loop (viter, "(&s@ay)", &name, &csum_v))
    {
      gs_free char *checksum = ostree_checksum_from_bytes_v (csum_v);
      g_print ("%s %s\n", name, checksum);
    }
  csum_v = NULL;

  ret = TRUE;
 out:
  if (viter)
    g_variant_iter_free (viter);
  return ret;
}

static gboolean
do_print_metadata_key (OstreeRepo     *repo,
                       const char     *resolved_rev,
                       gboolean        detached,
                       const char     *key,
                       GError        **error)
{
  gboolean ret = FALSE;
  gs_unref_variant GVariant *value = NULL;
  gs_unref_variant GVariant *commit = NULL;
  gs_unref_variant GVariant *metadata = NULL;

  if (!detached)
    {
      if (!ostree_repo_load_variant (repo, OSTREE_OBJECT_TYPE_COMMIT,
                                     resolved_rev, &commit, error))
        goto out;
      /* PARSE OSTREE_SERIALIZED_COMMIT_VARIANT */
      metadata = g_variant_get_child_value (commit, 0);
    }
  else
    {
      if (!ostree_repo_read_commit_detached_metadata (repo, resolved_rev, &metadata,
                                                      NULL, error))
        goto out;
      if (metadata == NULL)
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                       "No detached metadata for commit %s", resolved_rev);
          goto out;
        }
    }
  
  value = g_variant_lookup_value (metadata, key, NULL);
  if (!value)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND,
                   "No such metadata key '%s'", key);
      goto out;
    }

  ot_dump_variant (value);

  ret = TRUE;
 out:
  return ret;
}

static void
print_signature (OstreeGpgVerifyResult *result,
                 guint signature_index)
{
  g_autoptr(GVariant) variant = NULL;
  g_autoptr(GDateTime) date_time_utc = NULL;
  g_autoptr(GDateTime) date_time_local = NULL;
  g_autoptr(gchar) formatted_date_time = NULL;
  gint64 timestamp;
  gint64 exp_timestamp;
  const char *fingerprint;
  const char *pubkey_algo;
  const char *user_name;
  const char *user_email;
  const char *key_id;
  gboolean valid;
  gboolean sig_expired;
  gboolean key_missing;
  gsize len;

  /* This function roughly mimics the verify output generated by
   * check_sig_and_print() in gnupg/g10/mainproc.c, though obviously
   * greatly simplified. */

  variant = ostree_gpg_verify_result_get_all (result, signature_index);

  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_VALID,
                       "b", &valid);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_SIG_EXPIRED,
                       "b", &sig_expired);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING,
                       "b", &key_missing);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT,
                       "&s", &fingerprint);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_TIMESTAMP,
                       "x", &timestamp);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_EXP_TIMESTAMP,
                       "x", &exp_timestamp);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_PUBKEY_ALGO_NAME,
                       "&s", &pubkey_algo);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_USER_NAME,
                       "&s", &user_name);
  g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL,
                       "&s", &user_email);

  len = strlen (fingerprint);
  key_id = (len > 16) ? fingerprint + len - 16 : fingerprint;

  date_time_utc = g_date_time_new_from_unix_utc (timestamp);
  date_time_local = g_date_time_to_local (date_time_utc);
  formatted_date_time = g_date_time_format (date_time_local, "%c");

  g_print ("  Signature made %s using %s key ID %s\n",
           formatted_date_time, pubkey_algo, key_id);

  g_clear_pointer (&date_time_utc, g_date_time_unref);
  g_clear_pointer (&date_time_local, g_date_time_unref);
  g_clear_pointer (&formatted_date_time, g_free);

  if (key_missing)
    g_print ("  Can't check signature: public key not found\n");
  else if (valid)
    g_print ("  Good signature from \"%s <%s>\"\n", user_name, user_email);
  else if (sig_expired)
    g_print ("  Expired signature from \"%s <%s>\"\n", user_name, user_email);
  else
    g_print ("  BAD signature from \"%s <%s>\"\n", user_name, user_email);

  if (exp_timestamp > 0)
    {
      date_time_utc = g_date_time_new_from_unix_utc (exp_timestamp);
      date_time_local = g_date_time_to_local (date_time_utc);
      formatted_date_time = g_date_time_format (date_time_local, "%c");

      if (sig_expired)
        g_print ("  Signature expired %s\n", formatted_date_time);
      else
        g_print ("  Signature expires %s\n", formatted_date_time);
    }
}

static gboolean
print_object (OstreeRepo          *repo,
              OstreeObjectType     objtype,
              const char          *checksum,
              GError             **error)
{
  gs_unref_variant GVariant *variant = NULL;
  OstreeDumpFlags flags = OSTREE_DUMP_NONE;
  gboolean ret = FALSE;

  if (!ostree_repo_load_variant (repo, objtype, checksum,
                                 &variant, error))
    goto out;
  if (opt_raw)
    flags |= OSTREE_DUMP_RAW;
  ot_dump_object (objtype, checksum, variant, flags);

  if (objtype == OSTREE_OBJECT_TYPE_COMMIT)
    {
      gs_unref_object OstreeGpgVerifyResult *result = NULL;
      GError *local_error = NULL;

      result = ostree_repo_verify_commit_ext (repo, checksum,
                                              NULL, NULL, NULL,
                                              &local_error);

      if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
        {
          g_clear_error (&local_error);
        }
      else if (local_error != NULL)
        {
          g_propagate_error (error, local_error);
          goto out;
        }
      else
        {
          guint n_sigs, ii;

          n_sigs = ostree_gpg_verify_result_count_all (result);
          g_print ("Found %u signature%s:\n", n_sigs, n_sigs == 1 ? "" : "s");

          for (ii = 0; ii < n_sigs; ii++)
            {
              g_print ("\n");
              print_signature (result, ii);
            }
        }
    }

  ret = TRUE;
out:
  return ret;
}

static gboolean
print_if_found (OstreeRepo        *repo,
                OstreeObjectType   objtype,
                const char        *checksum,
                gboolean          *inout_was_found,
                GCancellable      *cancellable,
                GError           **error)
{
  gboolean ret = FALSE;
  gboolean have_object = FALSE;

  if (*inout_was_found)
    return TRUE;

  if (!ostree_repo_has_object (repo, objtype, checksum, &have_object,
                               cancellable, error))
    goto out;
  if (have_object)
    {
      if (!print_object (repo, objtype, checksum, error))
        goto out;
      *inout_was_found = TRUE;
    }
  
  ret = TRUE;
 out:
  return ret;
}

gboolean
ostree_builtin_show (int argc, char **argv, GCancellable *cancellable, GError **error)
{
  GOptionContext *context;
  gs_unref_object OstreeRepo *repo = NULL;
  gboolean ret = FALSE;
  const char *rev;
  gs_free char *resolved_rev = NULL;

  context = g_option_context_new ("OBJECT - Output a metadata object");

  if (!ostree_option_context_parse (context, options, &argc, &argv, OSTREE_BUILTIN_FLAG_NONE, &repo, cancellable, error))
    goto out;

  if (argc <= 1)
    {
      ot_util_usage_error (context, "An object argument is required", error);
      goto out;
    }
  rev = argv[1];

  if (opt_print_metadata_key || opt_print_detached_metadata_key)
    {
      gboolean detached = opt_print_detached_metadata_key != NULL;
      const char *key = detached ? opt_print_detached_metadata_key : opt_print_metadata_key;
      if (!ostree_repo_resolve_rev (repo, rev, FALSE, &resolved_rev, error))
        goto out;

      if (!do_print_metadata_key (repo, resolved_rev, detached, key, error))
        goto out;
    }
  else if (opt_print_related)
    {
      if (!ostree_repo_resolve_rev (repo, rev, FALSE, &resolved_rev, error))
        goto out;

      if (!do_print_related (repo, rev, resolved_rev, error))
        goto out;
    }
  else if (opt_print_variant_type)
    {
      if (!do_print_variant_generic (G_VARIANT_TYPE (opt_print_variant_type), rev, error))
        goto out;
    }
  else
    {
      gboolean found = FALSE;
      if (!ostree_validate_checksum_string (rev, NULL))
        {
          if (!ostree_repo_resolve_rev (repo, rev, FALSE, &resolved_rev, error))
            goto out;
          if (!print_object (repo, OSTREE_OBJECT_TYPE_COMMIT, resolved_rev, error))
            goto out;
        }
      else
        {
          if (!print_if_found (repo, OSTREE_OBJECT_TYPE_COMMIT, rev,
                               &found, cancellable, error))
            goto out;
          if (!print_if_found (repo, OSTREE_OBJECT_TYPE_DIR_META, rev,
                               &found, cancellable, error))
            goto out;
          if (!print_if_found (repo, OSTREE_OBJECT_TYPE_DIR_TREE, rev,
                               &found, cancellable, error))
            goto out;
          if (!found)
            {
              gs_unref_object GFileInfo *finfo = NULL;
              gs_unref_variant GVariant *xattrs = NULL;
              GFileType filetype;
              
              if (!ostree_repo_load_file (repo, rev, NULL, &finfo, &xattrs,
                                          cancellable, error))
                goto out;

              g_print ("Object: %s\nType: %s\n", rev, ostree_object_type_to_string (OSTREE_OBJECT_TYPE_FILE));
              filetype = g_file_info_get_file_type (finfo);
              g_print ("File Type: ");
              switch (filetype)
                {
                case G_FILE_TYPE_REGULAR:
                  g_print ("regular\n");
                  g_print ("Size: %" G_GUINT64_FORMAT "\n", g_file_info_get_size (finfo));
                  break;
                case G_FILE_TYPE_SYMBOLIC_LINK:
                  g_print ("symlink\n");
                  g_print ("Target: %s\n", g_file_info_get_symlink_target (finfo));
                  break;
                default:
                  g_printerr ("(unknown type %u)\n", (guint)filetype);
                }

              g_print ("Mode: 0%04o\n", g_file_info_get_attribute_uint32 (finfo, "unix::mode"));
              g_print ("Uid: %u\n", g_file_info_get_attribute_uint32 (finfo, "unix::uid"));
              g_print ("Gid: %u\n", g_file_info_get_attribute_uint32 (finfo, "unix::gid"));

              g_print ("Extended Attributes: ");
              if (xattrs)
                {
                  gs_free char *xattr_string = g_variant_print (xattrs, TRUE);
                  g_print ("{ %s }\n", xattr_string);
                }
              else
                {
                  g_print ("(none)\n");
                }
            }
        }
    }
 
  ret = TRUE;
 out:
  if (context)
    g_option_context_free (context);
  return ret;
}
