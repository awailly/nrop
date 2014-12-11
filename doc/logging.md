---
title: nROP Documentation
layout: base
---

# Logging

The various logging levels are available at the *start of C files*. It is close to the Qemu approach, replacing the LOG function by the `va_args` version of `logging()`.

You can then enable the logs of a file by defining the `DEBUG_` variable at the start of the file. The `#define` is usually left commented, thus you only need to uncomment it.

# Available debugs

The current list of logs is available below:

    % grep -Rn "define DEBUG" . | egrep -v "(z3|capstone|qemu)"
    ./disassemblers/disassembler_xed.c:9://#define DEBUG_XED
    ./disassemblers/disassembler_meta.c:11://#define DEBUG_META
    ./converter.c:7://#define DEBUG_LLVM
    ./chain.c:11://#define DEBUG_CHAIN
    ./map.c:7:#define DEBUG_Z3_SOLVE
    ./plugins/plugin_rop.c:7://#define DEBUG_ROP
    ./plugins/plugin_rop.c:14://#define DEBUG_ROP_DEBUG

