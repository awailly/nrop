---
title: nROP disassemblers
layout: base
---

[xed]: https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads
[capstone]: http://capstone-engine.org
[xedonly]: http://ix.io/17D
[caponly]: http://ix.io/18a

The nrop tool relies on disassemblers to find instructions with correct size. The question of the best disassembler is thus legitimate to find all of the instructions. These section present the various disassemblers used by nrop and how the handle tricky ones. The default disassembler is defined in the `chain.h` file.

# Interface

The tool is designed to integrate new disassembler easily. The only step needed is to create a new file under the `disassemblers/` directory and implement the `disassemblers/disassembler.h` interface.

    struct disassembler_t
    {
        /**
         * Initialize the disassembler for the architecture.
         *
         * @param arch      the architecture name
         * @return          STATUS of the initialization
         */
        status_t (*initialize)(disassembler_t *, chunk_t);

        /**
         * Get the category of the instruction. Described as category_t
         *
         * @param instruction   the instruction to analyze
         * @return category     the category of the instruction
         */
        category_t (*get_category)(disassembler_t *, instruction_t *);

        /**
         * Get the instruction length.
         *
         * @param instruction   the instruction to analyze
         * @return length       the length of the instruction
         */
        uint64_t (*get_length)(disassembler_t *, instruction_t *);

        /**
         * Dump in the disassembler default format.
         *
         * @param instruction   the instruction to dump
         * @param format        the format to dump instructions
         * @return              STATUS of the dump format modification
         */
        status_t (*format)(disassembler_t *, instruction_t *, chunk_t *);

        /**
         * Dump the instruction in the INTEL format.
         *
         * @param instruction   the instruction to dump
         * @param buffer        the chunk to contain the dump
         * @param offset        the offset of the instruction in the executable
         *
         * @return              SUCCESS if the dump was successful,
         *                      FAILED otherwise
         */
        status_t (*dump_intel)(disassembler_t *, instruction_t *, chunk_t *, uint64_t);

        /**
         * Decode a sequence of bytes to a string. The format can be modified
         * through the set_format function.
         *
         * @param bytes         instruction to decode
         * @param chunk         corresponding string
         * @return              STATUS of the decoding
         */
        status_t (*decode)(disassembler_t *, instruction_t **, chunk_t);

        /**
         * Encode a sequence of bytes to a string. The format can be modified
         * through the set_format function.
         *
         * @param bytes         string to encode
         * @param chunk         corresponding sequence of bytes
         * @return              STATUS of the encoding
         */
        status_t (*encode)(disassembler_t *, chunk_t *, instruction_t *);

        /**
         * Destroy a disassembler_t object.
         */
        void (*destroy)(disassembler_t *);
    };

# XED

The [XED disassembler from pintools][xed] always performed well on tricky instructions, so it was the first choice for nROP. It is the faster, but only support x86 and x86_64.

# Capstone

The new [capstone framework][capstone] go a step further by supporting numerous architecture through a convenient API. Howerer the solution is ongoing and does not provide the same speed as XED, and does not encode.

# Meta

The meta disassembler aims at using all previous disassemblers for better coverage. It is slower, but it is a nice way to compare the `decode()` results. For example, the [list of instructions decoded by xed and not by capstone][xedonly] reveals some missing handlers for the REX prefix and FPU-related opcodes. On the other side, the [list of instructions decoded by capstone and not by xed][caponly] is showing the AVX512 decoding, together with false positives (repz on FPU).

Here is a snippet of code to run the instruction bytes in gdb:

    int main()
    {
        // Decoded by capstone :: pminub xmm0, xmm10 :: 000000: \xf3\x66\x41\x0f\xda\xc2\xf3\x0f                         .fA.....
        char *code = "\xf3\x66\x41\x0f\xda\xc2\xf3\x0f";

        ((void (*)()) code)();

        return 0;
    }

Resulting here in a nice SIGILL :)
