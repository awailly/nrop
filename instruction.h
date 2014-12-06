/*
 * Define an instruction
 */

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

typedef struct instruction_t instruction_t;

struct instruction_t
{
    chunk_t bytes;
    chunk_t str;

    /**
     * Clone an instruction_t object. Is implemented by the disassembler.
     */
    instruction_t *(*clone)(instruction_t *);

    /**
     * Destroys an instruction_t object. Implemented by the disassembler.
     */
    void (*destroy)(instruction_t *);
};

typedef enum
{
    NO_CAT,
    COND_BR,
    UNCOND_BR,
    SYSCALL,
    CALL,
    RET
} category_t;

instruction_t *create_instruction(chunk_t bytes);

#endif
