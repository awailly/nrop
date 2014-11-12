---
title: Download && Build nROP
layout: base
---

[1]: http://aurelien.wail.ly/nrop/getstarted

# Getting the sources

Clone the github repository

    git clone http://github.com/awailly/nrop
    cd nrop

You can grab the pre-compiled binary here.

# Build the project

## Reset the qemu changes

    cd qemu
    git stash

## Apply patches to qemu && build

    git apply ../qemu.noprologet.patch
    make -j32
    cd ..
    make -j32 # error wanted regarding "abi_ulong"

## Fix abi_ulong error

    cd qemu
    git apply ../qemu.patch
    cd ..
    make -j32 # error wanted regarding multiple main and icount

## Fix main and use_icount errors

    vim qemu/x86_64-linux-user/linux-user/main.o # :%s/main/ma1n/g
    vim qemu/stubs/cpu-get-icount.o # :%s/use_icount/use_1count/g
    make -j32

I know :)

# Use it

You can now follow the [Get Started guide][1]
