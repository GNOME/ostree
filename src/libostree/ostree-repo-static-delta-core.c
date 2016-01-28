/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2013 Colin Walters <walters@verbum.org>
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
 */

#include "config.h"

#include "ostree-core-private.h"
#include "ostree-repo-private.h"
#include "ostree-repo-static-delta-private.h"
#include "otutil.h"

gboolean
_ostree_static_delta_parse_checksum_array (GVariant      *array,
                                           guint8       **out_checksums_array,
                                           guint         *out_n_checksums,
                                           GError       **error)
{
  gsize n = g_variant_n_children (array);
  guint n_checksums;

  n_checksums = n / OSTREE_STATIC_DELTA_OBJTYPE_CSUM_LEN;

  if (G_UNLIKELY(n == 0 ||
                 n > (G_MAXUINT32/OSTREE_STATIC_DELTA_OBJTYPE_CSUM_LEN) ||
                 (n_checksums * OSTREE_STATIC_DELTA_OBJTYPE_CSUM_LEN) != n))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Invalid checksum array length %" G_GSIZE_FORMAT, n);
      return FALSE;
    }

  *out_checksums_array = (gpointer)g_variant_get_data (array);
  *out_n_checksums = n_checksums;

  return TRUE;
}


/**
 * ostree_repo_list_static_delta_names:
 * @self: Repo
 * @out_deltas: (out) (element-type utf8) (transfer container): String name of deltas (checksum-checksum.delta)
 * @cancellable: Cancellable
 * @error: Error
 *
 * This function synchronously enumerates all static deltas in the
 * repository, returning its result in @out_deltas.
 */ 
gboolean
ostree_repo_list_static_delta_names (OstreeRepo                  *self,
                                     GPtrArray                  **out_deltas,
                                     GCancellable                *cancellable,
                                     GError                     **error)
{
  gboolean ret = FALSE;
  g_autoptr(GPtrArray) ret_deltas = NULL;
  g_autoptr(GFileEnumerator) dir_enum = NULL;

  ret_deltas = g_ptr_array_new_with_free_func (g_free);

  if (g_file_query_exists (self->deltas_dir, NULL))
    {
      dir_enum = g_file_enumerate_children (self->deltas_dir, OSTREE_GIO_FAST_QUERYINFO,
                                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                            NULL, error);
      if (!dir_enum)
        goto out;

      while (TRUE)
        {
          g_autoptr(GFileEnumerator) dir_enum2 = NULL;
          GFileInfo *file_info;
          GFile *child;

          if (!gs_file_enumerator_iterate (dir_enum, &file_info, &child,
                                           NULL, error))
            goto out;
          if (file_info == NULL)
            break;

          if (g_file_info_get_file_type (file_info) != G_FILE_TYPE_DIRECTORY)
            continue;


          dir_enum2 = g_file_enumerate_children (child, OSTREE_GIO_FAST_QUERYINFO,
                                                 G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                 NULL, error);
          if (!dir_enum2)
            goto out;

          while (TRUE)
            {
              GFileInfo *file_info2;
              GFile *child2;
              const char *name1;
              const char *name2;

              if (!gs_file_enumerator_iterate (dir_enum2, &file_info2, &child2,
                                               NULL, error))
                goto out;
              if (file_info2 == NULL)
                break;

              if (g_file_info_get_file_type (file_info2) != G_FILE_TYPE_DIRECTORY)
                continue;

              name1 = gs_file_get_basename_cached (child);
              name2 = gs_file_get_basename_cached (child2);

              {
                g_autoptr(GFile) meta_path = g_file_get_child (child2, "superblock");

                if (g_file_query_exists (meta_path, NULL))
                  {
                    g_autofree char *buf = g_strconcat (name1, name2, NULL);
                    GString *out = g_string_new ("");
                    char checksum[65];
                    guchar csum[OSTREE_SHA256_DIGEST_LEN];
                    const char *dash = strchr (buf, '-');

                    ostree_checksum_b64_inplace_to_bytes (buf, csum);
                    ostree_checksum_inplace_from_bytes (csum, checksum);
                    g_string_append (out, checksum);
                    if (dash)
                      {
                        g_string_append_c (out, '-');
                        ostree_checksum_b64_inplace_to_bytes (dash+1, csum);
                        ostree_checksum_inplace_from_bytes (csum, checksum);
                        g_string_append (out, checksum);
                      }

                    g_ptr_array_add (ret_deltas, g_string_free (out, FALSE));
                  }
              }
            }
        }
    }

  ret = TRUE;
  gs_transfer_out_value (out_deltas, &ret_deltas);
 out:
  return ret;
}

