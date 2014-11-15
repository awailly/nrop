/*
 * Plugin interface
 */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "code.h"

typedef struct plugin_t plugin_t;

struct plugin_t
{
    status_t (*apply)(plugin_t *);
    void (*destroy) (plugin_t *);
};

#endif
