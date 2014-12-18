/*
 * Parser interface
 */

#include "parser_pe.h"

typedef struct private_parser_pe_t private_parser_pe_t;

struct private_parser_pe_t
{
    parser_pe_t public;
    region_t *code;

    status_t (*check_region_32b)(region_t *);
    status_t (*check_region_64b)(region_t *);
    status_t (*check_region)(region_t *);
};

static status_t check_binary_32b(void* start) {
    return start == 0;
}

static status_t check_binary_64b(void* start) {
    return start == 0;
}

static status_t check_region_32b(region_t *region)
{
    return check_binary_32b(region->get_chunk(region).ptr);
}

static status_t check_region_64b(region_t *region)
{
    return check_binary_64b(region->get_chunk(region).ptr);
}

static status_t check_region(region_t *region)
{
    if (region == NULL)
        return FAILED;

    if (check_binary_32b(region->get_chunk(region).ptr) || 
        check_binary_64b(region->get_chunk(region).ptr))
    {
        return SUCCESS;
    }
    else
    {
        return FAILED;
    }
}

static code_t *parse(private_parser_pe_t *this, region_t *region)
{
    pe_t *code = create_pe(chunk_create((unsigned char *)"PE", 2), region);

    if (this->check_region_32b(region))
    {
        return (code_t*) code;
    }
    else if (this->check_region_64b(region))
    {
        return (code_t*) code;
    }
    else
    {
        code->destroy(code);
        return NULL;
    }
}

static void destroy(private_parser_pe_t *this)
{
    free(this);
    this = NULL;
}

parser_pe_t *parser_pe_create()
{
    private_parser_pe_t *this = malloc_thing(private_parser_pe_t);

    this->public.interface.check_region = (status_t (*)(region_t *)) check_region;
    this->public.interface.parse = (code_t *(*)(parser_t *, region_t *)) parse;
    this->public.interface.destroy = (void (*)(parser_t *)) destroy;

    this->check_region_32b = (status_t (*)(region_t *)) check_region_32b;
    this->check_region_64b = (status_t (*)(region_t *)) check_region_64b;

    return &this->public;
}
