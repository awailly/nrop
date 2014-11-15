/*
 * Implementation of constraints_t
 */

#include "constraints.h"

typedef struct private_constraints_t private_constraints_t;

struct private_constraints_t
{
    constraints_t public;

    linked_list_t *constraintsop;
};

static void destroy(private_constraints_t *this)
{
    free(this);
    this = NULL;
}

constraints_t *constraints_create()
{
    private_constraints_t *this = malloc_thing(private_constraints_t);

    this->public.destroy = (void (*)(constraints_t *)) destroy;

    return &this->public;
}
