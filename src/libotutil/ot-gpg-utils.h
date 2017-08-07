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

#pragma once

#include <gpgme.h>
#include <gio/gio.h>
#include "libglnx.h"

G_BEGIN_DECLS

G_DEFINE_AUTO_CLEANUP_FREE_FUNC(gpgme_data_t, gpgme_data_release, NULL)
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(gpgme_ctx_t, gpgme_release, NULL)
G_DEFINE_AUTO_CLEANUP_FREE_FUNC(gpgme_key_t, gpgme_key_unref, NULL)

void ot_gpgme_error_to_gio_error (gpgme_error_t gpg_error, GError **error);

gboolean ot_gpgme_ctx_tmp_home_dir (gpgme_ctx_t     gpgme_ctx,
                                    char          **out_tmp_home_dir,
                                    GOutputStream **out_pubring_stream,
                                    GCancellable   *cancellable,
                                    GError        **error);

gpgme_data_t ot_gpgme_data_input (GInputStream *input_stream);
gpgme_data_t ot_gpgme_data_output (GOutputStream *output_stream);

gpgme_ctx_t ot_gpgme_new_ctx (const char *homedir,
                              GError    **error);

G_END_DECLS
