/*
 * Define an instruction
 */

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

typedef struct instruction_t instruction_t;

struct instruction_t
{
    /**
     * Add instruction bytes to instruction.
     *
     * @param insn      the byte to add.
     * @return          SUCCESS if the byte was successfully added, FAILED otherwise
     */
    status_t (*add_chunk)(instruction_t*, chunk_t);

    /**
     * Destroys an instruction_t object.
     */
    void (*destroy)(instruction_t *);
};

instruction_t *create_instruction(chunk_t bytes);

#endif
