/*
 * Copyright © 2017 Endless Mobile, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.0+
 *
 * Authors:
 *  - Philip Withnall <withnall@endlessm.com>
 */

#pragma once

#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * OstreeBloom:
 *
 * An implementation of a [bloom filter](https://en.wikipedia.org/wiki/Bloom_filter)
 * which is suitable for building a filter and looking keys up in an existing
 * filter.
 *
 * Since: 2017.8
 */
typedef struct _OstreeBloom OstreeBloom;

/**
 * OstreeBloomHashFunc:
 * @element: a pointer to the element to hash
 * @k: hash function parameter
 *
 * Function prototype for a
 * [universal hash function](https://en.wikipedia.org/wiki/Universal_hashing),
 * parameterised on @k, which hashes @element to a #guint64 hash value.
 *
 * It is up to the implementer of the hash function whether %NULL is valid for
 * @element.
 *
 * Since: 2017.8
 */
typedef guint64 (*OstreeBloomHashFunc) (gconstpointer element,
                                        guint8        k);

#define OSTREE_TYPE_BLOOM (ostree_bloom_get_type ())

G_GNUC_INTERNAL
GType ostree_bloom_get_type (void);

G_GNUC_INTERNAL
OstreeBloom *ostree_bloom_new (gsize               n_bytes,
                               guint8              k,
                               OstreeBloomHashFunc hash_func);

G_GNUC_INTERNAL
OstreeBloom *ostree_bloom_new_from_bytes (GBytes              *bytes,
                                          guint8               k,
                                          OstreeBloomHashFunc  hash_func);

G_GNUC_INTERNAL
OstreeBloom *ostree_bloom_ref (OstreeBloom *bloom);
G_GNUC_INTERNAL
void ostree_bloom_unref (OstreeBloom *bloom);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (OstreeBloom, ostree_bloom_unref)

G_GNUC_INTERNAL
gboolean ostree_bloom_maybe_contains (OstreeBloom   *bloom,
                                      gconstpointer  element);

G_GNUC_INTERNAL
GBytes *ostree_bloom_seal (OstreeBloom *bloom);

G_GNUC_INTERNAL
void ostree_bloom_add_element (OstreeBloom   *bloom,
                               gconstpointer  element);

G_GNUC_INTERNAL
gsize ostree_bloom_get_size (OstreeBloom *bloom);
G_GNUC_INTERNAL
guint8 ostree_bloom_get_k (OstreeBloom *bloom);
G_GNUC_INTERNAL
OstreeBloomHashFunc ostree_bloom_get_hash_func (OstreeBloom *bloom);

G_GNUC_INTERNAL
guint64 ostree_str_bloom_hash (gconstpointer element,
                               guint8        k);

G_END_DECLS
