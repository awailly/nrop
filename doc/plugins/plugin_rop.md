---
title: nROP Documentation
layout: base
---

# Multithreading

The current multithreading is not optimal as the way qemu and LLVM are used is not thread-safe. Numerous mutex tries to minimize the impact but some cases will leave the tool running on one thread (for example with a lot of bad instructions).

It is also ugly to print in the same terminal with multiple threads, and one can forget to enable the multithread when pushing on the git. This is how you can control the threading:

    % grep MULTITHREAD plugins/plugin_rop.h
    #define MULTITHREAD

# Instruction categories

By default nrop gives a positive match on gadgets with a ending basic block behavior. It may thus return gadgets that are not fitting your constraints, i.e. only RET ending instructions. You can tune the behavior by filtering in the `is_last_inst()` function of the plugin, or directly in the disassembler `get_category()` function.

A quick fix is to `| grep ret` on the output.
