#!/bin/bash
#
# Copyright (C) 2016 Red Hat
#
# SPDX-License-Identifier: LGPL-2.0+
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

set -euo pipefail

. $(dirname $0)/libtest.sh

echo "1..4"

COMMIT_SIGN="--gpg-homedir=${TEST_GPG_KEYHOME} --gpg-sign=${TEST_GPG_KEYID_1}"

# Test pulling from a repo which gives error 500 (internal server error) a lot of the time.
setup_fake_remote_repo1 "archive" "${COMMIT_SIGN}" --random-500s=50

pushd ${test_tmpdir}
ostree_repo_init repo --mode=archive
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo
for x in $(seq 200); do
    if ${CMD_PREFIX} ostree --repo=repo pull --mirror origin main 2>err.txt; then
	echo "Success on iteration ${x}"
	break;
    fi
    assert_file_has_content err.txt "\(500.*Internal Server Error\)\|\(HTTP 500\)"
done

${CMD_PREFIX} ostree --repo=repo fsck
${CMD_PREFIX} ostree --repo=repo rev-parse main

popd
echo "ok repeated pull after 500s"

# Sanity check with no network retries and 408s given, pull should fail.
rm ostree-srv httpd repo -rf
setup_fake_remote_repo1 "archive" "${COMMIT_SIGN}" --random-408s=99

pushd ${test_tmpdir}
ostree_repo_init repo --mode=archive
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo
assert_fail ${CMD_PREFIX} ostree --repo=repo pull --mirror origin --network-retries=0 main 2>err.txt
assert_file_has_content err.txt "\(408.*Request Timeout\)\|\(HTTP 408\)"

popd
echo "ok no retries after a 408"

# Test pulling a repo which gives error 408 (request timeout) a lot of the time.
rm ostree-srv httpd repo -rf
setup_fake_remote_repo1 "archive" "${COMMIT_SIGN}" --random-408s=50

pushd ${test_tmpdir}
ostree_repo_init repo --mode=archive
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo
for x in $(seq 40); do
    if ${CMD_PREFIX} ostree --repo=repo pull --mirror origin --network-retries=2 main 2>err.txt; then
       echo "Success on iteration ${x}"
       break;
    fi
    assert_file_has_content err.txt "\(408.*Request Timeout\)\|\(HTTP 408\)"
done

${CMD_PREFIX} ostree --repo=repo fsck
${CMD_PREFIX} ostree --repo=repo rev-parse main

popd
echo "ok repeated pull after 408s"

# Test pulling a repo that gives 408s a lot of the time, with many network retries.
rm ostree-srv httpd repo -rf
setup_fake_remote_repo1 "archive" "${COMMIT_SIGN}" --random-408s=50

pushd ${test_tmpdir}
ostree_repo_init repo --mode=archive
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo

# Using 8 network retries gives error rate of <0.5%, when --random-408s=50
if ${CMD_PREFIX} ostree --repo=repo pull --mirror origin --network-retries=8 main 2>err.txt; then
   echo "Success with big number of network retries"
fi

${CMD_PREFIX} ostree --repo=repo fsck
${CMD_PREFIX} ostree --repo=repo rev-parse main

popd
echo "ok big number of retries with one 408"
