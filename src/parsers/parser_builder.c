/*
 * Parser interface
 */

#include "parser_builder.h"

typedef struct private_parser_builder_t private_parser_builder_t;

struct private_parser_builder_t
{
    parser_builder_t public;

    linked_list_t *parsers_list;
};

static void add_parser(private_parser_builder_t *this, parser_t *parser)
{
    this->parsers_list->insert_last(this->parsers_list, parser);
}

static bool compare_parsers(void *a, void *b)
{
    return a == b;
}

static void remove_parser(private_parser_builder_t *this, parser_t *parser)
{
    this->parsers_list->remove(this->parsers_list, parser, compare_parsers);
}

static enumerator_t *get_enumerator(private_parser_builder_t *this)
{
    return this->parsers_list->create_enumerator(this->parsers_list);
}

static void destroy(private_parser_builder_t *this)
{
    this->parsers_list->destroy(this->parsers_list);
    free(this);
    this = NULL;
}

parser_builder_t *parser_builder_create()
{
    private_parser_builder_t *this = malloc_thing(private_parser_builder_t);

    this->public.add_parser = (void (*)(parser_builder_t *, parser_t *)) add_parser;
    this->public.remove_parser = (void (*)(parser_builder_t *, parser_t *)) remove_parser;
    this->public.get_enumerator = (enumerator_t *(*)(parser_builder_t *)) get_enumerator;
    this->public.destroy = (void (*)(parser_builder_t *)) destroy;

    this->parsers_list = linked_list_create();

    return &this->public;
}
