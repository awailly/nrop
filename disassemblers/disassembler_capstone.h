/*
 * Implementation of disassembler for Capstone
 *
 * 
 * 
 */

#ifndef __DISASSEMBLER_CAPSTONE_H__
#define __DISASSEMBLER_CAPSTONE_H__

#include <capstone/capstone.h>
#include "elf_type.h"
#include "disassembler.h"

#define DISASSINSTANCE create_capstone

typedef struct disass_capstone_t disass_capstone_t;

struct disass_capstone_t
{
    disassembler_t interface;

    /**
     * Destroys a disass_capstone_t object.
     */
    void (*destroy)(disass_capstone_t);
};

disass_capstone_t *create_capstone();

#endif
