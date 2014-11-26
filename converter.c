/*
 * Implementation
 */

#include "converter.h"

//#define DEBUG_LLVM
#ifdef DEBUG_LLVM
#  define LOG_LLVM(...) logging(__VA_ARGS__)
#else
#  define LOG_LLVM(...) do { } while (0)
#endif

typedef struct private_converter_t private_converter_t;

struct private_converter_t
{
    converter_t public;

    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMPassManagerRef pass_mgr;
    LLVMExecutionEngineRef exec_engine;
    LLVMValueRef Fn;

    int m_globalsIdx[TCG_MAX_TEMPS];
    LLVMValueRef m_values[TCG_MAX_TEMPS];
    LLVMValueRef m_memValuesPtr[TCG_MAX_TEMPS];
    LLVMBasicBlockRef m_labels[TCG_MAX_TEMPS];
    LLVMValueRef rip;
    LLVMValueRef df;

    TCGContext *s;

    Z3_context ctx;
    Z3_ast formula;
    linked_list_t *Z3_symbol_list;
    chunk_t prefix;

    void (*optimize_llvm)(private_converter_t *);
};

typedef struct TCGHelperInfo {
    void *func;
    const char *name;
    unsigned flags;
    unsigned sizemask;
} TCGHelperInfo;

static LLVMValueRef getValue(private_converter_t *this, int idx);

static LLVMTypeRef intType(int w)
{
    return LLVMIntType(w);
}

static LLVMTypeRef intPtrType(int w)
{
    return LLVMPointerType(intType(w), 0);
}

static LLVMTypeRef wordType()
{
    return intType(TCG_TARGET_REG_BITS);
}

static LLVMTypeRef tcgType(int type)
{
    return type == TCG_TYPE_I64 ? intType(64) : intType(32);
}

static LLVMTypeRef tcgPtrType(int type)
{
    return type == TCG_TYPE_I64 ? intPtrType(64) : intPtrType(32);
}

static LLVMValueRef getPtrForValue(private_converter_t *this, int idx)
{
    TCGTemp temp = this->s->temps[idx];

    /*LOG_LLVM("BILOU: %s %08x %08x %08x\n", temp.name, temp.mem_allocated, temp.type, idx);*/

    if (this->m_memValuesPtr[idx] == NULL)
    {
        if ((!temp.temp_allocated) && (!temp.temp_local)) /*((temp.mem_allocated) || (temp.fixed_reg))*/
        {
            /*
            LLVMValueRef indices[] = { LLVMConstInt(LLVMInt64Type(), this->m_globalsIdx[idx], 0) };
            LLVMValueRef v = LLVMConstGEP(LLVMGetFirstParam(this->Fn), indices, 1);
            this->m_memValuesPtr[idx] = LLVMBuildIntToPtr(this->builder, v, tcgPtrType(temp.type), temp.name);
            */
            LLVMValueRef v;

            v = LLVMAddGlobal(this->module, tcgType(temp.type), temp.name);

            //LLVMSetThreadLocal(v, 1);
            /* Filtering qemu cc_* */
            if ((temp.name[0] == 'c') && (temp.name[1] == 'c'))
            {
                LLVMSetInitializer(v, LLVMGetUndef(tcgType(temp.type)));
                LLVMSetLinkage(v, LLVMPrivateLinkage);
            }
            this->m_memValuesPtr[idx] = LLVMBuildPointerCast(this->builder, v, tcgPtrType(temp.type), temp.name);
        }
        else if (temp.temp_local)
        {
            this->m_memValuesPtr[idx] = LLVMBuildAlloca(this->builder, tcgType(temp.type), "alloca");
        }
        else
        {
            LLVMValueRef v = getValue(this, this->m_globalsIdx[idx]);
            v = LLVMBuildAdd(this->builder, v, LLVMConstInt(wordType(), temp.mem_offset, 0), "ADD");
            this->m_memValuesPtr[idx] = LLVMBuildIntToPtr(this->builder, v, tcgPtrType(temp.type), "i2p");
        }
    }

    return this->m_memValuesPtr[idx];
}

void delValue(private_converter_t *this, int idx)
{
    this->m_values[idx] = NULL;
}

static void setValue(private_converter_t *this, int idx, LLVMValueRef v)
{
    delValue(this, idx);
    this->m_values[idx] = v;

    if (idx < this->s->nb_globals)
    {
        /* For SSA
        this->m_memValuesPtr[idx] = NULL;
        */
        LLVMBuildStore(this->builder, v, getPtrForValue(this, idx));
    }
    else if (this->s->temps[idx].temp_local)
    {
        /* For SSA
        this->m_memValuesPtr[idx] = NULL;
        */
        LLVMBuildStore(this->builder, v, getPtrForValue(this, idx));
    }
}

static LLVMValueRef getValue(private_converter_t *this, int idx)
{
    if (this->m_values[idx] == NULL)
    {
        /*
        TCGTemp temp = this->s->temps[idx];
        */

        if (0) /*(temp.fixed_reg)*/
        {
            LLVMValueRef zero = LLVMConstInt(wordType(), 0, 0);
            this->m_values[idx] = LLVMBuildAdd(this->builder, zero, zero, "zorro");
        }
        else
            this->m_values[idx] = LLVMBuildLoad(this->builder, getPtrForValue(this, idx), "Lgv");
    }

    return this->m_values[idx];
}

static LLVMBasicBlockRef getLabel(private_converter_t *this, int idx)
{
    if (!this->m_labels[idx])
        this->m_labels[idx] = LLVMAppendBasicBlock(this->Fn, "getlabel");

    return this->m_labels[idx];
}

void delPtrForValue(private_converter_t *this, int idx)
{
    this->m_memValuesPtr[idx] = NULL;
}

static void startNewBasicBlock(private_converter_t *this, LLVMBasicBlockRef b)
{
    int i;

    if (!b)
        b = LLVMAppendBasicBlock(this->Fn, "startnew");
    
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(this->builder)))
        LLVMBuildBr(this->builder, b);

    LLVMPositionBuilderAtEnd(this->builder, b);

    /* Invalidate all temps */
    for(i=0; i<TCG_MAX_TEMPS; ++i)
        delValue(this, i);

    /* Invalidate all pointers to globals */
    for(i=0; i<this->s->nb_globals; ++i)
        delPtrForValue(this, i);
}

void adjustTypeSize_target(private_converter_t *this, unsigned target, LLVMValueRef *v)
{
    LLVMValueRef va = *v;
    if (target == 32)
    {
        /* Is buggy, need another way te determine if va is 64b*/
        if (LLVMTypeOf(va) == intType(64))
        {
            *v = LLVMBuildTrunc(this->builder, *v, intType(target), "adj");
        }
    }
}

void adjustTypeSize(private_converter_t *this, unsigned target, LLVMValueRef *v, LLVMValueRef *v2)
{
    adjustTypeSize_target(this, target, v);
    adjustTypeSize_target(this, target, v2);
}

/* Find helper name.  */
static inline const char *tcg_find_helper(TCGContext *s, uintptr_t val)
{
    const char *ret = NULL;
    if (s->helpers) {
        TCGHelperInfo *info = g_hash_table_lookup(s->helpers, (gpointer)val);
        if (info) {
            ret = info->name;
        }
    }
    return ret;
}

