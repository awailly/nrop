/*
 * Disassembler interface
 */

#ifndef __DISASSEMBLER_H__
#define __DISASSEMBLER_H__

#include "instruction.h"

typedef struct disassembler_t disassembler_t;

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

#endif
