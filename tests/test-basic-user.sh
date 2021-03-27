#!/bin/bash
#
# Copyright (C) 2011 Colin Walters <walters@verbum.org>
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

skip_without_user_xattrs

mode="bare-user"
setup_test_repository "$mode"

extra_basic_tests=7
. $(dirname $0)/basic-test.sh

# Reset things so we don't inherit a lot of state from earlier tests
rm repo files -rf
setup_test_repository "bare-user"

cd ${test_tmpdir}
objpath_nonexec=$(ostree_file_path_to_object_path repo test2 baz/cow)
assert_file_has_mode ${objpath_nonexec} 644
objpath_ro=$(ostree_file_path_to_object_path repo test2 baz/cowro)
assert_file_has_mode ${objpath_ro} 600
objpath_exec=$(ostree_file_path_to_object_path repo test2 baz/deeper/ohyeahx)
assert_file_has_mode ${objpath_exec} 755
echo "ok bare-user committed modes"

rm test2-checkout -rf
$OSTREE checkout -U -H test2 test2-checkout
cd test2-checkout
assert_file_has_mode baz/cow 644
assert_file_has_mode baz/cowro 600
assert_file_has_mode baz/deeper/ohyeahx 755
echo "ok bare-user checkout modes"

rm test2-checkout -rf
$OSTREE checkout -U -H test2 test2-checkout
touch test2-checkout/unwritable
chmod 0400 test2-checkout/unwritable
$OSTREE commit -b test2-unwritable --tree=dir=test2-checkout
chmod 0600 test2-checkout/unwritable
rm test2-checkout -rf
$OSTREE checkout -U -H test2-unwritable test2-checkout
cd test2-checkout
assert_file_has_mode unwritable 400
echo "ok bare-user unwritable"

rm test2-checkout -rf
$OSTREE checkout -U -H test2 test2-checkout
cat > statoverride.txt <<EOF
=0 /unreadable
EOF
touch test2-checkout/unreadable
$OSTREE commit -b test2-unreadable --statoverride=statoverride.txt --tree=dir=test2-checkout
$OSTREE fsck
rm test2-checkout -rf
$OSTREE checkout -U -H test2-unreadable test2-checkout
assert_file_has_mode test2-checkout/unreadable 400
# Should not be hardlinked
assert_streq $(stat -c "%h" test2-checkout/unreadable) 1
echo "ok bare-user handled unreadable file"

cd ${test_tmpdir}
mkdir -p components/{dbus,systemd}/usr/{bin,lib}
echo dbus binary > components/dbus/usr/bin/dbus-daemon
chmod a+x components/dbus/usr/bin/dbus-daemon
echo dbus lib > components/dbus/usr/lib/libdbus.so.1
echo dbus helper > components/dbus/usr/lib/dbus-daemon-helper
chmod a+x components/dbus/usr/lib/dbus-daemon-helper
echo systemd binary > components/systemd/usr/bin/systemd
chmod a+x components/systemd/usr/bin/systemd
echo systemd lib > components/systemd/usr/lib/libsystemd.so.1

# Make the gid on dbus 81 like fedora
$OSTREE commit -b component-dbus --owner-uid 0 --owner-gid 81 --tree=dir=components/dbus
$OSTREE commit -b component-systemd --owner-uid 0 --owner-gid 0 --tree=dir=components/systemd
rm rootfs -rf
for component in dbus systemd; do
    $OSTREE checkout -U -H component-${component} --union rootfs
done
echo 'some rootfs data' > rootfs/usr/lib/cache.txt
$OSTREE commit -b rootfs --link-checkout-speedup --tree=dir=rootfs
$OSTREE ls rootfs /usr/bin/systemd >ls.txt
assert_file_has_content ls.txt '^-007.. 0 0 .*/usr/bin/systemd'
$OSTREE ls rootfs /usr/lib/dbus-daemon-helper >ls.txt
assert_file_has_content ls.txt '^-007.. 0 81 .*/usr/lib/dbus-daemon-helper'
echo "ok bare-user link-checkout-speedup maintains uids"

