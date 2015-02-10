/*
 * Header for restore insertion plugin
 */

#ifndef __PLUGIN_RESTORE_H__
#define __PLUGIN_RESTORE_H__

#include "plugin.h"
#include "xed-interface.h"
#include <elf.h>
#include "elf_type.h"
#include "chain.h"

typedef struct plugin_restore_t plugin_restore_t;

struct plugin_restore_t
{
    plugin_t interface;
};

plugin_restore_t *plugin_restore_create(code_t *code);

#endif
