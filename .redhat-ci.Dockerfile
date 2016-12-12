FROM fedora:25

RUN dnf install -y \
        gcc \
        sudo \
        which \
        attr \
        fuse \
        gjs \
        parallel \
        clang \
        libubsan \
        libasan \
        libtsan \
        gnome-desktop-testing \
        redhat-rpm-config \
        elfutils \
        'dnf-command(builddep)' \
 && dnf builddep -y \
        ostree \
 && dnf clean all

# create an unprivileged user for testing
RUN adduser testuser
