#!/bin/bash

# Verify our /etc merge works with selinux

set -xeuo pipefail

dn=$(dirname $0)
. ${dn}/libinsttest.sh

# Create a new deployment
ostree admin deploy --karg-proc-cmdline ${host_refspec}
new_deployment_path=/ostree/deploy/${host_osname}/deploy/${host_commit}.1

# Test /etc directory mtime
if ! test ${new_deployment_path}/etc/NetworkManager -nt /etc/NetworkManager; then
    ls -al ${new_deployment_path}/etc/NetworkManager /etc/NetworkManager
    fatal "/etc directory mtime not newer"
fi

# A set of files that have a variety of security contexts
for file in fstab passwd exports hostname sysctl.conf yum.repos.d \
            NetworkManager/dispatcher.d/hook-network-manager; do
    if ! test -e /etc/${file}; then
        continue
    fi

    current=$(cd /etc && ls -Z ${file})
    new=$(cd ${new_deployment_path}/etc && ls -Z ${file})
    assert_streq "${current}" "${new}"
done

# This captures all of the boot entries; it'd be slightly annoying
# to try to figure out the accurate one, so let's just ensure that at least
# one entry is boot_t.
# https://bugzilla.redhat.com/show_bug.cgi?id=1536991
ls -Z /boot/ostree/*/ > bootlsz.txt
assert_file_has_content_literal bootlsz.txt 'system_u:object_r:boot_t:s0 vmlinuz-'
assert_file_has_content_literal bootlsz.txt 'system_u:object_r:boot_t:s0 initramfs-'

# Cleanup
ostree admin undeploy 0
