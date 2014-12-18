/*
 * Parser builder
 */

#ifndef __PARSER_BUILDER_H_
#define __PARSER_BUILDER_H_

#include "utils.h"
#include "parser.h"
#include "linked_list.h"

typedef struct parser_builder_t parser_builder_t;

struct parser_builder_t
{
    void (*add_parser)(parser_builder_t *, parser_t *);
    void (*remove_parser)(parser_builder_t *, parser_t *);
    enumerator_t *(*get_enumerator)(parser_builder_t *);
    void (*destroy)(parser_builder_t *);
};

parser_builder_t *parser_builder_create();

#endif
