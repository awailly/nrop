---
title: Download nROP
layout: base
---

[1]: ../doc
[2]: ../download
[3]: ../demos

# Build it

The process to build the program is described in the [Documentation page][2]

# Usage

The program expect the binary to analyze in the last position (`argv[argc - 1]`). Without any option it is a fast ROP gadget finder for the binary.

The `-t` option create a target shellcode from the hexadecimal representation.

The `-o` option output a new binary file if you modified the binary to analyze with a plugin (look into the `plugins/` directory for examples).

The `-c` option specify constraints to respect when building the ROP payload.

So the simplest usage of nROP is :

    % ./packer /usr/lib/libc.so.6

# Creating a target

We start with a `push 0x1234` shellcode, assembling to `6834120000`, and insert a ret, `c3`.

    % ./packer -t 6834120000c3 /usr/lib/libc.so.6

You can see the entrypoint of the file at first. nROP look for gadgets in all PHT with the +x flag, you can change this behavior in the plugin. If you want to scan everywhere you have to consider the input file as pure binary.

Then you have the shellcode dumped by XED (or capstone), analyzed by the Qemu TCG and dumped again, finally translated into LLVM and optimized.

    % ./packer -t 68ccedffff5a4883ec085b4831c04829d0c3 /usr/lib/libc.so.6

For advanced examples and usages see the [demonstrations][3].

# Use it

You can now follow the [debugging guide][1], as you will encounter tons of bugs
