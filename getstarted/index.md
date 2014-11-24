---
title: Download nROP
layout: base
---

[1]: http://aurelien.wail.ly/nrop/doc
[2]: http://aurelien.wail.ly/nrop/download

# Build it

The process to build the program is described in the [Documentation page][2]

# Usage

The program expect a target (`-t`) shellcode, say x86 here, and the binary to analyze in the last position (`argv[argc - 1]`).

The `-o` option output a new binary file if you modified the binary to analyze with a plugin (look into the `plugins/` directory for examples). It is not needed to use the nROP program.

The `-c` option specify constraints to respect when building the ROP payload. It is not needed to use the nROP program.

# Creating a target

We start with a `push 0x1234` shellcode, assembling to `6834120000`, and insert a ret, `c3`.

    % ./packer -t 6834120000c3 /usr/lib/libc.so.6

You can see the entrypoint of the file at first. nROP look for gadgets in all PHT with the +x flag, you can change this behavior in the plugin. If you want to scan everywhere you have to consider the input file as pure binary.

Then you have the shellcode dumped by XED (or capstone), analyzed by the Qemu TCG and dumped again, finally translated into LLVM and optimized.

    % ./packer -t 68ccedffff5a4883ec085b4831c04829d0c3 /usr/lib/libc.so.6

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

You can now follow the [debugging guide][1], as you will encounter tons of bugs
