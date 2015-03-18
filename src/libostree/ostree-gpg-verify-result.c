/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2015 Red Hat, Inc.
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

#include "libgsystem.h"

#include "ostree-gpg-verify-result-private.h"

#include <gpgme.h>

/**
 * SECTION: libostree-gpg-verify-result
 * @title: GPG signature verification results
 * @short_description: Inspect detached GPG signatures
 *
 * #OstreeGpgVerifyResult contains verification details for GPG signatures
 * read from a detached #OstreeRepo metadata object.
 *
 * Use ostree_gpg_verify_result_count_all() and
 * ostree_gpg_verify_result_count_valid() to quickly check overall signature
 * validity.
 *
 * Use ostree_gpg_verify_result_lookup() to find a signature by the key ID
 * or fingerprint of the signing key.
 *
 * For more in-depth inspection, such as presenting signature details to the
 * user, pass an array of attribute values to ostree_gpg_verify_result_get()
 * or get all signature details with ostree_gpg_verify_result_get_all().
 */

typedef struct {
  GObjectClass parent_class;
} OstreeGpgVerifyResultClass;

/* This must stay synchronized with the enum declaration. */
static OstreeGpgSignatureAttr all_signature_attrs[] = {
  OSTREE_GPG_SIGNATURE_ATTR_VALID,
  OSTREE_GPG_SIGNATURE_ATTR_SIG_EXPIRED,
  OSTREE_GPG_SIGNATURE_ATTR_KEY_EXPIRED,
  OSTREE_GPG_SIGNATURE_ATTR_KEY_REVOKED,
  OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING,
  OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT,
  OSTREE_GPG_SIGNATURE_ATTR_TIMESTAMP,
  OSTREE_GPG_SIGNATURE_ATTR_EXP_TIMESTAMP,
  OSTREE_GPG_SIGNATURE_ATTR_PUBKEY_ALGO_NAME,
  OSTREE_GPG_SIGNATURE_ATTR_HASH_ALGO_NAME,
  OSTREE_GPG_SIGNATURE_ATTR_USER_NAME,
  OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL
};

static void ostree_gpg_verify_result_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (OstreeGpgVerifyResult,
                         ostree_gpg_verify_result,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                ostree_gpg_verify_result_initable_iface_init))

static gboolean
signature_is_valid (gpgme_signature_t signature)
{
  /* Mimic the way librepo tests for a valid signature, checking both
   * summary and status fields.
   *
   * - VALID summary flag means the signature is fully valid.
   * - GREEN summary flag means the signature is valid with caveats.
   * - No summary but also no error means the signature is valid but
   *   the signing key is not certified with a trusted signature.
   */
  return (signature->summary & GPGME_SIGSUM_VALID) ||
         (signature->summary & GPGME_SIGSUM_GREEN) ||
         (signature->summary == 0 && signature->status == GPG_ERR_NO_ERROR);
}

static gboolean
signing_key_is_revoked (gpgme_signature_t signature)
{
  /* In my testing, GPGME does not set the GPGME_SIGSUM_KEY_REVOKED summary
   * bit on a revoked signing key but rather GPGME_SIGSUM_SYS_ERROR and the
   * status field shows GPG_ERR_CERT_REVOKED.  Turns out GPGME is expecting
   * GPG_ERR_CERT_REVOKED in the validity_reason field which would then set
   * the summary bit.  Unsure if this is a bug, but best check for both. */

  return (signature->summary & GPGME_SIGSUM_KEY_REVOKED) ||
         ((signature->summary & GPGME_SIGSUM_SYS_ERROR) &&
          gpgme_err_code (signature->status) == GPG_ERR_CERT_REVOKED);
}

static void
ostree_gpg_verify_result_finalize (GObject *object)
{
  OstreeGpgVerifyResult *result = OSTREE_GPG_VERIFY_RESULT (object);

  if (result->context != NULL)
    gpgme_release (result->context);

  if (result->details != NULL)
    gpgme_result_unref (result->details);

  G_OBJECT_CLASS (ostree_gpg_verify_result_parent_class)->finalize (object);
}

static gboolean
ostree_gpg_verify_result_initable_init (GInitable     *initable,
                                        GCancellable  *cancellable,
                                        GError       **error)
{
  OstreeGpgVerifyResult *result = OSTREE_GPG_VERIFY_RESULT (initable);
  gpgme_error_t gpg_error;
  gboolean ret = FALSE;

  gpg_error = gpgme_new (&result->context);
  if (gpg_error != GPG_ERR_NO_ERROR)
    {
      _ostree_gpg_error_to_gio_error (gpg_error, error);
      g_prefix_error (error, "Unable to create context: ");
      goto out;
    }

  ret = TRUE;

out:
  return ret;
}

static void
ostree_gpg_verify_result_class_init (OstreeGpgVerifyResultClass *class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (class);
  object_class->finalize = ostree_gpg_verify_result_finalize;
}

static void
ostree_gpg_verify_result_init (OstreeGpgVerifyResult *result)
{
}