static void tcg_to_llvm(private_converter_t *this)
{
    TCGOpcode opc;
    int op_index;
    const TCGOpDef *def;
    const TCGArg *args;
    LLVMTypeRef FnType;
    LLVMTypeRef ParamTypes[1];
    LLVMBasicBlockRef bbi;
    int i;

    /*
    LLVMValueRef zero = LLVMConstInt(wordType(), 0, 0);
    */

    ParamTypes[0] = LLVMInt64Type();
    FnType = LLVMFunctionType(LLVMInt64Type(), ParamTypes, 1, 0);
    this->Fn = LLVMAddFunction(this->module, "F0cktion", FnType);
    LLVMSetFunctionCallConv(this->Fn, LLVMCCallConv);

    bbi = LLVMAppendBasicBlock(this->Fn, "entry");
    LLVMPositionBuilderAtEnd(this->builder, bbi);
    
    /*
    bbi = LLVMGetEntryBasicBlock(this->Fn);
    this->builder->SetInsertPoint(bb);*/

    // Allocate local temps
    for(i=this->s->nb_globals; i<TCG_MAX_TEMPS; ++i) {
        if(this->s->temps[i].temp_local) {
            //std::ostringstream pName;
            //pName << "loc_" << (i - s->nb_globals) << "ptr";
            char *name;
            if (this->s->temps[i].name)
                name = this->s->temps[i].name;
            else
                name = "ALLOA";
            this->m_memValuesPtr[i] = LLVMBuildAlloca(this->builder,
                tcgType(this->s->temps[i].type), name);
        }
    }


    args = this->s->gen_opparam_buf;
    op_index = 0;

    for(;;) {
        opc = this->s->gen_opc_buf[op_index++];
        def = &tcg_op_defs[opc];

        /*LOG_LLVM("Parsing %08x - %s\n", opc, def->name);*/

        LLVMValueRef v;
        int nb_args = def->nb_args;

        if (opc == 0)
            break;

        switch(opc) {
        case INDEX_op_debug_insn_start:
            break;

        /* predefined ops */
        case INDEX_op_nop:
        case INDEX_op_nop1:
        case INDEX_op_nop2:
        case INDEX_op_nop3:
            break;

        case INDEX_op_nopn:
            nb_args = args[0];
            break;

        case INDEX_op_discard:
            delValue(this, args[0]);
            break;

        case INDEX_op_call:
            {
                const char *kw;
                int nb_oargs = args[0] >> 16;
                int nb_iargs = args[0] & 0xffff;
                nb_args = nb_oargs + nb_iargs + def->nb_cargs + 1;

                /*LOG_LLVM(args[]);

                LLVMBuildRet(this->builder, LLVMConstInt(wordType(), 0, 0));*/
                kw = tcg_find_helper(this->s, args[nb_oargs + nb_iargs + 1]);

                if ((kw) && ((!strcmp(kw, "raise_exception")) || !strcmp(kw, "raise_interrupt") || !strcmp(kw, "pause")))
                {
                    LLVMBuildRet(this->builder, LLVMConstInt(wordType(), 0, 0));
                }
            }
            break;

        case INDEX_op_br:
            LLVMBuildBr(this->builder, getLabel(this, args[0]));
            startNewBasicBlock(this, NULL);
            break;

        case INDEX_op_movcond_i32:
            break;

        case INDEX_op_movcond_i64:
            break;

#define __OP_BRCOND_C(tcg_cond, cond)                               \
                case tcg_cond:                                          \
                    {\
                    LLVMValueRef v1, v2; \
                    v1 = getValue(this, args[0]); \
                    v2 = getValue(this, args[1]); \
                    if (LLVMTypeOf(v2) == intType(32))                                                    \
                        adjustTypeSize(this, 32, &v1, &v2);                             \
                    v = LLVMBuildICmp(this->builder, cond,                   \
                            v1, v2, "B");      \
                    }\
                break;

#define __OP_BRCOND(opc_name, bits)                                 \
        case opc_name: {                                                \
            assert(getValue(this, args[0])->getType() == intType(bits));      \
            assert(getValue(this, args[1])->getType() == intType(bits));      \
            switch(args[2]) {                                           \
                __OP_BRCOND_C(TCG_COND_EQ,  LLVMIntEQ)                        \
                __OP_BRCOND_C(TCG_COND_NE,  LLVMIntNE)                        \
                __OP_BRCOND_C(TCG_COND_LT,  LLVMIntSLT)                        \
                __OP_BRCOND_C(TCG_COND_GE,  LLVMIntSGE)                        \
                __OP_BRCOND_C(TCG_COND_LE,  LLVMIntSLE)                        \
                __OP_BRCOND_C(TCG_COND_GT,  LLVMIntSGT)                        \
                __OP_BRCOND_C(TCG_COND_LTU, LLVMIntULT)                        \
                __OP_BRCOND_C(TCG_COND_GEU, LLVMIntUGE)                        \
                __OP_BRCOND_C(TCG_COND_LEU, LLVMIntULE)                        \
                __OP_BRCOND_C(TCG_COND_GTU, LLVMIntUGT)                        \
                default:                                                \
                    tcg_abort();                                        \
            }                                                           \
            LLVMBasicBlockRef bb = LLVMAppendBasicBlock(this->Fn, "brcond");             \
            LLVMBuildCondBr(this->builder, v, getLabel(this, args[3]), bb);           \
            startNewBasicBlock(this, bb);                                     \
        } break;

        __OP_BRCOND(INDEX_op_brcond_i32, 32)

#if TCG_TARGET_REG_BITS == 64
        __OP_BRCOND(INDEX_op_brcond_i64, 64)
#endif

#undef __OP_BRCOND_C
#undef __OP_BRCOND

#define __OP_SETCOND_C(tcg_cond, cond)                              \
                case tcg_cond:                                          \
                    v = LLVMBuildICmp(this->builder,cond, v1, v2, "B");           \
                break;

#define __OP_SETCOND(opc_name, bits)                                \
        case opc_name: {                                                \
            LLVMValueRef  retptr = getPtrForValue(this, args[0]);                    \
            LLVMBuildLoad(this->builder,retptr, "Lsc");                  \
            LLVMValueRef  v1  = getValue(this, args[1]);                             \
            LLVMValueRef  v2  = getValue(this, args[2]);                             \
            assert(ret->getType() == intType(bits));                    \
            assert(v1->getType() == intType(bits));                     \
            assert(v2->getType() == intType(bits));                     \
            switch(args[3]) {                                           \
                __OP_SETCOND_C(TCG_COND_EQ,  LLVMIntEQ)                       \
                __OP_SETCOND_C(TCG_COND_NE,  LLVMIntNE)                       \
                __OP_SETCOND_C(TCG_COND_LT,  LLVMIntSLT)                       \
                __OP_SETCOND_C(TCG_COND_GE,  LLVMIntSGE)                       \
                __OP_SETCOND_C(TCG_COND_LE,  LLVMIntSLE)                       \
                __OP_SETCOND_C(TCG_COND_GT,  LLVMIntSGT)                       \
                __OP_SETCOND_C(TCG_COND_LTU, LLVMIntULT)                       \
                __OP_SETCOND_C(TCG_COND_GEU, LLVMIntUGE)                       \
                __OP_SETCOND_C(TCG_COND_LEU, LLVMIntULE)                       \
                __OP_SETCOND_C(TCG_COND_GTU, LLVMIntUGT)                       \
                default:                                                \
                    tcg_abort();                                        \
            }                                                           \
            LLVMBasicBlockRef bb = LLVMAppendBasicBlock(this->Fn, "setZero");  \
            LLVMBasicBlockRef bbSet = LLVMAppendBasicBlock(this->Fn, "setOne");\
            LLVMBasicBlockRef finished = LLVMAppendBasicBlock(this->Fn, "done");\
            LLVMBuildCondBr(this->builder, v, bbSet, bb);                       \
            LLVMPositionBuilderAtEnd(this->builder, bbSet); \
            setValue(this, args[0], LLVMConstInt(intType(bits), 1, 0));      \
            delValue(this, args[0]);                                          \
            LLVMBuildBr(this->builder, finished);                               \
            LLVMPositionBuilderAtEnd(this->builder, bb); \
            setValue(this, args[0], LLVMConstInt(intType(bits), 0, 0));      \
            delValue(this, args[0]);                                          \
            LLVMBuildBr(this->builder, finished);                               \
            LLVMPositionBuilderAtEnd(this->builder, finished); \
        } break;
        
        __OP_SETCOND(INDEX_op_setcond_i32, 32)
        
#if TCG_TARGET_REG_BITS == 64
        __OP_SETCOND(INDEX_op_setcond_i64, 64)
#endif

#undef __OP_SETCOND_C
#undef __OP_SETCOND

        case INDEX_op_set_label:
            startNewBasicBlock(this, getLabel(this, args[0]));
            break;

        case INDEX_op_movi_i32:
            setValue(this, args[0], LLVMConstInt(intType(32), args[1], 0));
            break;

        case INDEX_op_mov_i32:
            // Move operation may perform truncation of the value
            assert(getValue(this, args[1])->getType() == intType(32) ||
                    getValue(this, args[1])->getType() == intType(64));
            setValue(this, args[0],
                    LLVMBuildTrunc(this->builder, getValue(this, args[1]), intType(32), "T"));
            break;

#if TCG_TARGET_REG_BITS == 64
        case INDEX_op_movi_i64:
            {
            const char *kw;
            kw = tcg_find_helper(this->s, args[1]);
            if ((kw) && ((!strcmp(kw, "raise_exception")) || !strcmp(kw, "raise_interrupt") || !strcmp(kw, "pause")))
            {
                LLVMBuildRet(this->builder, LLVMConstInt(wordType(), 0, 0));
            }
            setValue(this, args[0], LLVMConstInt(intType(64), args[1], 0));
            }
            break;

        case INDEX_op_mov_i64:
            assert(getValue(this, args[1])->getType() == intType(64));
            LLVMValueRef v1 = getValue(this, args[1]);
            if (LLVMTypeOf(v1) == intType(32))
                v1 = LLVMBuildZExt(this->builder, v1, intType(64), "mviz");
            setValue(this, args[0], v1);
            break;
#endif

        /* size extensions */
#define __EXT_OP(opc_name, truncBits, opBits, signE )               \
        case opc_name:                                                  \
            /*                                                          \
            assert(getValue(this, args[1])->getType() == intType(opBits) ||   \
                   getValue(this, args[1])->getType() == intType(truncBits)); \
            */                                                          \
            setValue(this, args[0], LLVMBuild ## signE ## Ext(this->builder,         \
                    LLVMBuildTrunc(this->builder,                               \
                        getValue(this, args[1]), intType(truncBits), "T"),         \
                    intType(opBits), "S"));                                  \
            break;

        __EXT_OP(INDEX_op_ext8s_i32,   8, 32, S)
        __EXT_OP(INDEX_op_ext8u_i32,   8, 32, Z)
        __EXT_OP(INDEX_op_ext16s_i32, 16, 32, S)
        __EXT_OP(INDEX_op_ext16u_i32, 16, 32, Z)

#if TCG_TARGET_REG_BITS == 64
        __EXT_OP(INDEX_op_ext8s_i64,   8, 64, S)
        __EXT_OP(INDEX_op_ext8u_i64,   8, 64, Z)
        __EXT_OP(INDEX_op_ext16s_i64, 16, 64, S)
        __EXT_OP(INDEX_op_ext16u_i64, 16, 64, Z)
        __EXT_OP(INDEX_op_ext32s_i64, 32, 64, S)
        __EXT_OP(INDEX_op_ext32u_i64, 32, 64, Z)
#endif

#undef __EXT_OP

        /* load/store */
#define __LD_OP(opc_name, memBits, regBits, signE)                  \
        case opc_name:                                                  \
            assert(getValue(this, args[1])->getType() == wordType());         \
            /*
            if (this->s->temps[args[1]].fixed_reg)\
                v = LLVMBuildAdd(this->builder, zero, zero, "zorrosbak");\
            else*/\
            {\
            v = LLVMBuildAdd(this->builder, getValue(this, args[1]),                  \
                        LLVMConstInt(wordType(), args[2], 0), "AddLd");         \
            v = LLVMBuildIntToPtr(this->builder, v, intPtrType(memBits), "Ild");       \
            v = LLVMBuildLoad(this->builder, v, "Lld");                                \
            }\
            setValue(this, args[0], LLVMBuild ## signE ## Ext(this->builder,         \
                        v, intType(regBits), "Z"));                          \
            break;

#define __ST_OP(opc_name, memBits, regBits)                         \
        case opc_name:  {                                                 \
            assert(getValue(this, args[0])->getType() == intType(regBits));   \
            assert(getValue(this, args[1])->getType() == wordType());         \
            LLVMValueRef  valueToStore = getValue(this, args[0]);                    \
                                                                            \
            if ((this->s->temps[args[1]].fixed_reg) &&\
                (args[2] == 0x80))\
            { \
                if (this->rip == NULL)\
                {\
                    this->rip = LLVMAddGlobal(this->module, intType(64), "rip");\
                    LLVMSetThreadLocal(this->rip, 1);\
                }\
                v = this->rip;\
            }\
            else\
            {\
                                                                            \
                v = LLVMBuildAdd(this->builder, getValue(this, args[1]),                  \
                            LLVMConstInt(wordType(), args[2], 0), "AddSt");         \
                v = LLVMBuildIntToPtr(this->builder, v, intPtrType(memBits), "Ist");       \
            }\
            LLVMBuildStore(this->builder, LLVMBuildTrunc(this->builder,                 \
                    valueToStore, intType(memBits), "T"), v);           \
        } break;

        __LD_OP(INDEX_op_ld8u_i32,   8, 32, Z)
        __LD_OP(INDEX_op_ld8s_i32,   8, 32, S)
        __LD_OP(INDEX_op_ld16u_i32, 16, 32, Z)
        __LD_OP(INDEX_op_ld16s_i32, 16, 32, S)
        __LD_OP(INDEX_op_ld_i32,    32, 32, Z)

        __ST_OP(INDEX_op_st8_i32,   8, 32)
        __ST_OP(INDEX_op_st16_i32, 16, 32)
        __ST_OP(INDEX_op_st_i32,   32, 32)

#define __LD_OP_QEMU(opc_name, regBits) \
        case opc_name:\
        {\
            v = getValue(this, args[1]); \
            v = LLVMBuildIntToPtr(this->builder, v, intPtrType(regBits), "Ildq");       \
            v = LLVMBuildLoad(this->builder, v, "Ldq"); \
            /*setValue(this, args[0], v);*/ \
            setValue(this, args[0], LLVMBuildZExt(this->builder, v, intType(regBits), "Lz")); \
        }\
        break;

#define __ST_OP_QEMU(opc_name, regBits) \
        case opc_name:\
        {\
            LLVMValueRef  valueToStore = getValue(this, args[0]);                    \
            v = getValue(this, args[1]);                                             \
            v = LLVMBuildIntToPtr(this->builder, v, intPtrType(regBits), "Istq");       \
            LLVMBuildStore(this->builder, LLVMBuildTrunc(this->builder, valueToStore, intType(regBits), "Tq"), v); \
        }\
        break;

        __LD_OP_QEMU(INDEX_op_qemu_ld_i32, 32)
        __ST_OP_QEMU(INDEX_op_qemu_st_i32, 32)

#if TCG_TARGET_REG_BITS == 64
        __LD_OP(INDEX_op_ld8u_i64,   8, 64, Z)
        __LD_OP(INDEX_op_ld8s_i64,   8, 64, S)
        __LD_OP(INDEX_op_ld16u_i64, 16, 64, Z)
        __LD_OP(INDEX_op_ld16s_i64, 16, 64, S)
        __LD_OP(INDEX_op_ld32u_i64, 32, 64, Z)
        __LD_OP(INDEX_op_ld32s_i64, 32, 64, S)
        __LD_OP(INDEX_op_ld_i64,    64, 64, Z)

        __ST_OP(INDEX_op_st8_i64,   8, 64)
        __ST_OP(INDEX_op_st16_i64, 16, 64)
        __ST_OP(INDEX_op_st32_i64, 32, 64)
        __ST_OP(INDEX_op_st_i64,   64, 64)

        __LD_OP_QEMU(INDEX_op_qemu_ld_i64, 64)
        __ST_OP_QEMU(INDEX_op_qemu_st_i64, 64)
#endif

#undef __LD_OP
#undef __ST_OP
#undef __LD_OP_QEMU
#undef __ST_OP_QEMU

        /* arith */
#define __ARITH_OP(opc_name, op, bits)                              \
        case opc_name: {                                                \
            LLVMValueRef v1 = getValue(this, args[1]);                              \
            if (LLVMTypeOf(v1) == intPtrType(bits)) \
                v1 = LLVMBuildPtrToInt(this->builder, v1, intType(bits), "P2i");\
                                                                        \
            LLVMValueRef v2 = getValue(this, args[2]);                  \
            if (LLVMTypeOf(v2) == intPtrType(bits)) \
                v2 = LLVMBuildPtrToInt(this->builder, v2, intType(bits), "p2I");\
            adjustTypeSize(this, bits, &v1, &v2);                             \
            assert(v1->getType() == intType(bits));                     \
            assert(v2->getType() == intType(bits));                     \
            setValue(this, args[0], LLVMBuild ## op(this->builder, v1, v2, "Oarith"));          \
        } break;

#define __ARITH_OP_MUL2(opc_name, signE, bits) \
        case opc_name: {\
            LLVMValueRef v1 = getValue(this, args[2]);\
            LLVMValueRef v2 = getValue(this, args[3]);\
            uint ## bits ## _t mask;\
            mask = -1;\
            v1 = LLVMBuildZExt(this->builder, v1, intType(2*bits), "Z"); \
            v2 = LLVMBuildZExt(this->builder, v2, intType(2*bits), "Z"); \
            v = LLVMBuildMul(this->builder, v1, v2, "Mul");\
                           \
            v1 = LLVMBuildAnd(this->builder, v, LLVMConstInt(intType(2*bits), mask, 0), "Andm");\
            v1 = LLVMBuildTrunc(this->builder, v1, intType(bits), "Tand");\
            setValue(this, args[0], v1);\
            v1 = LLVMBuildLShr(this->builder, v,LLVMConstInt(intType(2*bits), bits, 0), "Shrm");\
            v1 = LLVMBuildTrunc(this->builder, v1, intType(bits), "Tshr");\
            setValue(this, args[1], v1);\
        } break;

#define __ARITH_OP_DIV2(opc_name, signE, bits)                      \
        case opc_name:                                                  \
            assert(getValue(this, args[2])->getType() == intType(bits));      \
            assert(getValue(this, args[3])->getType() == intType(bits));      \
            assert(getValue(this, args[4])->getType() == intType(bits));      \
            v = LLVMBuildShl(this->builder,                                     \
                    LLVMBuildZExt(this->builder,                                \
                        getValue(this, args[3]), intType(bits*2), "ZEXT"),            \
                    LLVMBuildZExt(this->builder,                                \
                        LLVMConstInt(intType(bits), bits, 0),          \
                        intType(bits*2), "ZEXTE"), "SHL");                              \
            v = LLVMBuildOr(this->builder, v,                                   \
                    LLVMBuildZExt(this->builder,                                \
                        getValue(this, args[2]), intType(bits*2)));           \
            setValue(this, args[0], LLVMBuild ## signE ## Div(this->builder,         \
                    v, getValue(this, args[4])));                             \
            setValue(this, args[1], LLVMBuild ## signE ## Rem(this->builder,         \
                    v, getValue(this, args[4])));                             \
            break;

