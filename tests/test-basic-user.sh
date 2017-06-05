#!/bin/bash
#
# Copyright (C) 2011 Colin Walters <walters@verbum.org>
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

skip_without_user_xattrs

setup_test_repository "bare-user"

extra_basic_tests=3
. $(dirname $0)/basic-test.sh

# Reset things so we don't inherit a lot of state from earlier tests
rm repo files -rf
setup_test_repository "bare-user"

cd ${test_tmpdir}
objpath_nonexec=$(ostree_file_path_to_object_path repo test2 baz/cow)
# This actually depends on umask, that's going to be a pain
assert_file_has_mode ${objpath_nonexec} 664
objpath_ro=$(ostree_file_path_to_object_path repo test2 baz/cowro)
assert_file_has_mode ${objpath_ro} 600
objpath_exec=$(ostree_file_path_to_object_path repo test2 baz/deeper/ohyeahx)
assert_file_has_mode ${objpath_exec} 755
echo "ok bare-user committed modes"

rm test2-checkout -rf
$OSTREE checkout -U -H test2 test2-checkout
cd test2-checkout
assert_file_has_mode baz/cow 664
assert_file_has_mode baz/cowro 600
assert_file_has_mode baz/deeper/ohyeahx 755
echo "ok bare-user checkout modes"

rm test2-checkout -rf
$OSTREE checkout -U -H test2 test2-checkout
touch test2-checkout/unreadable
chmod 0400 test2-checkout/unreadable
$OSTREE commit -b test2-unreadable --tree=dir=test2-checkout
chmod 0600 test2-checkout/unreadable
rm test2-checkout -rf
$OSTREE checkout -U -H test2-unreadable test2-checkout
cd test2-checkout
assert_file_has_mode unreadable 400
echo "ok bare-user unreadable"
