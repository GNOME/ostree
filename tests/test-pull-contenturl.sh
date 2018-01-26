#!/bin/bash
#
# Copyright (C) 2016 Red Hat, Inc.
#
# SPDX-License-Identifier: LGPL-2.0+

set -euo pipefail

. $(dirname $0)/libtest.sh

echo "1..2"

COMMIT_SIGN=""
if has_gpgme; then
  COMMIT_SIGN="--gpg-homedir=${TEST_GPG_KEYHOME} --gpg-sign=${TEST_GPG_KEYID_1}"
fi

setup_fake_remote_repo1 "archive" "${COMMIT_SIGN}"

# create a summary
${CMD_PREFIX} ostree --repo=${test_tmpdir}/ostree-srv/gnomerepo \
  summary -u ${COMMIT_SIGN}

# Let's bring up an identical server in which meta files are missing
cd ${test_tmpdir}
mkdir httpd-content
cd httpd-content
cp -a ${test_tmpdir}/ostree-srv ostree

# delete all the meta stuff from here
rm ostree/gnomerepo/summary
if has_gpgme; then
  rm ostree/gnomerepo/summary.sig
  find ostree/gnomerepo/objects -name '*.commitmeta' | xargs rm
fi

# delete all the content stuff from there
find ${test_tmpdir}/ostree-srv/gnomerepo/objects \
  ! -name '*.commitmeta' -type f | xargs rm

${OSTREE_HTTPD} --autoexit --daemonize -p ${test_tmpdir}/httpd-content-port
content_port=$(cat ${test_tmpdir}/httpd-content-port)
echo "http://127.0.0.1:${content_port}" > ${test_tmpdir}/httpd-content-address

cd ${test_tmpdir}
mkdir repo
ostree_repo_init repo
if has_gpgme; then VERIFY=true; else VERIFY=false; fi
${CMD_PREFIX} ostree --repo=repo remote add origin \
  --set=gpg-verify=$VERIFY --set=gpg-verify-summary=$VERIFY \
  --contenturl=$(cat httpd-content-address)/ostree/gnomerepo \
  $(cat httpd-address)/ostree/gnomerepo
${CMD_PREFIX} ostree --repo=repo pull origin:main

echo "ok pull objects from contenturl"

if ! has_gpgme; then
  echo "ok don't pull sigs from contenturl # SKIP not compiled with gpgme"
else
  echo "ok don't pull sigs from contenturl"
fi
