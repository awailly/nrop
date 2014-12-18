/*
 * Define a map
 */

#ifndef __MAP_H_
#define __MAP_H_

#include "utils.h"
#include "linked_list.h"
#include "chunk.h"
#include <z3.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/IPO.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>


typedef struct map_t map_t;

typedef enum
{
    UNKN,
    BAD,
    PN1,
    PN2,
    PC1,
    PC2,
    PC3
} gadget_type;

typedef struct Z3_symbol_cell
{
    LLVMValueRef valueref;
    Z3_ast symbol;
    Z3_context ctx;
    chunk_t name;
    unsigned int index;
    chunk_t prefix;
    bool is_global;
    int read_access;
    int write_access;
} Z3_symbol_cell;

struct map_t
{
    Z3_ast (*get_ast)(map_t *);
    linked_list_t *(*get_symbols)(map_t *);
    void (*dump)(map_t *);
    gadget_type (*compare)(map_t *, map_t *);
    void (*destroy)(map_t *);
};

map_t *map_create(Z3_context ctx, Z3_ast ast, linked_list_t *symbols);
#endif
