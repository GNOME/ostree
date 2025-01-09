// This file was generated by gir (https://github.com/gtk-rs/gir)
// from gir-files
// DO NOT EDIT

#include "manual.h"
#include <stdio.h>

#define PRINT_CONSTANT(CONSTANT_NAME) \
  printf ("%s;", #CONSTANT_NAME); \
  printf (_Generic ((CONSTANT_NAME), \
              char *: "%s", \
              const char *: "%s", \
              char: "%c", \
              signed char: "%hhd", \
              unsigned char: "%hhu", \
              short int: "%hd", \
              unsigned short int: "%hu", \
              int: "%d", \
              unsigned int: "%u", \
              long: "%ld", \
              unsigned long: "%lu", \
              long long: "%lld", \
              unsigned long long: "%llu", \
              float: "%f", \
              double: "%f", \
              long double: "%ld"), \
          CONSTANT_NAME); \
  printf ("\n");

int
main ()
{
  PRINT_CONSTANT ((guint)OSTREE_CHECKSUM_FLAGS_CANONICAL_PERMISSIONS);
  PRINT_CONSTANT ((guint)OSTREE_CHECKSUM_FLAGS_IGNORE_XATTRS);
  PRINT_CONSTANT ((guint)OSTREE_CHECKSUM_FLAGS_NONE);
  PRINT_CONSTANT (OSTREE_COMMIT_GVARIANT_STRING);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_ARCHITECTURE);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_COLLECTION_BINDING);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_ENDOFLIFE);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_ENDOFLIFE_REBASE);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_REF_BINDING);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_SOURCE_TITLE);
  PRINT_CONSTANT (OSTREE_COMMIT_META_KEY_VERSION);
  PRINT_CONSTANT ((gint)OSTREE_DEPLOYMENT_UNLOCKED_DEVELOPMENT);
  PRINT_CONSTANT ((gint)OSTREE_DEPLOYMENT_UNLOCKED_HOTFIX);
  PRINT_CONSTANT ((gint)OSTREE_DEPLOYMENT_UNLOCKED_NONE);
  PRINT_CONSTANT ((gint)OSTREE_DEPLOYMENT_UNLOCKED_TRANSIENT);
  PRINT_CONSTANT ((guint)OSTREE_DIFF_FLAGS_IGNORE_XATTRS);
  PRINT_CONSTANT ((guint)OSTREE_DIFF_FLAGS_NONE);
  PRINT_CONSTANT (OSTREE_DIRMETA_GVARIANT_STRING);
  PRINT_CONSTANT (OSTREE_FILEMETA_GVARIANT_STRING);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_EXPIRED_KEY);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_EXPIRED_SIGNATURE);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_INVALID_SIGNATURE);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_MISSING_KEY);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_NO_SIGNATURE);
  PRINT_CONSTANT ((gint)OSTREE_GPG_ERROR_REVOKED_KEY);
  PRINT_CONSTANT (OSTREE_GPG_KEY_GVARIANT_STRING);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_EXP_TIMESTAMP);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT_PRIMARY);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_HASH_ALGO_NAME);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_KEY_EXPIRED);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_KEY_EXP_TIMESTAMP);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_KEY_EXP_TIMESTAMP_PRIMARY);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_KEY_REVOKED);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_PUBKEY_ALGO_NAME);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_SIG_EXPIRED);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_TIMESTAMP);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_USER_NAME);
  PRINT_CONSTANT ((gint)OSTREE_GPG_SIGNATURE_ATTR_VALID);
  PRINT_CONSTANT ((guint)OSTREE_GPG_SIGNATURE_FORMAT_DEFAULT);
  PRINT_CONSTANT (OSTREE_MAX_METADATA_SIZE);
  PRINT_CONSTANT (OSTREE_MAX_METADATA_WARN_SIZE);
  PRINT_CONSTANT (OSTREE_METADATA_KEY_BOOTABLE);
  PRINT_CONSTANT (OSTREE_METADATA_KEY_LINUX);
  PRINT_CONSTANT (OSTREE_META_KEY_DEPLOY_COLLECTION_ID);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_COMMIT);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_COMMIT_META);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_DIR_META);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_DIR_TREE);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_FILE);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_FILE_XATTRS);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_FILE_XATTRS_LINK);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_PAYLOAD_LINK);
  PRINT_CONSTANT ((gint)OSTREE_OBJECT_TYPE_TOMBSTONE_COMMIT);
  PRINT_CONSTANT (OSTREE_ORIGIN_TRANSIENT_GROUP);
  PRINT_CONSTANT (OSTREE_PATH_BOOTED);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_FILTER_ALLOW);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_FILTER_SKIP);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_MODE_NONE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_MODE_USER);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_OVERWRITE_ADD_FILES);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_OVERWRITE_NONE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_OVERWRITE_UNION_FILES);
  PRINT_CONSTANT ((gint)OSTREE_REPO_CHECKOUT_OVERWRITE_UNION_IDENTICAL);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_FILTER_ALLOW);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_FILTER_SKIP);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_ITER_RESULT_DIR);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_ITER_RESULT_END);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_ITER_RESULT_ERROR);
  PRINT_CONSTANT ((gint)OSTREE_REPO_COMMIT_ITER_RESULT_FILE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_CANONICAL_PERMISSIONS);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_CONSUME);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_DEVINO_CANONICAL);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_ERROR_ON_UNLABELED);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_GENERATE_SIZES);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_MODIFIER_FLAGS_SKIP_XATTRS);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_STATE_FSCK_PARTIAL);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_STATE_NORMAL);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_STATE_PARTIAL);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_TRAVERSE_FLAG_COMMIT_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_COMMIT_TRAVERSE_FLAG_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_OBJECTS_ALL);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_OBJECTS_LOOSE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_OBJECTS_NO_PARENTS);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_OBJECTS_PACKED);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_REFS_EXT_ALIASES);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_REFS_EXT_EXCLUDE_MIRRORS);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_REFS_EXT_EXCLUDE_REMOTES);
  PRINT_CONSTANT ((guint)OSTREE_REPO_LIST_REFS_EXT_NONE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_LOCK_EXCLUSIVE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_LOCK_SHARED);
  PRINT_CONSTANT (OSTREE_REPO_METADATA_REF);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_ARCHIVE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_ARCHIVE_Z2);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_BARE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_BARE_SPLIT_XATTRS);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_BARE_USER);
  PRINT_CONSTANT ((gint)OSTREE_REPO_MODE_BARE_USER_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PRUNE_FLAGS_COMMIT_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PRUNE_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PRUNE_FLAGS_NO_PRUNE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PRUNE_FLAGS_REFS_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_BAREUSERONLY_FILES);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_COMMIT_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_MIRROR);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_TRUSTED_HTTP);
  PRINT_CONSTANT ((guint)OSTREE_REPO_PULL_FLAGS_UNTRUSTED);
  PRINT_CONSTANT ((gint)OSTREE_REPO_REMOTE_CHANGE_ADD);
  PRINT_CONSTANT ((gint)OSTREE_REPO_REMOTE_CHANGE_ADD_IF_NOT_EXISTS);
  PRINT_CONSTANT ((gint)OSTREE_REPO_REMOTE_CHANGE_DELETE);
  PRINT_CONSTANT ((gint)OSTREE_REPO_REMOTE_CHANGE_DELETE_IF_EXISTS);
  PRINT_CONSTANT ((gint)OSTREE_REPO_REMOTE_CHANGE_REPLACE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_RESOLVE_REV_EXT_LOCAL_ONLY);
  PRINT_CONSTANT ((guint)OSTREE_REPO_RESOLVE_REV_EXT_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_VERIFY_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_REPO_VERIFY_FLAGS_NO_GPG);
  PRINT_CONSTANT ((guint)OSTREE_REPO_VERIFY_FLAGS_NO_SIGNAPI);
  PRINT_CONSTANT ((guint)OSTREE_SEPOLICY_RESTORECON_FLAGS_ALLOW_NOLABEL);
  PRINT_CONSTANT ((guint)OSTREE_SEPOLICY_RESTORECON_FLAGS_KEEP_EXISTING);
  PRINT_CONSTANT ((guint)OSTREE_SEPOLICY_RESTORECON_FLAGS_NONE);
  PRINT_CONSTANT (OSTREE_SHA256_DIGEST_LEN);
  PRINT_CONSTANT (OSTREE_SHA256_STRING_LEN);
  PRINT_CONSTANT (OSTREE_SIGN_NAME_ED25519);
  PRINT_CONSTANT ((gint)OSTREE_STATIC_DELTA_GENERATE_OPT_LOWLATENCY);
  PRINT_CONSTANT ((gint)OSTREE_STATIC_DELTA_GENERATE_OPT_MAJOR);
  PRINT_CONSTANT ((gint)OSTREE_STATIC_DELTA_INDEX_FLAGS_NONE);
  PRINT_CONSTANT (OSTREE_SUMMARY_GVARIANT_STRING);
  PRINT_CONSTANT (OSTREE_SUMMARY_SIG_GVARIANT_STRING);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_NOT_DEFAULT);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_NO_CLEAN);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_RETAIN);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_RETAIN_PENDING);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_SIMPLE_WRITE_DEPLOYMENT_FLAGS_RETAIN_ROLLBACK);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_UPGRADER_FLAGS_IGNORE_UNCONFIGURED);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_UPGRADER_FLAGS_STAGE);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_UPGRADER_PULL_FLAGS_ALLOW_OLDER);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_UPGRADER_PULL_FLAGS_NONE);
  PRINT_CONSTANT ((guint)OSTREE_SYSROOT_UPGRADER_PULL_FLAGS_SYNTHETIC);
  PRINT_CONSTANT (OSTREE_TIMESTAMP);
  PRINT_CONSTANT (OSTREE_TREE_GVARIANT_STRING);
  return 0;
}
