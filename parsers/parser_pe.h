/*
 * Parser interface
 */

#ifndef __PARSER_PE_T_
#define __PARSER_PE_T_

#include "parser.h"
#include "pe_type.h"
#include "utils.h"

typedef struct parser_pe_t parser_pe_t;

struct parser_pe_t
{
    parser_t interface;
};

parser_pe_t *parser_pe_create();

#endif
