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
     * Allocate an instruction with the size specific to the disassembler.
     * This is needed as every disassembler has its own vision of an
     * instruction.
     *
     * @return              a pointer to the allocated instruction
     */
    instruction_t *(*alloc_instruction)(disassembler_t *);

    /**
     * Clone an instruction with disassembler specific size. Meant to be
     * used with linked_list_t
     *
     * @param               a pointer to the instruction to clone
     * @return              the pointer toward the new instruction
     */
    void *(*clone_instruction)(void *instruction);

    /**
     * Destroy the instruction prevously allocated
     *
     * @param               a pointer to the instruction to destroy
     */
    void (*destroy_instruction)(void *instruction);

    /**
     * Return the size of disassembler instance of instruction. It is not
     * the size of the decoded instruction in bytes, but the internal
     * structure used by the disassembler.
     *
     * @return              the size of the expected instructions
     */
    uint64_t (*get_instruction_size)(disassembler_t *);

    /**
     * Destroy a disassembler_t object.
     */
    void (*destroy)(disassembler_t *);
};

#endif