#define __ARITH_OP_ROT(opc_name, op1, op2, bits)                    \
        case opc_name:                                                  \
            assert(getValue(this, args[1])->getType() == intType(bits));      \
            assert(getValue(this, args[2])->getType() == intType(bits));      \
            v = LLVMBuildSub(this->builder,                                     \
                    LLVMConstInt(intType(bits), bits, 0),              \
                    getValue(this, args[2]), "S");                                 \
            setValue(this, args[0], LLVMBuildOr(this->builder,                        \
                    LLVMBuild ## op1 (this->builder,                           \
                        getValue(this, args[1]), getValue(this, args[2]), "ROT"),          \
                    LLVMBuild ## op2 (this->builder,                           \
                        getValue(this, args[1]), v, "ROTR"), "OR"));                        \
            break;

#define __ARITH_OP_I(opc_name, op, i, bits)                         \
        case opc_name:                                                  \
            assert(getValue(this, args[1])->getType() == intType(bits));      \
            setValue(this, args[0], LLVMBuild ## op(this->builder,                   \
                        LLVMConstInt(intType(bits), i, 0),             \
                        getValue(this, args[1]), "XOR"));                            \
            break;

#define __ARITH_OP_BSWAP(opc_name, sBits, bits)                     \
        case opc_name: {                                                \
                           LOG_LLVM("You have to do bswap, sorrf baby :(\n"); \
            } break;


        __ARITH_OP(INDEX_op_add_i32, Add, 32)
        __ARITH_OP(INDEX_op_sub_i32, Sub, 32)
        __ARITH_OP(INDEX_op_mul_i32, Mul, 32)
        __ARITH_OP_MUL2(INDEX_op_mulu2_i32, Z, 32)
        __ARITH_OP_MUL2(INDEX_op_muls2_i32, S, 32)