gboolean
_ostree_repo_static_delta_part_have_all_objects (OstreeRepo             *repo,
                                                 GVariant               *checksum_array,
                                                 gboolean               *out_have_all,
                                                 GCancellable           *cancellable,
                                                 GError                **error)
{
  gboolean ret = FALSE;
  guint8 *checksums_data;
  guint i,n_checksums;
  gboolean have_object = TRUE;

  if (!_ostree_static_delta_parse_checksum_array (checksum_array,
                                                  &checksums_data,
                                                  &n_checksums,
                                                  error))
    goto out;

  for (i = 0; i < n_checksums; i++)
    {
      guint8 objtype = *checksums_data;
      const guint8 *csum = checksums_data + 1;
      char tmp_checksum[65];

      if (G_UNLIKELY(!ostree_validate_structureof_objtype (objtype, error)))
        goto out;

      ostree_checksum_inplace_from_bytes (csum, tmp_checksum);

      if (!ostree_repo_has_object (repo, (OstreeObjectType) objtype, tmp_checksum,
                                   &have_object, cancellable, error))
        goto out;

      if (!have_object)
        break;

      checksums_data += OSTREE_STATIC_DELTA_OBJTYPE_CSUM_LEN;
    }

  ret = TRUE;
  *out_have_all = have_object;
 out:
  return ret;
}

/**
 * ostree_repo_static_delta_execute_offline:
 * @self: Repo
 * @dir_or_file: Path to a directory containing static delta data, or directly to the superblock
 * @skip_validation: If %TRUE, assume data integrity
 * @cancellable: Cancellable
 * @error: Error
 *
 * Given a directory representing an already-downloaded static delta
 * on disk, apply it, generating a new commit.  The directory must be
 * named with the form "FROM-TO", where both are checksums, and it
 * must contain a file named "superblock", along with at least one part.
 */