static void
ostree_gpg_verify_result_initable_iface_init (GInitableIface *iface)
{
  iface->init = ostree_gpg_verify_result_initable_init;
}

/**
 * ostree_gpg_verify_result_count_all:
 * @result: an #OstreeGpgVerifyResult
 *
 * Counts all the signatures in @result.
 *
 * Returns: signature count
 */
guint
ostree_gpg_verify_result_count_all (OstreeGpgVerifyResult *result)
{
  gpgme_signature_t signature;
  guint count = 0;

  g_return_val_if_fail (OSTREE_IS_GPG_VERIFY_RESULT (result), 0);

  for (signature = result->details->signatures;
       signature != NULL;
       signature = signature->next)
    {
      count++;
    }

  return count;
}

/**
 * ostree_gpg_verify_result_count_valid:
 * @result: an #OstreeGpgVerifyResult
 *
 * Counts only the valid signatures in @result.
 *
 * Returns: valid signature count
 */
guint
ostree_gpg_verify_result_count_valid (OstreeGpgVerifyResult *result)
{
  gpgme_signature_t signature;
  guint count = 0;

  g_return_val_if_fail (OSTREE_IS_GPG_VERIFY_RESULT (result), 0);

  for (signature = result->details->signatures;
       signature != NULL;
       signature = signature->next)
    {
      if (signature_is_valid (signature))
        count++;
    }

  return count;
}

/**
 * ostree_gpg_verify_result_lookup:
 * @result: an #OstreeGpgVerifyResult
 * @key_id: a GPG key ID or fingerprint
 * @out_signature_index: (out): return location for the index of the signature
 *                              signed by @key_id, or %NULL
 *
 * Searches @result for a signature signed by @key_id.  If a match is found,
 * the function returns %TRUE and sets @out_signature_index so that further
 * signature details can be obtained through ostree_gpg_verify_result_get().
 * If no match is found, the function returns %FALSE and leaves
 * @out_signature_index unchanged.
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean
ostree_gpg_verify_result_lookup (OstreeGpgVerifyResult *result,
                                 const gchar *key_id,
                                 guint *out_signature_index)
{
  gs_free char *key_id_upper = NULL;
  gpgme_signature_t signature;
  guint signature_index;
  gboolean ret = FALSE;

  g_return_val_if_fail (OSTREE_IS_GPG_VERIFY_RESULT (result), NULL);
  g_return_val_if_fail (key_id != NULL, NULL);

  /* signature->fpr is always upper-case. */
  key_id_upper = g_ascii_strup (key_id, -1);

  for (signature = result->details->signatures, signature_index = 0;
       signature != NULL;
       signature = signature->next, signature_index++)
    {
      if (signature->fpr == NULL)
        continue;

      if (g_str_has_suffix (signature->fpr, key_id_upper))
        {
          if (out_signature_index != NULL)
            *out_signature_index = signature_index;
          ret = TRUE;
          break;
        }
    }

  return ret;
}

/**
 * ostree_gpg_verify_result_get:
 * @result: an #OstreeGpgVerifyResult
 * @signature_index: which signature to get attributes from
 * @attrs: (array length=n_attrs): Array of requested attributes
 * @n_attrs: Length of the @attrs array
 *
 * Builds a #GVariant tuple of requested attributes for the GPG signature at
 * @signature_index in @result.  See the #OstreeGpgSignatureAttr description
 * for the #GVariantType of each available attribute.
 *
 * It is a programmer error to request an invalid #OstreeGpgSignatureAttr or
 * an invalid @signature_index.  Use ostree_gpg_verify_result_count_all() to
 * find the number of signatures in @result.
 *
 * Returns: a new, floating, #GVariant tuple
 **/
