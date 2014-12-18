/*
 * Parser builder
 */

#ifndef __PLUGIN_BUILDER_H_
#define __PLUGIN_BUILDER_H_

#include "utils.h"
#include "plugin.h"
#include "linked_list.h"

typedef struct plugin_builder_t plugin_builder_t;

struct plugin_builder_t
{
    void (*add_plugin)(plugin_builder_t *, plugin_t *);
    void (*remove_plugin)(plugin_builder_t *, plugin_t *);
    enumerator_t *(*get_enumerator)(plugin_builder_t *);
    void (*destroy)(plugin_builder_t *);
};

plugin_builder_t *plugin_builder_create();

#endif
