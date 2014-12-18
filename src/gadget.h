/*
 * Define ROP gadget, i.e. sequence of instructions
 */

#ifndef __GADGET_H__
#define __GADGET_H__

typedef struct gadget_t gadget_t;

struct gadget_t
{
    /**
     * Add instruction bytes to gadget.
     *
     * @param insn      the instruction_t to add.
     * @return          SUCCESS if the instruction was successfully added, FAILED otherwise
     */
    status_t (*add_instruction)(gadget_t*, instruction_t*);

    /**
     * Destroys an elf_t object.
     */
    void (*destroy)(gadget_t *);
};

gadget_t *create_gadget();

#endif