#ifdef TCG_TARGET_HAS_div_i32
        __ARITH_OP(INDEX_op_div_i32,  SDiv, 32)
        __ARITH_OP(INDEX_op_divu_i32, UDiv, 32)
        __ARITH_OP(INDEX_op_rem_i32,  SRem, 32)
        __ARITH_OP(INDEX_op_remu_i32, URem, 32)
#else
        __ARITH_OP_DIV2(INDEX_op_div2_i32,  S, 32)
        __ARITH_OP_DIV2(INDEX_op_divu2_i32, U, 32)
#endif

        __ARITH_OP(INDEX_op_and_i32, And, 32)
        __ARITH_OP(INDEX_op_or_i32,   Or, 32)
        __ARITH_OP(INDEX_op_xor_i32, Xor, 32)

        __ARITH_OP(INDEX_op_shl_i32,  Shl, 32)
        __ARITH_OP(INDEX_op_shr_i32, LShr, 32)
        __ARITH_OP(INDEX_op_sar_i32, AShr, 32)

        __ARITH_OP_ROT(INDEX_op_rotl_i32, Shl, LShr, 32)
        __ARITH_OP_ROT(INDEX_op_rotr_i32, LShr, Shl, 32)

        __ARITH_OP_I(INDEX_op_not_i32, Xor, (uint32_t) -1, 32)
        __ARITH_OP_I(INDEX_op_neg_i32, Sub, 0, 32)

        __ARITH_OP_BSWAP(INDEX_op_bswap16_i32, 16, 32)
        __ARITH_OP_BSWAP(INDEX_op_bswap32_i32, 32, 32)

#if TCG_TARGET_REG_BITS == 64
        __ARITH_OP(INDEX_op_add_i64, Add, 64)
        __ARITH_OP(INDEX_op_sub_i64, Sub, 64)
        __ARITH_OP(INDEX_op_mul_i64, Mul, 64)
        __ARITH_OP_MUL2(INDEX_op_mulu2_i64, Z, 64)
        __ARITH_OP_MUL2(INDEX_op_muls2_i64, S, 64)

#ifdef TCG_TARGET_HAS_div_i64
        __ARITH_OP(INDEX_op_div_i64,  SDiv, 64)
        __ARITH_OP(INDEX_op_divu_i64, UDiv, 64)
        __ARITH_OP(INDEX_op_rem_i64,  SRem, 64)
        __ARITH_OP(INDEX_op_remu_i64, URem, 64)
#else
        __ARITH_OP_DIV2(INDEX_op_div2_i64,  S, 64)
        __ARITH_OP_DIV2(INDEX_op_divu2_i64, U, 64)
#endif

        __ARITH_OP(INDEX_op_and_i64, And, 64)
        __ARITH_OP(INDEX_op_or_i64,   Or, 64)
        __ARITH_OP(INDEX_op_xor_i64, Xor, 64)

        __ARITH_OP(INDEX_op_shl_i64,  Shl, 64)
        __ARITH_OP(INDEX_op_shr_i64, LShr, 64)
        __ARITH_OP(INDEX_op_sar_i64, AShr, 64)

        __ARITH_OP_ROT(INDEX_op_rotl_i64, Shl, LShr, 64)
        __ARITH_OP_ROT(INDEX_op_rotr_i64, LShr, Shl, 64)

        __ARITH_OP_I(INDEX_op_not_i64, Xor, (uint64_t) -1, 64)
        __ARITH_OP_I(INDEX_op_neg_i64, Sub, 0, 64)

        __ARITH_OP_BSWAP(INDEX_op_bswap16_i64, 16, 64)
        __ARITH_OP_BSWAP(INDEX_op_bswap32_i64, 32, 64)
        __ARITH_OP_BSWAP(INDEX_op_bswap64_i64, 64, 64)
#endif

#undef __ARITH_OP_BSWAP
#undef __ARITH_OP_I
#undef __ARITH_OP_ROT
#undef __ARITH_OP_DIV2
#undef __ARITH_OP

        /* QEMU specific */
#if TCG_TARGET_REG_BITS == 64

#define __OP_QEMU_ST(opc_name, bits)                                \
        case opc_name:                                                  \
            generateQemuMemOp(this, false,                                    \
                LLVMBuildIntCast(this->builder,                                 \
                    getValue(this, args[0]), intType(bits), false),           \
                getValue(this, args[1]), args[2], bits);                      \
            break;


#define __OP_QEMU_LD(opc_name, bits, signE)                         \
        case opc_name:                                                  \
            v = generateQemuMemOp(this, true, NULL,                           \
                getValue(this, args[1]), args[2], bits);                      \
            setValue(this, args[0], LLVMBuild ## signE ## Ext(this->builder,         \
                v, intType(TARGET_LONG_BITS), "LD"));         \
            break;

#define __OP_QEMU_LDD(opc_name, bits)                               \
        case opc_name:                                                  \
            v = generateQemuMemOp(this, true, NULL,                           \
                getValue(this, args[1]), args[2], bits);                      \
            setValue(this, args[0], v);         \
            break;

        /*
        __OP_QEMU_ST(INDEX_op_qemu_st8,   8)
        __OP_QEMU_ST(INDEX_op_qemu_st16, 16)
        __OP_QEMU_ST(INDEX_op_qemu_st32, 32)
        __OP_QEMU_ST(INDEX_op_qemu_st64, 64)

        __OP_QEMU_LD(INDEX_op_qemu_ld8s,   8, S)
        __OP_QEMU_LD(INDEX_op_qemu_ld8u,   8, Z)
        __OP_QEMU_LD(INDEX_op_qemu_ld16s, 16, S)
        __OP_QEMU_LD(INDEX_op_qemu_ld16u, 16, Z)
        __OP_QEMU_LD(INDEX_op_qemu_ld32s, 32, S)
        __OP_QEMU_LD(INDEX_op_qemu_ld32u, 32, Z)
        __OP_QEMU_LD(INDEX_op_qemu_ld64,  64, Z)

        __OP_QEMU_LDD(INDEX_op_qemu_ld32, 32)
        */

#undef __OP_QEMU_LD
#undef __OP_QEMU_ST
#undef __OP_QEMU_LDD

