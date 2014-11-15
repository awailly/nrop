/*
 * Parser interface
 */

#ifndef __PARSER_H_
#define __PARSER_H_

#include "utils.h"
#include "region.h"
#include "code.h"
/*#include "elf_type.h"*/

typedef struct parser_t parser_t;

struct parser_t
{
    status_t (*check_region)(region_t *);
    code_t *(*parse)(parser_t *, region_t *);
    void (*destroy)(parser_t *);
};

#endif
