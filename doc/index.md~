---
title: nROP Documentation
layout: base
---

# Testing

./packer -t 68ccedffff5a4883ec085b4831c04829d0c3 /usr/lib/libc.so.6

# FAQ

## /bin/sh: llvm-config : commande introuvable

pacaur -S llvm

## make: *** No rule to make target `/usr/include/stdc-predef.h', needed by `packer.o'.  Stop.

Remove the lines below `DO NOT REMOVE` in the makefile, or update it with `make depend`.

## /usr/include/glib-2.0/glib/gtypes.h:34:24: fatal error: glibconfig.h: No such file or directory

Replace the static end of `INCLUDES` line in the Makefile by

    `pkg-config --cflags glib-2.0`

If you don't have the `pkg-config` executable, replace with correct location

## Out of tree build

Run the following command:

    PYTHON=$(which python2) ./configure --target-list=x86_64-linux-user --disable-sparse --disable-strip --disable-werror --disable-sdl --disable-gtk --disable-virtfs --disable-vnc --disable-cocoa --disable-xen --disable-xen-pci-passthrough --disable-brlapi --disable-vnc-tls --disable-vnc-sasl --disable-vnc-jpeg --disable-vnc-png --disable-vnc-ws --disable-curses --disable-curl --disable-fdt --disable-bluez --disable-slirp --disable-kvm --disable-rdma --disable-system --disable-guest-base --disable-pie --disable-uuid --disable-vde --disable-netmap --disable-linux-aio --disable-cap-ng --disable-attr --disable-blobs --disable-docs --disable-vhost-net --disable-spice --disable-libiscsi --disable-libnfs --disable-smartcard-nss --disable-libusb --disable-usb-redir --disable-guest-agent --disable-seccomp --disable-coroutine-pool --disable-glusterfs --disable-libssh2 --disable-vhdx --disable-quorum --disable-bsd-user

## qemu/rules.mak:33: recipe for target 'coroutine-ucontext.o' failed

Install valgrind

## error: ‘INDEX_op_qemu_st8’ undeclared (first use in this function)

With new qemu the TCG operations may change their names, so comment out the line and make again.

## While making nrop: g++: error: qemu/trace/default.o: No such file or directory

Welcome to Sparta. the `QEMU_ALL` variable in the Makefile is a dirty step to link against all qemu objects. Some will be deleted from qemu and have to be removed in `QEMU_ALL`. Others have to be added and will appear as 

    undefined reference to `qapi_event_send_block_job_error'

You thus have to include the new object file in the QEMU_ALL variable. The current version was built again qemu commit :

    commit e9d818b8b1a7fadc6c92256b716f1bc21b8daabc

If you are brave enough, you can feed the QEMU_ALL variable with the output of `find qemu/ -name "*.o" | awk '{print}' ORS=' '`

### Yes I am

Ok,

    find qemu/ -name "*.o" | awk '{print}' ORS=' '
    # Out conflicts
    make 2>&1 | grep "In function" | awk '{ print $1 }' | uniq | awk '{print substr($0, 0, length($0))}' ORS='|'
    find qemu/ -name "*.o" | egrep -v "($_)" | awk '{print}' ORS=' '

I think qemu/x86*/linux-user/main.o is filtered, so add it manually. Then for each undefined

    grep -Rni vmstate_register_with_alias_id qemu # to find multiple references
    # Remove the module from QEMU_ALL

Something about jit_register_code happens, replace `__jit` by `__jat` in qemu/tcg/tcg.c.

# Development specific

## From 1.X to 2.X

The raise_exception moved from 

    00125615: adc al, 0x1 ; pshufb xmm0, xmm2 ; pcmpistri xmm1, xmm0, 0x12 ; movsxd rax, ecx ; mov ecx, 0x10 ; sub ecx, esi ; cmp ecx, eax ; mov esi, ecx ; jnle 0x1255a2 ;
    000000: \x14\x01\x66\x0f\x38\x00\xc2\x66\x0f\x3a\x63\xc8\x12\x48\x63\xc1 ..f.8..f.:c..Hc.
    000010: \xb9\x10\x00\x00\x00\x29\xf1\x39\xc1\x89\xce\x0f\x8f\x6c\xff\xff .....).9.....l..
    000020: \xff                                              .
     movi_i64 tmp1,$0x1
     nopn $0x2,$0x2
     movi_i64 tmp11,$cc_compute_c
     call tmp11,$0x50,$1,tmp7,cc_dst,cc_src,cc_src2,cc_op
     add_i64 tmp0,rax,tmp1
     add_i64 tmp0,tmp0,tmp7
     deposit_i64 rax,rax,tmp0,$0x0,$0x8
     mov_i64 cc_src2,tmp7
     movi_i64 cc_src,$0x1
     mov_i64 cc_dst,tmp0
     discard cc_op
     movi_i32 cc_op,$0xa
     movi_i64 tmp3,$0x1055acc2
     st_i64 tmp3,env,$0x80
     movi_i32 tmp12,$0x6
     movi_i64 tmp11,$raise_exception
     call tmp11,$0x0,$0,env,tmp12
     end 

to

    00125615: adc al, 0x1 ; pshufb xmm0, xmm2 ; pcmpistri xmm1, xmm0, 0x12 ; movsxd rax, ecx ; mov ecx, 0x10 ; sub ecx, esi ; cmp ecx, eax ; mov esi, ecx ; jnle 0x1255a2 ;
    000000: \x14\x01\x66\x0f\x38\x00\xc2\x66\x0f\x3a\x63\xc8\x12\x48\x63\xc1 ..f.8..f.:c..Hc.
    000010: \xb9\x10\x00\x00\x00\x29\xf1\x39\xc1\x89\xce\x0f\x8f\x6c\xff\xff .....).9.....l..
    000020: \xff                                              .
     movi_i64 tmp1,$0x1
     nopn $0x2,$0x2
     call cc_compute_c,$0x50,$1,tmp7,cc_dst,cc_src,cc_src2,cc_op
     add_i64 tmp0,rax,tmp1
     add_i64 tmp0,tmp0,tmp7
     deposit_i64 rax,rax,tmp0,$0x0,$0x8
     mov_i64 cc_src2,tmp7
     movi_i64 cc_src,$0x1
     mov_i64 cc_dst,tmp0
     discard cc_op
     movi_i32 cc_op,$0xa
     movi_i64 tmp3,$0x1084b622
     st_i64 tmp3,env,$0x80
     movi_i32 tmp11,$0x6
     call raise_exception,$0x0,$0,env,tmp11
     end

Note that the `call` now use the `raise_exception` name and not `tmp11`, thus I filter on call for now. I will do the same for `pause` and `raise_interrupt`. The `INDEX_op_call` handling in qemu/tcg/tcg.c is different with qemu2.X and I imported the new `tcg_find_helper`.
