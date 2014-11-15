/*
 * Header for junk insertion plugin
 */

#ifndef __PLUGIN_JUNK_H__
#define __PLUGIN_JUNK_H__

#include "plugin.h"
#include "xed-interface.h"
#include <elf.h>
#include "elf_type.h"
#include "chain.h"

typedef struct plugin_junk_t plugin_junk_t;

struct plugin_junk_t
{
    plugin_t interface;
};

plugin_junk_t *plugin_junk_create(code_t *code);

#endif