#endif

        case INDEX_op_exit_tb:
            LLVMBuildRet(this->builder, LLVMConstInt(wordType(), args[0], 0));
            break;

        case INDEX_op_goto_tb:
            /* XXX: tb linking is disabled */
            break;

        case INDEX_op_deposit_i32: {
            //llvm::errs() << *m_tbFunction << "\n";
            LLVMValueRef arg1 = getValue(this, args[1]);
            //llvm::errs() << "arg1=" << *arg1 << "\n";
            //arg1 = LLVMBuildTrunc(arg1, intType(32));


            LLVMValueRef arg2 = getValue(this, args[2]);
            //llvm::errs() << "arg2=" << *arg2 << "\n";
            arg2 = LLVMBuildTrunc(this->builder, arg2, intType(32), "T");

            uint32_t ofs = args[3];
            uint32_t len = args[4];
            /*
             * uint64_t temp_val;
             */

            if (ofs == 0 && len == 32) {
                setValue(this, args[0], arg2);
                break;
            }

            uint32_t mask = (1u << len) - 1;
            LLVMValueRef t1, ret;
            if (ofs + len < 32) {
                t1 = LLVMBuildAnd(this->builder, arg2, LLVMConstInt(intType(32), mask, 0), "AddDepositC");
                t1 = LLVMBuildShl(this->builder, t1, LLVMConstInt(intType(32), ofs, 0), "S");
                /*
                temp_val = ((args[2] & mask) << ofs);
                */
            } else {
                t1 = LLVMBuildShl(this->builder, arg2, LLVMConstInt(intType(32), ofs, 0), "S");
                /*
                temp_val = (args[2] << ofs);
                */
            }

            ret = LLVMBuildAnd(this->builder, arg1, LLVMConstInt(intType(32), ~(mask << ofs), 0), "AddDeposit");
            ret = LLVMBuildOr(this->builder, ret, t1, "Odep");
            /*
            ret = LLVMConstInt(intType(32), ((args[1] & ~(mask << ofs)) | temp_val), 0);
            */
            setValue(this, args[0], ret);
        }
        break;

        case INDEX_op_deposit_i64: {
            //llvm::errs() << *m_tbFunction << "\n";
            LLVMValueRef arg1 = getValue(this, args[1]);
            //llvm::errs() << "arg1=" << *arg1 << "\n";
            //arg1 = LLVMBuildTrunc(arg1, intType(32));


            LLVMValueRef arg2 = getValue(this, args[2]);
            //llvm::errs() << "arg2=" << *arg2 << "\n";
            arg2 = LLVMBuildTrunc(this->builder, arg2, intType(64), "T");

            uint64_t ofs = args[3];
            uint64_t len = args[4];
            /*
            uint64_t temp_val;
            */

            if (ofs == 0 && len == 64)
            {
                setValue(this, args[0], arg2);
                break;
            }

            uint64_t mask = (1u << len) - 1;
            LLVMValueRef t1, ret;
            if (ofs + len < 64) {
                t1 = LLVMBuildAnd(this->builder, arg2, LLVMConstInt(intType(64), mask, 0), "AddDepositCx");
                t1 = LLVMBuildShl(this->builder, t1, LLVMConstInt(intType(64), ofs, 0), "S");
                /*
                temp_val = ((args[2] & mask) << ofs);
                */
            } else {
                t1 = LLVMBuildShl(this->builder, arg2, LLVMConstInt(intType(64), ofs, 0), "S");
                /*
                temp_val = (args[2] << ofs);
                */
            }

            ret = LLVMBuildAnd(this->builder, arg1, LLVMConstInt(intType(64), ~(mask << ofs), 0), "AddDepositx");
            ret = LLVMBuildOr(this->builder, ret, t1, "O");
            /*
            ret = LLVMConstInt(intType(64), ((args[1] & ~(mask << ofs)) | temp_val), 0);
            */
            setValue(this, args[0], ret);
        }
        break;

        default:
            LOG_LLVM("ERROR: unknown TCG micro operation %s (%08x|%08x)\n", def->name, INDEX_op_qemu_ld_i64, INDEX_op_end);
            tcg_abort();
            break;
        }

        args+= nb_args;
    }

    /*LLVMBuildRet(this->builder, LLVMConstInt(wordType(), 0, 0));*/

    this->optimize_llvm(this);
}

static void optimize_llvm(private_converter_t *this)
{
    char *error;

    error = NULL;

    LOG_LLVM("Verification of the module\n");
    //this->public.dump(&this->public);
    /*LLVMVerifyModule(this->module, LLVMPrintMessageAction, &error);*/
    LLVMVerifyModule(this->module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);

    /*
    LLVMModuleProviderRef provider;

    provider = LLVMCreateModuleProviderForExistingModule(this->module);

    if (LLVMCreateJITCompiler(&this->exec_engine, provider, 2, &error) != 0)
    {
        LOG_LLVM("Error while LLVMCreateExecutionEngineForModule in converter_create: %s\n", error);
        LLVMDisposeMessage (error);
        abort();
    }*/

    /*this->pass_mgr =  LLVMCreateFunctionPassManagerForModule (this->module);*/
    this->pass_mgr = LLVMCreatePassManager();
    /*
    LLVMAddTargetData (LLVMGetExecutionEngineTargetData (this->exec_engine),
            this->pass_mgr);*/
    LLVMAddBasicAliasAnalysisPass (this->pass_mgr);
    LLVMAddTypeBasedAliasAnalysisPass (this->pass_mgr);
    LLVMAddScalarReplAggregatesPassSSA (this->pass_mgr);
    LLVMAddPromoteMemoryToRegisterPass (this->pass_mgr);
    LLVMAddIPConstantPropagationPass (this->pass_mgr);
    LLVMAddGVNPass (this->pass_mgr);
    LLVMAddInstructionCombiningPass (this->pass_mgr);
    LLVMAddReassociatePass (this->pass_mgr);
    LLVMAddCFGSimplificationPass (this->pass_mgr);
    LLVMAddDeadStoreEliminationPass(this->pass_mgr);
    LLVMAddAggressiveDCEPass(this->pass_mgr);
    LLVMAddEarlyCSEPass (this->pass_mgr);
    LLVMAddConstantPropagationPass(this->pass_mgr);
    LLVMAddIndVarSimplifyPass(this->pass_mgr);
    LLVMAddLICMPass(this->pass_mgr);
    LLVMAddSCCPPass(this->pass_mgr);
    LLVMAddCorrelatedValuePropagationPass(this->pass_mgr);

    LLVMAddScalarReplAggregatesPass (this->pass_mgr);
    LLVMAddConstantMergePass (this->pass_mgr);
    LLVMAddGlobalDCEPass (this->pass_mgr);
    LLVMAddGlobalOptimizerPass(this->pass_mgr);
    LLVMAddPruneEHPass(this->pass_mgr);
    LLVMAddConstantPropagationPass(this->pass_mgr);
    LLVMAddInstructionCombiningPass (this->pass_mgr);
    /*LLVMInitializeFunctionPassManager (this->pass_mgr);*/

    /*LOG_LLVM("Running pass manager\n");*/
    if (1)
        LLVMRunPassManager(this->pass_mgr, this->module);
}

static void dump(private_converter_t *this)
{
    /*this->optimize_llvm(this);*/
    char *msg;
    
    msg = LLVMPrintModuleToString(this->module);

    logging("Dumping\n");
    /*LLVMDumpModule(this->module);*/
    logging("%s\n", msg);
}

/**
   \brief Create a variable using the given name and type.
*/
Z3_ast mk_var(Z3_context ctx, const char * name, Z3_sort ty) 
{
    Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
    Z3_ast a = Z3_mk_const(ctx, s, ty);
    /*
    printf("inc ref for %s\n", name);
    Z3_inc_ref(ctx, a);
    printf("for %s\n", name);
    */
    return a;
}

/**
   \brief Create a boolean variable using the given name.
*/
Z3_ast mk_bool_var(Z3_context ctx, const char * name) 
{
    Z3_sort ty = Z3_mk_bool_sort(ctx);
    return mk_var(ctx, name, ty);
}

/**
   \brief Create an integer variable using the given name.
*/
Z3_ast mk_int_var(Z3_context ctx, const char * name) 
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return mk_var(ctx, name, ty);
}

/**
   \brief Create a Z3 integer node using a C int. 
*/
Z3_ast mk_int(Z3_context ctx, int v) 
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return Z3_mk_int(ctx, v, ty);
}

/**
   \brief Create a real variable using the given name.
*/
Z3_ast mk_real_var(Z3_context ctx, const char * name) 
{
    Z3_sort ty = Z3_mk_real_sort(ctx);
    return mk_var(ctx, name, ty);
}

uint64_t get_llvm_type_size(LLVMValueRef valueref)
{
    uint64_t valueref_size;

    if (LLVMTypeOf(valueref) == intType(128) || (LLVMTypeOf(valueref) == intPtrType(128)))
        valueref_size = 128;
    else if (LLVMTypeOf(valueref) == intType(64) || (LLVMTypeOf(valueref) == intPtrType(64)))
        valueref_size = 64;
    else if (LLVMTypeOf(valueref) == intType(32) || (LLVMTypeOf(valueref) == intPtrType(32)))
        valueref_size = 32;
    else if (LLVMTypeOf(valueref) == intType(16) || (LLVMTypeOf(valueref) == intPtrType(16)))
        valueref_size = 16;
    else if (LLVMTypeOf(valueref) == intType(8) || (LLVMTypeOf(valueref) == intPtrType(8)))
        valueref_size = 8;
    else if (LLVMTypeOf(valueref) == intType(1) || (LLVMTypeOf(valueref) == intPtrType(1)))
        valueref_size = 1;
    else
    {
        logging("Size not supported (%x) in create_Z3_var_internal\n", LLVMTypeOf(valueref));
        logging("int64:%08x pint64:%08x int32:%08x pint32:%08x\n", intType(64), intPtrType(64), intType(32), intPtrType(32));
        logging("int16:%08x pint16:%08x int8:%08x pint8:%08x\n", intType(16), intPtrType(16), intType(8), intPtrType(8));
        valueref_size = 0;
    }

    return valueref_size;
}

