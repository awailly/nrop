/*
 * Parser interface
 */

#ifndef __PARSER_ELF_T_
#define __PARSER_ELF_T_

#include "parser.h"
#include <elf.h>
#include "elf_type.h"
#include "utils.h"

typedef struct parser_elf_t parser_elf_t;

struct parser_elf_t
{
    parser_t interface;
};

parser_elf_t *parser_elf_create();

#endif
