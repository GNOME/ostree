#!/bin/bash
#
# Copyright © 2017 Endless Mobile, Inc.
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
#
# Authors:
#  - Philip Withnall <withnall@endlessm.com>

set -euo pipefail

. $(dirname $0)/libtest.sh

echo "1..2"

COMMIT_SIGN="--gpg-homedir=${TEST_GPG_KEYHOME} --gpg-sign=${TEST_GPG_KEYID_1}"
setup_fake_remote_repo1 "archive-z2" "${COMMIT_SIGN}"

# Set up a second branch.
mkdir ${test_tmpdir}/ostree-srv/other-files
cd ${test_tmpdir}/ostree-srv/other-files
echo 'hello world some object' > hello-world
${CMD_PREFIX} ostree  --repo=${test_tmpdir}/ostree-srv/gnomerepo commit ${COMMIT_SIGN} -b other -s "A commit" -m "Example commit body"

# Generate the summary file.
${CMD_PREFIX} ostree --repo=${test_tmpdir}/ostree-srv/gnomerepo summary -u

# Check out the repository.
prev_dir=`pwd`
cd ${test_tmpdir}
ostree_repo_init repo --mode=archive-z2
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo
${CMD_PREFIX} ostree --repo=repo pull --mirror origin

# Check the summary file exists in the checkout, and can be viewed.
assert_has_file repo/summary
output=$(${OSTREE} summary --view)
echo "$output" | sed -e 's/^/# /'
echo "$output" | grep --quiet --no-messages "* main"
echo "$output" | grep --quiet --no-messages "* other"
echo "$output" | grep --quiet --no-messages "ostree.summary.last-modified"
echo "$output" | grep --quiet --no-messages "Static Deltas (ostree.static-deltas): {}"
echo "ok view summary"

# Check the summary can be viewed raw too.
raw_output=$(${OSTREE} summary --view --raw)
echo "$raw_output" | sed -e 's/^/# /'
echo "$raw_output" | grep --quiet --no-messages "('main', ("
echo "$raw_output" | grep --quiet --no-messages "('other', ("
echo "$raw_output" | grep --quiet --no-messages "{'ostree.summary.last-modified': <uint64"
echo "ok view summary raw"

libtest_cleanup_gpg
