/*
 * Header for rop insertion plugin
 */

#ifndef __PLUGIN_ROP_H__
#define __PLUGIN_ROP_H__

#include "plugin.h"
#include "xed-interface.h"
#include <elf.h>
#include "elf_type.h"
#include "constraints.h"
#include "chain.h"
#include "thpool.h"
#include "instruction.h"
#include "disassembler_xed.h"
#include "disassembler_capstone.h"

typedef struct plugin_rop_t plugin_rop_t;

struct plugin_rop_t
{
    plugin_t interface;
};

typedef struct th_arg th_arg;

struct th_arg
{
    chain_t *target;
    chain_t *c;
};

plugin_rop_t *plugin_rop_create(code_t *code, char *constraints, chunk_t target);

#endif