gboolean
ostree_repo_static_delta_execute_offline (OstreeRepo                    *self,
                                          GFile                         *dir_or_file,
                                          gboolean                       skip_validation,
                                          GCancellable                  *cancellable,
                                          GError                      **error)
{
  gboolean ret = FALSE;
  guint i, n;
  g_autoptr(GFile) meta_file = NULL;
  g_autoptr(GFile) dir = NULL;
  g_autoptr(GVariant) meta = NULL;
  g_autoptr(GVariant) headers = NULL;
  g_autoptr(GVariant) metadata = NULL;
  g_autoptr(GVariant) fallback = NULL;
  g_autofree char *to_checksum = NULL;
  g_autofree char *from_checksum = NULL;
  GFileType file_type;


  file_type = g_file_query_file_type (dir_or_file, 0, cancellable);
  if (file_type == G_FILE_TYPE_DIRECTORY)
    {
      dir = g_object_ref (dir_or_file);
      meta_file = g_file_get_child (dir, "superblock");
    }
  else
    {
      meta_file = g_object_ref (dir_or_file);
      dir = g_file_get_parent (meta_file);
    }

  if (!ot_util_variant_map (meta_file, G_VARIANT_TYPE (OSTREE_STATIC_DELTA_SUPERBLOCK_FORMAT),
                            FALSE, &meta, error))
    goto out;

  /* Parsing OSTREE_STATIC_DELTA_SUPERBLOCK_FORMAT */

  metadata = g_variant_get_child_value (meta, 0);

  /* Write the to-commit object */
  {
    g_autoptr(GVariant) to_csum_v = NULL;
    g_autoptr(GVariant) from_csum_v = NULL;
    g_autoptr(GVariant) to_commit = NULL;
    gboolean have_to_commit;
    gboolean have_from_commit;

    to_csum_v = g_variant_get_child_value (meta, 3);
    if (!ostree_validate_structureof_csum_v (to_csum_v, error))
      goto out;
    to_checksum = ostree_checksum_from_bytes_v (to_csum_v);

    from_csum_v = g_variant_get_child_value (meta, 2);
    if (g_variant_n_children (from_csum_v) > 0)
      {
        if (!ostree_validate_structureof_csum_v (from_csum_v, error))
          goto out;
        from_checksum = ostree_checksum_from_bytes_v (from_csum_v);

        if (!ostree_repo_has_object (self, OSTREE_OBJECT_TYPE_COMMIT, from_checksum,
                                     &have_from_commit, cancellable, error))
          goto out;

        if (!have_from_commit)
          {
            g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                         "Commit %s, which is the delta source, is not in repository", from_checksum);
            goto out;
          }
      }

    if (!ostree_repo_has_object (self, OSTREE_OBJECT_TYPE_COMMIT, to_checksum,
                                 &have_to_commit, cancellable, error))
      goto out;
    
    if (!have_to_commit)
      {
        g_autofree char *detached_path = _ostree_get_relative_static_delta_path (from_checksum, to_checksum, "commitmeta");
        g_autoptr(GVariant) detached_data = NULL;

        detached_data = g_variant_lookup_value (metadata, detached_path, G_VARIANT_TYPE("a{sv}"));
        if (detached_data && !ostree_repo_write_commit_detached_metadata (self,
                                                                          to_checksum,
                                                                          detached_data,
                                                                          cancellable,
                                                                          error))
          goto out;

        to_commit = g_variant_get_child_value (meta, 4);
        if (!ostree_repo_write_metadata (self, OSTREE_OBJECT_TYPE_COMMIT,
                                         to_checksum, to_commit, NULL,
                                         cancellable, error))
          goto out;
      }
  }

  fallback = g_variant_get_child_value (meta, 7);
  if (g_variant_n_children (fallback) > 0)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Cannot execute delta offline: contains nonempty http fallback entries");
      goto out;
    }

  headers = g_variant_get_child_value (meta, 6);
  n = g_variant_n_children (headers);
  for (i = 0; i < n; i++)
    {
      guint32 version;
      guint64 size;
      guint64 usize;
      const guchar *csum;
      gboolean have_all;
      g_autoptr(GBytes) delta_data = NULL;
      g_autoptr(GVariant) part_data = NULL;
      g_autoptr(GVariant) header = NULL;
      g_autoptr(GVariant) csum_v = NULL;
      g_autoptr(GVariant) objects = NULL;
      g_autoptr(GBytes) bytes = NULL;
      g_autofree char *deltapart_path = NULL;

      header = g_variant_get_child_value (headers, i);
      g_variant_get (header, "(u@aytt@ay)", &version, &csum_v, &size, &usize, &objects);

      if (version > OSTREE_DELTAPART_VERSION)
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                       "Delta part has too new version %u", version);
          goto out;
        }

      if (!_ostree_repo_static_delta_part_have_all_objects (self, objects, &have_all,
                                                            cancellable, error))
        goto out;

      /* If we already have these objects, don't bother executing the
       * static delta.
       */
      if (have_all)
        continue;

      csum = ostree_checksum_bytes_peek_validate (csum_v, error);
      if (!csum)
        goto out;

      deltapart_path =
        _ostree_get_relative_static_delta_part_path (from_checksum, to_checksum, i);

      part_data = g_variant_lookup_value (metadata, deltapart_path, G_VARIANT_TYPE("(yay)"));
      if (part_data)
        {
          bytes = g_variant_get_data_as_bytes (part_data);
        }
      else
        {
          g_autoptr(GFile) part_path = ot_gfile_resolve_path_printf (dir, "%u", i);
          GMappedFile *mfile = gs_file_map_noatime (part_path, cancellable, error);
          if (!mfile)
            goto out;

          bytes = g_mapped_file_get_bytes (mfile);
          g_mapped_file_unref (mfile);
        }

      if (!skip_validation)
        {
          g_autoptr(GInputStream) in = g_memory_input_stream_new_from_bytes (bytes);

          g_autofree char *expected_checksum = ostree_checksum_from_bytes (csum);
          if (!_ostree_static_delta_part_validate (self, in, i,
                                                   expected_checksum,
                                                   cancellable, error))
            goto out;
        }

      if (!_ostree_static_delta_part_execute (self, objects, bytes, skip_validation,
                                              cancellable, error))
        {
          g_prefix_error (error, "executing delta part %i: ", i);
          goto out;
        }
    }

  ret = TRUE;
 out:
  return ret;
}
