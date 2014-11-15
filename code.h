/*
 * Parsed code from region
 */

#ifndef __CODE_H__
#define __CODE_H__

#include "utils.h"
#include "region.h"
#include "linked_list.h"

typedef struct code_t code_t;

struct code_t
{
    uint64_t entry;

    chunk_t (*get_type)(code_t*);
    region_t *(*get_region)(code_t*);
    status_t (*write_to_file)(code_t*, char*);

    void (*destroy)(code_t* this);
};

#endif
