/*
 * Copyright (C) 2011 Colin Walters <walters@verbum.org>
 * Copyright (C) 2013 Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 *
 * SPDX-License-Identifier: LGPL-2.0+
 *
 * Author: Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 */

//#pragma once

#include "ostree-gpg-verify-result.h"

G_BEGIN_DECLS

#define OSTREE_TYPE_GPG_VERIFIER _ostree_gpg_verifier_get_type()
#define OSTREE_GPG_VERIFIER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSTREE_TYPE_GPG_VERIFIER, OstreeGpgVerifier))
#define OSTREE_IS_GPG_VERIFIER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSTREE_TYPE_GPG_VERIFIER))

typedef struct OstreeGpgVerifier OstreeGpgVerifier;

/* If this type becomes public in future, move this autoptr cleanup
 * definition to the ostree-autocleanups.h header file. Right now it
 * relies on glnx's fallback definition of the macro. */
G_DEFINE_AUTOPTR_CLEANUP_FUNC (OstreeGpgVerifier, g_object_unref)

GType _ostree_gpg_verifier_get_type (void);

OstreeGpgVerifier *_ostree_gpg_verifier_new (void);

OstreeGpgVerifyResult *_ostree_gpg_verifier_check_signature (OstreeGpgVerifier *self,
                                                             GBytes            *signed_data,
                                                             GBytes            *signatures,
                                                             GCancellable      *cancellable,
                                                             GError           **error);

gboolean      _ostree_gpg_verifier_add_keyring_dir (OstreeGpgVerifier   *self,
                                                    GFile               *path,
                                                    GCancellable        *cancellable,
                                                    GError             **error);

gboolean      _ostree_gpg_verifier_add_keyring_dir_at (OstreeGpgVerifier   *self,
                                                       int                  dfd,
                                                       const char          *path,
                                                       GCancellable        *cancellable,
                                                       GError             **error);

gboolean      _ostree_gpg_verifier_add_global_keyring_dir (OstreeGpgVerifier  *self,
                                                           GCancellable       *cancellable,
                                                           GError            **error);

void _ostree_gpg_verifier_add_keyring_data (OstreeGpgVerifier *self,
                                            GBytes            *data,
                                            const char        *data_source);
void _ostree_gpg_verifier_add_keyring_file (OstreeGpgVerifier *self,
                                            GFile             *path);

void _ostree_gpg_verifier_add_key_ascii_file (OstreeGpgVerifier *self,
                                              const char        *path);

G_END_DECLS
