---
title: nROP Documentation
layout: base
---

[1]: faq.html
[2]: disassemblers.html
[3]: logging.html
[4]: plugins/plugin_rop.html
[5]: converter.html

# Documentation

The nrop components and tricks are detailed in the following sections. If you feel that something is poorly explained feel free to contact me :)

# Disassemblers

nrop relied for a long time on the XED disassemblers, but now a new tiny interface is available to use your custom state-of-art disassembler. You can see the in-depth details in the [dedicated documentation page][2].

# Logging/Debug

Several levels of logs and debug are available in the nrop source code. However they are disabled by default for performances and have to be enabled before compilation. The [logging documentation page][3] details how it is implemented and how to enable them.

# Plugins

The nrop architecture allows more than just Return-Oriented Programming. You can play with the ELF file format and create new plugins by registering your new code in the main file `packer.c`.

The following pages are explaining specific plugins.

- [plugin_rop documentation][4]
- [plugin_junk documentation][4]

# Using the engine as a library

If you only need the assembly converter to LLVM, [read the dedicated page][5]

# FAQ

The compilation can be complicated, and the [faq dedicated page][1] is a convenient start to progress.

