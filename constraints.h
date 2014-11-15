/*
 * Define a constraints
 */

#ifndef __CONSTRAINTS_H_
#define __CONSTRAINTS_H_

#include "utils.h"
#include "xed-interface.h"
#include "linked_list.h"

typedef struct constraints_t constraints_t;

struct constraints_t
{
    void (*destroy)(constraints_t *);
};

constraints_t *constraints_create();
#endif
