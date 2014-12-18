/*
 * Used to convert tcg to llvm
 */

#ifndef __CONVERTER_H
#define __CONVERTER_H

#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/IPO.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>

/*
 * Using the -isystem qemu to make qemu headers warnings to disappear
 */
#include <include/exec/exec-all.h>
#include <tcg/tcg.h>

#include <z3.h>

#include "utils.h"
#include "chunk.h"
#include "map.h"
#include "linked_list.h"

typedef struct converter_t converter_t;

struct converter_t
{
    void (*tcg_to_llvm)(converter_t *);
    map_t *(*llvm_to_z3)(converter_t *);
    void (*dump)(converter_t *);
    void (*set_prefix)(converter_t *, chunk_t);

    void (*destroy)(converter_t *);
};

converter_t *converter_create(TCGContext *s, Z3_context ctx);

#endif