uint64_t get_z3_size(private_converter_t *this, Z3_ast z3value)
{
    Z3_sort sort;

    sort = Z3_get_sort(this->ctx, z3value);

    if (Z3_get_sort_kind(this->ctx, sort) != Z3_BV_SORT)
        return 1;

    return Z3_get_bv_sort_size(this->ctx, sort);
}

Z3_ast create_Z3_var_internal(private_converter_t *this, LLVMValueRef valueref, int need_inc, int read_access)
{
    Z3_ast res;
    Z3_sort bv_sort;
    Z3_context ctx;

    LLVMTypeKind type;
    uint64_t valueref_size;

    enumerator_t *e;
    Z3_symbol_cell *c, *target_cell;

    target_cell = NULL;
    ctx = this->ctx;

    valueref_size = get_llvm_type_size(valueref);

    bv_sort = Z3_mk_bv_sort(ctx, valueref_size);

    e = this->Z3_symbol_list->create_enumerator(this->Z3_symbol_list);

    while ((target_cell == NULL) && (e->enumerate(e, &c)))
        if (valueref == c->valueref)
            target_cell = c;

    e->destroy(e);

    if (target_cell)
    {
        target_cell->write_access = need_inc;

        if (need_inc)
        {
            chunk_t new_chunk;

            target_cell->index++;

            new_chunk = chunk_calloc(255);
            snprintf((char*) new_chunk.ptr, new_chunk.len, "%s%i", target_cell->name.ptr, target_cell->index);

            if (target_cell->prefix.ptr)
                new_chunk = chunk_cat("cm", target_cell->prefix, new_chunk);

            LOG_LLVM("Creating incremented z3 variable @%x[%s]\n", new_chunk.ptr, new_chunk.ptr);
            target_cell->symbol = mk_var(ctx, (char*)new_chunk.ptr, bv_sort);

            chunk_clear(&new_chunk);
        }

        res = target_cell->symbol;
    }
    else
    {
        bool need_final_free;

        need_final_free = false;

        if ((target_cell = malloc(sizeof(*target_cell))) == NULL)
            LOG_LLVM("Error while allocating target_cell in create_Z3_var_internal\n");

        target_cell->valueref = valueref;
        target_cell->index = 0;
        target_cell->name = chunk_empty;
        target_cell->prefix = chunk_clone(this->prefix);
        target_cell->is_global = LLVMIsAGlobalValue(valueref);
        target_cell->read_access = read_access;
        target_cell->write_access = need_inc;

        type = LLVMGetTypeKind(LLVMTypeOf(valueref));

        /* FIXME Refactor */
        switch (type)
        {
            /* 
             * This is either a Void, a ConstInt or a Named integer (%bla)
             */
            case LLVMVoidTypeKind:
            {
                const char *name;
                chunk_t final_name;

                name = LLVMGetValueName(valueref);
                target_cell->name = chunk_calloc(strlen(name)+1);
                strncpy((char*)target_cell->name.ptr, name, target_cell->name.len);
                final_name = target_cell->name;

                if (this->prefix.ptr)
                {
                    final_name = chunk_cat("cc", this->prefix, target_cell->name);
                    need_final_free = true;
                }

                res = mk_var(ctx, (char*)final_name.ptr, bv_sort);

                if (need_final_free)
                    chunk_free(&final_name);

                break;
            }
            case LLVMIntegerTypeKind:
            {
                /*
                 * ConstInt: dst = src
                 */
                if (LLVMIsAConstantInt(valueref))
                    res = Z3_mk_unsigned_int64(ctx, LLVMConstIntGetZExtValue(valueref), bv_sort);
                /*
                 * Named integer: dst = src
                 */
                else
                {
                    const char *name;
                    chunk_t final_name;

                    name = LLVMGetValueName(valueref);

                    if (*name == 0)
                        name = "_";

                    target_cell->name = chunk_calloc(strlen(name)+1);
                    strncpy((char*)target_cell->name.ptr, name, target_cell->name.len);
                    final_name = target_cell->name;

                    if (this->prefix.ptr)
                    {
                        final_name = chunk_cat("cc", this->prefix, target_cell->name);
                        need_final_free = true;
                    }

                    res = mk_var(ctx, (char*)final_name.ptr, bv_sort);

                    if (need_final_free)
                        chunk_free(&final_name);
                    }

                break;
            }
            /*
             * This is a PointerInt (*bla)
             */
            case LLVMPointerTypeKind:
            {
                const char *name;
                chunk_t final_name;

                name = LLVMGetValueName(valueref);
                target_cell->name = chunk_calloc(strlen(name)+1);
                strncpy((char*)target_cell->name.ptr, name, target_cell->name.len);
                final_name = target_cell->name;

                if (this->prefix.ptr)
                {
                    final_name = chunk_cat("cc", this->prefix, target_cell->name);
                    need_final_free = true;
                }

                res = mk_var(ctx, (char*)final_name.ptr, bv_sort);

                if (need_final_free)
                    chunk_free(&final_name);
                break;
            }
            default:
            {
                LOG_LLVM("       Found something BAD: %x\n", type);
                res = NULL;
            }
        }

        target_cell->symbol = res;
        this->Z3_symbol_list->insert_last(this->Z3_symbol_list, target_cell);
    }

    if (res == NULL)
        LOG_LLVM("BEWARE, RETURNING NULL\n");

    return res;
}

Z3_ast create_Z3_var(private_converter_t *this, LLVMValueRef valueref)
{
    return create_Z3_var_internal(this, valueref, 0, 0);
}

Z3_ast create_Z3_var_read(private_converter_t *this, LLVMValueRef valueref)
{
    return create_Z3_var_internal(this, valueref, 0, 1);
}

Z3_ast create_Z3_var_inc(private_converter_t *this, LLVMValueRef valueref)
{
    return create_Z3_var_internal(this, valueref, 1, 0);
}

void error_handler(Z3_context c, Z3_error_code e)
{
    if (!c)
        printf("No context in error_handler\n");

    printf("[Z3 error handler]\n");
    printf("Error code: %d\n", e);
    printf("Error msg : %s\n", Z3_get_error_msg_ex(c, e));
    exit(0);
}

