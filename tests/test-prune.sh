#!/bin/bash
#
# Copyright (C) 2015 Red Hat, Inc.
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

set -e

. $(dirname $0)/libtest.sh

setup_fake_remote_repo1 "archive-z2"

echo '1..1'

cd ${test_tmpdir}
mkdir repo
${CMD_PREFIX} ostree --repo=repo init
${CMD_PREFIX} ostree --repo=repo remote add --set=gpg-verify=false origin $(cat httpd-address)/ostree/gnomerepo

mkdir -p tree/root
touch tree/root/a

# Add a few commits
seq 5 | while read; do
    echo a >> tree/root/a
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/ostree-srv/gnomerepo commit --branch=test -m test -s test tree
done


${CMD_PREFIX} ostree --repo=repo pull --depth=-1 origin test

${CMD_PREFIX} ostree prune --repo=repo --refs-only --depth=1 -v
find repo | grep \.commit$ | wc -l > commitcount
assert_file_has_content commitcount "^2$"
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_file_has_content tombstonecommitcount "^0$"

${CMD_PREFIX} ostree prune --repo=repo --refs-only --depth=0 -v
find repo | grep \.commit$ | wc -l > commitcount
assert_file_has_content commitcount "^1$"
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_file_has_content tombstonecommitcount "^0$"

${CMD_PREFIX} ostree --repo=repo fsck --add-tombstones
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_file_has_content repo/config "tombstone-commits=true"
assert_file_has_content tombstonecommitcount "^1$"

# pull once again and use tombstone commits
${CMD_PREFIX} ostree --repo=repo pull --depth=-1 origin test

${CMD_PREFIX} ostree --repo=repo fsck --add-tombstones
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_file_has_content tombstonecommitcount "^0$"

${CMD_PREFIX} ostree prune --repo=repo --refs-only --depth=0 -v
find repo/objects -name '*.commit' | wc -l > commitcount
assert_file_has_content commitcount "^1$"
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_not_file_has_content tombstonecommitcount "^0$"

# and that tombstone are deleted once the commits are pulled again
${CMD_PREFIX} ostree --repo=repo pull --depth=-1 origin test
find repo/objects -name '*.tombstone-commit' | wc -l > tombstonecommitcount
assert_file_has_content tombstonecommitcount "^0$"

echo "ok prune"