cd ${test_tmpdir}
rm -rf test2-checkout
$OSTREE checkout -H -U test2 test2-checkout
# With --link-checkout-speedup, specifying --owner-uid should "win" by default.
myuid=$(id -u)
mygid=$(id -g)
newuid=$((${myuid} + 1))
newgid=$((${mygid} + 1))
$OSTREE commit ${COMMIT_ARGS} --owner-uid ${newuid} --owner-gid ${newgid} \
        --link-checkout-speedup -b test2-linkcheckout-test --tree=dir=test2-checkout
$OSTREE ls test2-linkcheckout-test /baz/cow > ls.txt
assert_file_has_content ls.txt "^-006.. ${newuid} ${newgid} .*/baz/cow"

# But --devino-canonical should override that
$OSTREE commit ${COMMIT_ARGS} --owner-uid ${newuid} --owner-gid ${newgid} \
        -I -b test2-devino-test --table-output --tree=dir=test2-checkout > out.txt
$OSTREE ls test2-devino-test /baz/cow > ls.txt
assert_file_has_content ls.txt "^-006.. ${myuid} ${mygid} .*/baz/cow"
assert_file_has_content out.txt "Content Cache Hits: [1-9][0-9]*"

$OSTREE refs --delete test2-{linkcheckout,devino}-test
echo "ok commit with -I"

mkdir -p xattrroot/bin xattrroot/home/ostree
touch xattrroot/bin/sudo xattrroot/bin/bash xattrroot/bin/bwrap

cat >setx.py <<'EOF'
#/usr/bin/python
import os
import socket
import sys

import xattr
from gi.repository import GLib


def setx(filename, uid, gid, mode, xattrs=None):
    if xattrs is None:
        xattrs = {}
    type_bits = os.stat(filename).st_mode & 0o170000
    v = GLib.Variant('(uuua(ayay))', (socket.htonl(uid), socket.htonl(gid),
                                      socket.htonl(type_bits | mode),
                                      xattrs.items()))
    xattr.setxattr(filename, "user.ostreemeta",
                   v.get_data_as_bytes().get_data())


setx("xattrroot", 0, 0, 0o0755)
setx("xattrroot/bin", 0, 0, 0o0755)
setx("xattrroot/bin/bwrap", 0, 0, 0o0755, {
    b"security.capability": (b"\x00\x01\x02\x00\x20\x00\x00\x00"
                             b"\x20\x00\x00\x00\x00\x00\x00\x00"
                             b"\x00\x00\x00\x00")})
setx("xattrroot/bin/bash", 0, 0, 0o0755)
setx("xattrroot/bin/sudo", 0, 0, 0o4755)
setx("xattrroot/home", 0, 0, 0o0755)
setx("xattrroot/home/ostree", 1001, 80, 0o0700)
EOF
python setx.py
attr -l xattrroot/bin/bwrap

$OSTREE commit -b xattrtest --tree=dir=xattrroot \
               --link-checkout-speedup --consume \
               --use-bare-user-xattrs
$OSTREE ls -RX xattrtest >out
cat >expected <<EOF
d00755 0 0      0 { @a(ayay) [] } /
d00755 0 0      0 { @a(ayay) [] } /bin
-00755 0 0      0 { @a(ayay) [] } /bin/bash
-00755 0 0      0 { [([byte 0x73, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x2e, 0x63, 0x61, 0x70, 0x61, 0x62, 0x69, 0x6c, 0x69, 0x74, 0x79], [byte 0x00, 0x01, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])] } /bin/bwrap
-04755 0 0      0 { @a(ayay) [] } /bin/sudo
d00755 0 0      0 { @a(ayay) [] } /home
d00700 1001 80      0 { @a(ayay) [] } /home/ostree
EOF

diff -u expected out || fatal "Tree contents incorrect"

echo "ok commit with --use-bare-user-xattrs"
