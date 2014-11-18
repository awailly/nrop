/*
 * Implementation of disassembler for Intel XED
 *
 * Rev: 65163
 * pin-2.13-65163
 */

#ifndef __DISASSEMBLER_XED_H__
#define __DISASSEMBLER_XED_H__

#include "xed-interface.h"
#include "elf_type.h"
#include "disassembler.h"

typedef struct disass_xed_t disass_xed_t;

struct disass_xed_t
{
    disassembler_t interface;

    /**
     * Destroys a disass_xed_t object.
     */
    void (*destroy)(disass_xed_t);
};

disass_xed_t *create_xed();

#endif
