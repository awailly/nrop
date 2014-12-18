/*
 * Implementation of disassembler with all disassemblers
 *
 */

#ifndef __DISASSEMBLER_META_H__
#define __DISASSEMBLER_META_H__

#include "elf_type.h"
#include "disassembler.h"

//#define DISASSINSTANCE create_meta

typedef struct disass_meta_t disass_meta_t;

struct disass_meta_t
{
    disassembler_t interface;

    /**
     * Destroys a disass_meta_t object.
     */
    void (*destroy)(disass_meta_t);
};

disass_meta_t *create_meta();

#endif