static map_t *llvm_to_z3(private_converter_t *this)
{
    LLVMBasicBlockRef bb;
    LLVMValueRef insn;
    LLVMOpcode opcode;
    LLVMValueRef operand;
    LLVMTypeKind type;

    Z3_ast store_name, Z3_res, args[2];
    Z3_sort bv_sort, array_sort;
    Z3_sort bv_sort_32, array_sort_32;
    /* Z3_ast ram; */
    Z3_symbol_cell *ram;
    Z3_symbol_cell *env;

    int number_of_operands;
    int operand_num;

    bv_sort = Z3_mk_bv_sort(this->ctx, 64);
    array_sort = Z3_mk_array_sort(this->ctx, bv_sort, bv_sort);

    bv_sort_32 = Z3_mk_bv_sort(this->ctx, 32);
    array_sort_32 = Z3_mk_array_sort(this->ctx, bv_sort_32, bv_sort_32);

    bb = LLVMGetFirstBasicBlock(this->Fn);
    insn = LLVMGetFirstInstruction(bb);

    /* Small hack as ram is not explicit in LLVM
        ram = mk_var(this->ctx, "ram", array_sort);
     */
    if ((ram = malloc(sizeof(*ram))) == NULL)
        LOG_LLVM("Error while allocating ram in llvm_to_z3\n");

    ram->valueref = NULL;
    ram->index = 0;
    ram->name = chunk_calloc(255);
    strncpy((char*)ram->name.ptr, "ram", ram->name.len);
    ram->prefix = chunk_clone(this->prefix);

    if (ram->prefix.ptr)
    {
        chunk_t ram_name;
        ram_name = chunk_cat("cc", ram->prefix, ram->name);
        ram->symbol = mk_var(this->ctx, (char*)ram_name.ptr, array_sort);
        chunk_free(&ram_name);
    }
    else
        ram->symbol = mk_var(this->ctx, (char*)ram->name.ptr, array_sort);

    ram->is_global = 1;

    if ((env = malloc(sizeof(*env))) == NULL)
        LOG_LLVM("Error while allocating env in llvm_to_z3\n");

    env->valueref = NULL;
    env->index = 0;
    env->name = chunk_calloc(255);
    strncpy((char*)env->name.ptr, "env", env->name.len);
    env->prefix = chunk_clone(this->prefix);

    if (env->prefix.ptr)
    {
        chunk_t env_name;
        env_name = chunk_cat("cc", env->prefix, env->name);
        env->symbol = mk_var(this->ctx, (char*)env_name.ptr, array_sort_32);
        chunk_free(&env_name);
    }
    else
        env->symbol = mk_var(this->ctx, (char*)env->name.ptr, array_sort_32);

    env->is_global = 1;

    do
    {
        /*
         * Handling Opcode
         */
        if (LLVMHasMetadata(insn))
            LOG_LLVM("Has metadata!\n");

        opcode = LLVMGetInstructionOpcode(insn);
        type = LLVMGetTypeKind(LLVMTypeOf(insn));
        Z3_res = NULL;

        if (type)
        {
            LOG_LLVM("Store [%x:%x] in %s\n", type, LLVMTypeOf(insn), LLVMGetValueName(insn));
            store_name = create_Z3_var(this, insn);
        }
        else
        {
            store_name = NULL;
        }

        LOG_LLVM("Opcode: %i ", opcode);

        switch (opcode)
        {
            case LLVMRet:
            {
                LOG_LLVM("LLVMRet\n");
                break;
            }
            case LLVMBr:
            {
                LOG_LLVM("LLVMBr\n");
                break;
            }
            case LLVMSwitch:
            {
                LOG_LLVM("LLVMSwitch\n");
                break;
            }
            case LLVMIndirectBr:
            {
                LOG_LLVM("LLVMIndirectBr\n");
                break;
            }
            case LLVMInvoke:
            {
                LOG_LLVM("LLVMInvoke\n");
                break;
            }
            case LLVMUnreachable:
            {
                LOG_LLVM("LLVMUnreachable\n");
                break;
            }
            case LLVMAdd:
            {
                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                LOG_LLVM("LLVMAdd\n");

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvadd(this->ctx, Z3_dst, Z3_src);

                break;
            }
            case LLVMFAdd:
            {
                LOG_LLVM("LLVMFAdd\n");
                break;
            }
            case LLVMSub:
            {
                LOG_LLVM("LLVMSub\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvsub(this->ctx, Z3_src, Z3_dst);
                break;
            }
            case LLVMFSub:
            {
                LOG_LLVM("LLVMFSub\n");
                break;
            }
            case LLVMMul:
            {
                LOG_LLVM("LLVMMul\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvmul(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMFMul:
            {
                LOG_LLVM("LLVMFMul\n");
                break;
            }
            case LLVMUDiv:
            {
                LOG_LLVM("LLVMUDiv\n");
                break;
            }
            case LLVMSDiv:
            {
                LOG_LLVM("LLVMSDiv\n");
                break;
            }
            case LLVMFDiv:
            {
                LOG_LLVM("LLVMFDiv\n");
                break;
            }
            case LLVMURem:
            {
                LOG_LLVM("LLVMURem\n");
                break;
            }
            case LLVMSRem:
            {
                LOG_LLVM("LLVMSRem\n");
                break;
            }
            case LLVMFRem:
            {
                LOG_LLVM("LLVMFRem\n");
                break;
            }
            case LLVMShl:
            {
                LOG_LLVM("LLVMShl\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvshl(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMLShr:
            {
                LOG_LLVM("LLVMLShr\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvashr(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMAShr:
            {
                LOG_LLVM("LLVMAShr\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvashr(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMAnd:
            {
                LOG_LLVM("LLVMAnd\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvand(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMOr:
            {
                LOG_LLVM("LLVMOr\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvor(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMXor:
            {
                LOG_LLVM("LLVMXor\n");

                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);
                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_bvxor(this->ctx, Z3_dst, Z3_src);
                break;
            }
            case LLVMAlloca:
            {
                LOG_LLVM("LLVMAlloca\n");
                break;
            }
            case LLVMLoad:
            {
                LLVMValueRef src;
                Z3_ast Z3_src;

                /*operation = Z3_mk_store;*/
                LOG_LLVM("LLVMLoad\n");

                src = LLVMGetOperand(insn, 0);

                Z3_src = create_Z3_var_read(this, src);

                type = LLVMGetTypeKind(LLVMTypeOf(src));

                switch (type)
                {
                    /* 
                     * This is either an ConstInt or a Named integer (%bla)
                     */
                    case LLVMIntegerTypeKind:
                    {
                        Z3_res = Z3_src;
                        break;
                    }
                    /*
                     * This is a PointerInt (*bla)
                     */
                    case LLVMPointerTypeKind:
                    {
                        if (LLVMIsAGlobalValue(src))
                            Z3_res = Z3_src;
                        else
                        {
                            if (get_z3_size(this, Z3_src) == 32)
                                Z3_res = Z3_mk_select(this->ctx, env->symbol, Z3_src);
                            else
                                Z3_res = Z3_mk_select(this->ctx, ram->symbol, Z3_src);
                        }
                        break;
                    }
                    default:
                    {
                        LOG_LLVM("       Found something BID: %x (will segfault, Z3_res random)\n", type);
                    }
                }
                break;
            }
            case LLVMStore:
            {
                LLVMValueRef src, dst;
                Z3_ast Z3_src, Z3_dst;

                /*operation = Z3_mk_store;*/
                LOG_LLVM("LLVMStore\n");

                src = LLVMGetOperand(insn, 0);
                dst = LLVMGetOperand(insn, 1);

                Z3_src = create_Z3_var(this, src);

                type = LLVMGetTypeKind(LLVMTypeOf(dst));

                switch (type)
                {
                    /* 
                     * This is either an ConstInt or a Named integer (%bla)
                     */
                    case LLVMIntegerTypeKind:
                    {
                        Z3_dst = create_Z3_var_inc(this, dst);
                        Z3_res = Z3_mk_eq(this->ctx, Z3_dst, Z3_src);
                        break;
                    }
                    /*
                     * This is a PointerInt (*bla)
                     */
                    case LLVMPointerTypeKind:
                    {
                        if (LLVMIsAGlobalValue(dst))
                        {
                            //Z3_res = Z3_mk_eq(this->ctx, Z3_dst, Z3_src);
                            Z3_dst = create_Z3_var_inc(this, dst);
                            store_name = Z3_dst;
                            Z3_res = Z3_src;
                        }
                        else
                        {
                            /* FIXME
                             * Should be something (= (ram) (store ram val)) as
                             * pointed by the Model if SAT.
                            store_name = mk_var(this->ctx, "JUNK", array_sort);
                             *
                             * FIXME
                             * Something with only get_z3_size
                             */
                            chunk_t new_chunk;
                            Z3_ast previous;
                            Z3_symbol_cell *new;

                            Z3_dst = create_Z3_var(this, dst);

                            new_chunk = chunk_calloc(255);

                            if (get_z3_size(this, Z3_src) == 32)
                            {
                                new = env;
                            }
                            else
                            {
                                new = ram;
                            }

                            new->index++;
                            previous = new->symbol;

                            snprintf((char*) new_chunk.ptr, new_chunk.len, "%s%i", new->name.ptr, new->index);

                            if (new->prefix.ptr)
                                new_chunk = chunk_cat("cm", new->prefix, new_chunk);

                            if (get_z3_size(this, Z3_src) == 32)
                            {
                                new->symbol = mk_var(this->ctx, (char*)new_chunk.ptr, array_sort_32);
                            }
                            else
                            {
                                new->symbol = mk_var(this->ctx, (char*)new_chunk.ptr, array_sort);
                            }

                            store_name = new->symbol;
                            Z3_res = Z3_mk_store(this->ctx, previous, Z3_dst, Z3_src);

                            chunk_clear(&new_chunk);
                        }
                        break;
                    }
                    default:
                    {
                        LOG_LLVM("       Found something BuD: %x (will segfault, Z3_res random)\n", type);
                    }
                }
                break;
            }
            case LLVMGetElementPtr:
            {
                LOG_LLVM("LLVMGetElementPtr\n");
                break;
            }
            case LLVMTrunc:
            {
                LLVMValueRef dst;
                Z3_ast Z3_dst;

                LOG_LLVM("LLVMTrunc\n");

                dst = LLVMGetOperand(insn, 0);

                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_extract(this->ctx, get_z3_size(this, store_name)-1, 0, Z3_dst);
                break;
            }
            case LLVMZExt:
            {
                LLVMValueRef dst;
                Z3_ast Z3_dst;

                LOG_LLVM("LLVMZExt\n");

                dst = LLVMGetOperand(insn, 0);

                Z3_dst = create_Z3_var(this, dst);

                Z3_res = Z3_mk_zero_ext(this->ctx, get_z3_size(this, store_name) - get_z3_size(this, Z3_dst), Z3_dst);
                break;
            }
            case LLVMSExt:
            {
                LOG_LLVM("LLVMSExt\n");
                break;
            }
            case LLVMFPToUI:
            {
                LOG_LLVM("LLVMFPToUI\n");
                break;
            }
            case LLVMFPToSI:
            {
                LOG_LLVM("LLVMFPToSI\n");
                break;
            }
            case LLVMUIToFP:
            {
                LOG_LLVM("LLVMUIToFP\n");
                break;
            }
            case LLVMSIToFP:
            {
                LOG_LLVM("LLVMSIToFP\n");
                break;
            }
            case LLVMFPTrunc:
            {
                LOG_LLVM("LLVMFPTrunc\n");
                break;
            }
            case LLVMFPExt:
            {
                LOG_LLVM("LLVMFPExt\n");
                break;
            }
            case LLVMPtrToInt:
            {
                LOG_LLVM("LLVMPtrToInt\n");
                break;
            }
            case LLVMIntToPtr:
            {
                LLVMValueRef src;

                LOG_LLVM("LLVMIntToPtr\n");

                src = LLVMGetOperand(insn, 0);
                Z3_res = create_Z3_var(this, src);
                break;
            }
            case LLVMBitCast:
            {
                LOG_LLVM("LLVMBitCast\n");
                break;
            }
            case LLVMAddrSpaceCast:
            {
                LOG_LLVM("LLVMAddrSpaceCast\n");
                break;
            }
            case LLVMICmp:
            {
                LOG_LLVM("LLVMICmp\n");
                break;
            }
            case LLVMFCmp:
            {
                LOG_LLVM("LLVMFCmp\n");
                break;
            }
            case LLVMPHI:
            {
                LOG_LLVM("LLVMPHI\n");
                break;
            }
            case LLVMCall:
            {
                LOG_LLVM("LLVMCall\n");
                break;
            }
            case LLVMSelect:
            {
                LOG_LLVM("LLVMSelect\n");
                break;
            }
            case LLVMUserOp1:
            {
                LOG_LLVM("LLVMUserOp1\n");
                break;
            }
            case LLVMUserOp2:
            {
                LOG_LLVM("LLVMUserOp2\n");
                break;
            }
            case LLVMVAArg:
            {
                LOG_LLVM("LLVMVAArg\n");
                break;
            }
            case LLVMExtractElement:
            {
                LOG_LLVM("LLVMExtractElement\n");
                break;
            }
            case LLVMInsertElement:
            {
                LOG_LLVM("LLVMInsertElement\n");
                break;
            }
            case LLVMShuffleVector:
            {
                LOG_LLVM("LLVMShuffleVector\n");
                break;
            }
            case LLVMExtractValue:
            {
                LOG_LLVM("LLVMExtractValue\n");
                break;
            }
            case LLVMInsertValue:
            {
                LOG_LLVM("LLVMInsertValue\n");
                break;
            }
            case LLVMFence:
            {
                LOG_LLVM("LLVMFence\n");
                break;
            }
            case LLVMAtomicCmpXchg:
            {
                LOG_LLVM("LLVMAtomicCmpXchg\n");
                break;
            }
            case LLVMAtomicRMW:
            {
                LOG_LLVM("LLVMAtomicRMW\n");
                break;
            }
            case LLVMResume:
            {
                LOG_LLVM("LLVMResume\n");
                break;
            }
            case LLVMLandingPad:
            {
                LOG_LLVM("LLVMLandingPad\n");
                break;
            }
        }

        number_of_operands = LLVMGetNumOperands(insn);

        /*
         * Handling Operands
         */
        for (operand_num = 0; operand_num < number_of_operands; operand_num++)
        {
            operand = LLVMGetOperand(insn, operand_num);
            type = LLVMGetTypeKind(LLVMTypeOf(operand));
            LOG_LLVM("   Got operand[%i/%i]: %x\n", operand_num+1, number_of_operands, type);

            switch (type)
            {
                case LLVMIntegerTypeKind:
                {
                    if (LLVMIsAConstantInt(operand))
                        LOG_LLVM("       Found inti: %x\n", LLVMConstIntGetZExtValue(operand));
                    else
                        LOG_LLVM("       Found ints: %s\n", LLVMGetValueName(operand));

                    break;
                }
                case LLVMPointerTypeKind:
                {
                    if (LLVMIsAGlobalValue(operand))
                        LOG_LLVM("       Found pointer: @%s\n", LLVMGetValueName(operand));
                    else
                        LOG_LLVM("       Found pointer: %s\n", LLVMGetValueName(operand));
                    break;
                }
                default:
                {
                    LOG_LLVM("       Found something :0\n");
                }
            }
        }

        /*
         * Assign the result to the symbol of needed
         */
        if (Z3_res)
        {
            Z3_ast tmp;
            uint64_t store_name_size, Z3_res_size;

            store_name_size = get_z3_size(this, store_name);
            Z3_res_size = get_z3_size(this, Z3_res);

            if (store_name_size > Z3_res_size)
            {
                Z3_ast Z3_res_ext;

                Z3_res_ext = Z3_mk_zero_ext(this->ctx, store_name_size - Z3_res_size, Z3_res);

                Z3_res = Z3_res_ext;
            }
            else if (store_name_size < Z3_res_size)
            {
                Z3_ast Z3_res_trunc;

                Z3_res_trunc = Z3_mk_extract(this->ctx, store_name_size - 1, 0, Z3_res);

                Z3_res = Z3_res_trunc;
            }

            tmp = Z3_mk_eq(this->ctx, store_name, Z3_res);
            Z3_res = tmp;

            /* Uncomment to dump each instruction 
             */
            LOG_LLVM("AST: %s\n", Z3_ast_to_string(this->ctx, Z3_res));

            /*
             * Adding to Z3 context
            operation(ctx, ram, store_val, store_to);
             */
            args[0] = this->formula;
            args[1] = Z3_res;
            this->formula = Z3_mk_and(this->ctx, 2, args);
        }
    } while ((insn = (LLVMGetNextInstruction(insn))));

    LOG_LLVM("AST: %s\n", Z3_ast_to_string(this->ctx, this->formula));

    chunk_free(&ram->name);
    chunk_free(&ram->prefix);

    free(ram);

    return map_create(this->ctx, this->formula, this->Z3_symbol_list);
}

/*
 * Set the prefix for z3 variables. It is easier to create equality between
 * z3 ast.
 *
 * @param prefix    The chunk containing the prefix to use. Can be NULL for
 *                  no prefix.
 * @return          void
 */
static void set_prefix(private_converter_t *this, chunk_t prefix)
{
    if (prefix.ptr)
    {
        if (this->prefix.ptr)
            chunk_clear(&this->prefix);

        this->prefix = chunk_calloc(prefix.len - 1);
        strncpy((char*)this->prefix.ptr, (char*)prefix.ptr, this->prefix.len);
    }
}

static void destroy_target_cell(void *target_cell)
{
    Z3_symbol_cell *c;

    c = (Z3_symbol_cell*) target_cell;

    if ((c) && (c->name.ptr))
        chunk_clear(&c->name);

    if ((c) && (c->prefix.ptr))
        chunk_clear(&c->prefix);

    free(target_cell);
    target_cell = NULL;
}

static void destroy(private_converter_t *this)
{
    LLVMDisposePassManager(this->pass_mgr);
    LLVMDisposeBuilder(this->builder);
    LLVMDisposeModule(this->module);

    /*
     * The map returned by llvm_to_z3 is responsible of context and symbol list
     * freeing.
     *
    Z3_del_context(this->ctx);
     */
    this->Z3_symbol_list->destroy_function(this->Z3_symbol_list, destroy_target_cell);
    chunk_clear(&this->prefix);

    free(this);
    this = NULL;
}

converter_t *converter_create(TCGContext *s, Z3_context ctx)
{
    private_converter_t *this = malloc_thing(private_converter_t);

    this->module = LLVMModuleCreateWithName("X");
    this->builder = LLVMCreateBuilder ();
    this->s = s;
    this->rip = NULL;
    this->df = NULL;

    memset(this->m_globalsIdx, 0, sizeof(this->m_globalsIdx));
    memset(this->m_values, 0, sizeof(this->m_values));
    memset(this->m_memValuesPtr, 0, sizeof(this->m_memValuesPtr));
    memset(this->m_labels, 0, sizeof(this->m_labels));

    int reg_to_idx[TCG_TARGET_NB_REGS];
    int i;

    for(i=0; i<TCG_TARGET_NB_REGS; ++i)
        reg_to_idx[i] = -1;

    int argNumber = 0;
    for(i=0; i<s->nb_globals; ++i) {
        if(s->temps[i].fixed_reg) {
            // This global is in fixed host register. We are
            // mapping such registers to function arguments
            this->m_globalsIdx[i] = argNumber++;
            reg_to_idx[s->temps[i].reg] = i;

        } else {
            // This global is in memory at (mem_reg + mem_offset).
            // Base value is not known yet, so just store mem_reg
            this->m_globalsIdx[i] = s->temps[i].mem_reg;
        }
    }

    // Map mem_reg to index for memory-based globals
    for(i=0; i<s->nb_globals; ++i) {
        if(!s->temps[i].fixed_reg) {
            assert(reg_to_idx[m_globalsIdx[i]] >= 0);
            this->m_globalsIdx[i] = reg_to_idx[this->m_globalsIdx[i]];
        }
    }

    LLVMInitializeNativeTarget ();
    LLVMLinkInJIT();

    this->ctx = ctx;
    this->formula = Z3_mk_true(this->ctx);
    this->prefix = chunk_empty;

    Z3_set_error_handler(this->ctx, error_handler);

    this->Z3_symbol_list = linked_list_create();

    this->optimize_llvm = (void (*)(private_converter_t *)) optimize_llvm;

    this->public.tcg_to_llvm = (void (*)(converter_t *)) tcg_to_llvm;
    this->public.llvm_to_z3 = (map_t *(*)(converter_t *)) llvm_to_z3;
    this->public.dump = (void (*)(converter_t *)) dump;
    this->public.set_prefix = (void (*)(converter_t *, chunk_t)) set_prefix;
    this->public.destroy = (void (*)(converter_t*)) destroy;

    return &this->public;
}
