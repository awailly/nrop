---
title: Download nROP
layout: base
---

# Dependencies

## Debian

The installation was tested on `Linux debian 3.2.0-4-amd64 #1 SMP Debian 3.2.60-1+deb7u3 x86_64 GNU/Linux`.

    sudo aptitude install build-essential git pkg-config zlib1g-dev libglib2.0-dev libpixman-1-dev libfdt-dev 

# Getting the sources

Clone the github repository

    git clone http://github.com/awailly/nrop
    cd nrop

You can grab the pre-compiled binary here.

# Build the project

## Get latest qemu

    git clone git://git.qemu-project.org/qemu.git # kikoo 200kB/s
    cd qemu
    PYTHON=$(which python2) ./configure --target-list=x86_64-linux-user --disable-sparse --disable-strip --disable-werror --disable-sdl --disable-gtk --disable-virtfs --disable-vnc --disable-cocoa --disable-xen --disable-xen-pci-passthrough --disable-brlapi --disable-vnc-tls --disable-vnc-sasl --disable-vnc-jpeg --disable-vnc-png --disable-vnc-ws --disable-curses --disable-curl --disable-fdt --disable-bluez --disable-slirp --disable-kvm --disable-rdma --disable-system --disable-guest-base --disable-pie --disable-uuid --disable-vde --disable-netmap --disable-linux-aio --disable-cap-ng --disable-attr --disable-blobs --disable-docs --disable-vhost-net --disable-spice --disable-libiscsi --disable-libnfs --disable-smartcard-nss --disable-libusb --disable-usb-redir --disable-guest-agent --disable-seccomp --disable-coroutine-pool --disable-glusterfs --disable-libssh2 --disable-vhdx --disable-quorum --disable-bsd-user
    make

## With qemu already pulled

    cd qemu
    git stash
    patch -p1 < ../qemu.noprologet.patch
    # git apply ../qemu.noprologet.patch
    make -j32
    cd ..
    make -j32 # error wanted regarding "abi_ulong"
    cd qemu
    patch -p1 < ../qemu.patch
    # git apply ../qemu.patch
    cd ..
    make -j32 # error wanted regarding multiple main and icount
    #vim qemu/x86_64-linux-user/linux-user/main.o # :%s/main/ma1n/g
    sed -i 's/main/ma1n/g' qemu/x86_64-linux-user/linux-user/main.o
    #vim qemu/stubs/cpu-get-icount.o # :%s/use_icount/use_1count/g
    sed -i 's/use_icount/use_1count/g' qemu/stubs/cpu-get-icount.o
    make -j32

# Use it

You can now follow the [Get Started guide](#{site.base_url}/getstarted)