GVariant *
ostree_gpg_verify_result_get (OstreeGpgVerifyResult *result,
                              guint signature_index,
                              OstreeGpgSignatureAttr *attrs,
                              guint n_attrs)
{
  GVariantBuilder builder;
  gpgme_key_t key = NULL;
  gpgme_signature_t signature;
  guint ii;

  g_return_val_if_fail (OSTREE_IS_GPG_VERIFY_RESULT (result), NULL);
  g_return_val_if_fail (attrs != NULL, NULL);
  g_return_val_if_fail (n_attrs > 0, NULL);

  signature = result->details->signatures;
  while (signature != NULL && signature_index > 0)
    {
      signature = signature->next;
      signature_index--;
    }

  g_return_val_if_fail (signature != NULL, NULL);

  /* Lookup the signing key if we need it.  Note, failure to find
   * the key is not a fatal error.  There's an attribute for that
   * (OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING). */
  for (ii = 0; ii < n_attrs; ii++)
    {
      if (attrs[ii] == OSTREE_GPG_SIGNATURE_ATTR_USER_NAME ||
          attrs[ii] == OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL)
        {
          (void) gpgme_get_key (result->context, signature->fpr, &key, 0);
          break;
        }
    }

  g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);

  for (ii = 0; ii < n_attrs; ii++)
    {
      GVariant *child;
      gboolean v_boolean;
      const char *v_string = NULL;

      switch (attrs[ii])
        {
          case OSTREE_GPG_SIGNATURE_ATTR_VALID:
            v_boolean = signature_is_valid (signature);
            child = g_variant_new_boolean (v_boolean);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_SIG_EXPIRED:
            v_boolean = ((signature->summary & GPGME_SIGSUM_SIG_EXPIRED) != 0);
            child = g_variant_new_boolean (v_boolean);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_KEY_EXPIRED:
            v_boolean = ((signature->summary & GPGME_SIGSUM_KEY_EXPIRED) != 0);
            child = g_variant_new_boolean (v_boolean);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_KEY_REVOKED:
            v_boolean = signing_key_is_revoked (signature);
            child = g_variant_new_boolean (v_boolean);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING:
            v_boolean = ((signature->summary & GPGME_SIGSUM_KEY_MISSING) != 0);
            child = g_variant_new_boolean (v_boolean);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT:
            child = g_variant_new_string (signature->fpr);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_TIMESTAMP:
            child = g_variant_new_int64 ((gint64) signature->timestamp);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_EXP_TIMESTAMP:
            child = g_variant_new_int64 ((gint64) signature->exp_timestamp);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_PUBKEY_ALGO_NAME:
            v_string = gpgme_pubkey_algo_name (signature->pubkey_algo);
            child = g_variant_new_string (v_string);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_HASH_ALGO_NAME:
            v_string = gpgme_hash_algo_name (signature->hash_algo);
            child = g_variant_new_string (v_string);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_USER_NAME:
            if (key != NULL && key->uids != NULL)
              v_string = key->uids->name;
            if (v_string == NULL)
              v_string = "[unknown name]";
            child = g_variant_new_string (v_string);
            break;

          case OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL:
            if (key != NULL && key->uids != NULL)
              v_string = key->uids->email;
            if (v_string == NULL)
              v_string = "[unknown email]";
            child = g_variant_new_string (v_string);
            break;

          default:
            g_critical ("Invalid signature attribute (%d)", attrs[ii]);
            g_variant_builder_clear (&builder);
            return NULL;
        }

      g_variant_builder_add_value (&builder, child);
    }

  if (key != NULL)
    gpgme_key_unref (key);

  return g_variant_builder_end (&builder);
}

/**
 * ostree_gpg_verify_result_get_all:
 * @result: an #OstreeGpgVerifyResult
 * @signature_index: which signature to get attributes from
 *
 * Builds a #GVariant tuple of all available attributes for the GPG signature
 * at @signature_index in @result.
 *
 * The child values in the returned #GVariant tuple are ordered to match the
 * #OstreeGpgSignatureAttr enumeration, which means the enum values can be
 * used as index values in functions like g_variant_get_child().  See the
 * #OstreeGpgSignatureAttr description for the #GVariantType of each
 * available attribute.
 *
 * <note>
 *   <para>
 *     The #OstreeGpgSignatureAttr enumeration may be extended in the future
 *     with new attributes, which would affect the #GVariant tuple returned by
 *     this function.  While the position and type of current child values in
 *     the #GVariant tuple will not change, to avoid backward-compatibility
 *     issues <emphasis>please do not depend on the tuple's overall size or
 *     type signature</emphasis>.
 *   </para>
 * </note>
 *
 * It is a programmer error to request an invalid @signature_index.  Use
 * ostree_gpg_verify_result_count_all() to find the number of signatures in
 * @result.
 *
 * Returns: a new, floating, #GVariant tuple
 **/
GVariant *
ostree_gpg_verify_result_get_all (OstreeGpgVerifyResult *result,
                                  guint signature_index)
{
  g_return_val_if_fail (OSTREE_IS_GPG_VERIFY_RESULT (result), NULL);

  return ostree_gpg_verify_result_get (result, signature_index,
                                       all_signature_attrs,
                                       G_N_ELEMENTS (all_signature_attrs));
}

void
_ostree_gpg_error_to_gio_error (gpgme_error_t   gpg_error,
                                GError        **error)
{
  GIOErrorEnum errcode;

  /* XXX This list is incomplete.  Add cases as needed. */

  switch (gpg_error)
    {
      /* special case - shouldn't be here */
      case GPG_ERR_NO_ERROR:
        g_return_if_reached ();

      /* special case - abort on out-of-memory */
      case GPG_ERR_ENOMEM:
        g_error ("%s: %s",
                 gpgme_strsource (gpg_error),
                 gpgme_strerror (gpg_error));

      case GPG_ERR_INV_VALUE:
        errcode = G_IO_ERROR_INVALID_ARGUMENT;
        break;

      default:
        errcode = G_IO_ERROR_FAILED;
        break;
    }

  g_set_error (error, G_IO_ERROR, errcode, "%s: %s",
               gpgme_strsource (gpg_error),
               gpgme_strerror (gpg_error));
}
