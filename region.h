/*
 * Region structure
 */

#ifndef __REGION_H__
#define __REGION_H__

#include "utils.h"
#include "chunk.h"

typedef struct region_t region_t;

struct region_t
{
    chunk_t (*get_chunk)(region_t* this);
    void (*destroy)(region_t* this);
};

region_t *region_create_file(char *filename);
region_t *region_create();

#endif
