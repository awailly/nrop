/*
 * Implementation of disassembler
 */

#include "disassembler_capstone.h"

#define BUFLEN  1000

#define DEBUG_CAPSTONE
#ifdef DEBUG_CAPSTONE
#  define LOG_CAPSTONE(...) logging(__VA_ARGS__)
#else
#  define LOG_CAPSTONE(...) do { } while (0)
#endif

typedef struct private_disass_capstone_t private_disass_capstone_t;

struct private_disass_capstone_t
{
    disass_capstone_t public;

    uint64_t arch;
    uint64_t mmode;
    csh handle;
};

typedef struct capstone_instruction_t capstone_instruction_t;

struct capstone_instruction_t
{
    instruction_t interface;

    cs_insn *insn;
};

static status_t initialize(private_disass_capstone_t *this, chunk_t c)
{
    bool long_mode;

    long_mode = (strncmp((char*) c.ptr, "ELF64", 5) == 0);

    if (long_mode) {
        this->arch = CS_ARCH_X86;
        this->mmode = CS_MODE_64;
    }
    else {
        this->arch = CS_ARCH_X86;
        this->mmode = CS_MODE_32;
    }

    if (cs_open(this->arch, this->mmode, &this->handle) != CS_ERR_OK)
    {
        LOG_CAPSTONE("Error while initializing capstone in disassembler_capstone.\n");
        return FAILED;
    }

    cs_option(this->handle, CS_OPT_DETAIL, CS_OPT_ON);

    return SUCCESS;
}

static category_t get_category(private_disass_capstone_t *this, instruction_t *i)
{
    cs_insn *insn;

    if (!this)
        return NO_CAT;

    insn = ((capstone_instruction_t*) i)->insn;

    if (cs_insn_group(this->handle, insn, X86_GRP_JUMP))
        return COND_BR;
    else if (cs_insn_group(this->handle, insn, X86_GRP_JUMP))
        return UNCOND_BR;
    else if (cs_insn_group(this->handle, insn, X86_GRP_INT))
        return SYSCALL;
    else if (cs_insn_group(this->handle, insn, X86_GRP_CALL))
        return CALL;
    else if (cs_insn_group(this->handle, insn, X86_GRP_RET))
        return RET;
    else
        return NO_CAT;
}

static uint64_t get_length(private_disass_capstone_t *this, instruction_t *i)
{
    cs_insn *insn;

    if (!this)
        return 0;

    insn = ((capstone_instruction_t*) i)->insn;

    LOG_CAPSTONE("[x] get_length() = %zx\n", insn->size);

    return insn->size;
}

static status_t format(private_disass_capstone_t *this, instruction_t *i, chunk_t format)
{
    cs_insn *x;

    if (!this)
        return FAILED;

    x = ((capstone_instruction_t*) i)->insn;

    format.len = strlen(x->op_str) + 1;
    memcpy(format.ptr, x->op_str, format.len);

    return SUCCESS;
}

static status_t dump_intel(private_disass_capstone_t *this, instruction_t *i, chunk_t *buffer, uint64_t offset_addr)
{
    cs_insn *x;
    //chunk_t address;
    chunk_t mnemonic;
    chunk_t str;
    chunk_t res;

    if (!this)
        return FAILED;

    x = ((capstone_instruction_t*) i)->insn;

    //address = chunk_calloc(21);
    //snprintf((char*)address.ptr, address.len, "%16lx: ", offset_addr);
    mnemonic.len = strlen(x->mnemonic) + 1;
    mnemonic.ptr = (u_char*) x->mnemonic;

    str.len = strlen(x->op_str) + 1;
    str.ptr = (u_char*) x->op_str;

    res = chunk_cat("cc", mnemonic, str);

    buffer->ptr = res.ptr;
    buffer->len = res.len;

    return SUCCESS;
}

static status_t decode(private_disass_capstone_t *this, instruction_t **i, chunk_t c)
{
    cs_insn *insn;
    size_t capstone_error_code;
    cs_err capstone_error;
    status_t status;

    if ((*i = (instruction_t*) malloc_thing(capstone_instruction_t)) == NULL)
    {
        logging("[x] Error while allocating capstone_instruction_t in disassembler_capstone.c\n");
        return FAILED;
    }

    capstone_error_code = cs_disasm(this->handle, c.ptr, c.len, 0x1000, 1, &insn);

    LOG_CAPSTONE("instruction @%x capstoned @%x chunk.ptr:%x chunk.len:%x\n", *i, insn, c.ptr, c.len);

    if (capstone_error_code == 1)
    {
        LOG_CAPSTONE("[x] Successfully decoded: %s\n", insn->op_str);

        ((capstone_instruction_t*) *i)->insn = insn;
        (*i)->bytes = chunk_calloc(insn->size + 1);
        memcpy((*i)->bytes.ptr, c.ptr, insn->size);

        (*i)->str= chunk_empty;

        status = SUCCESS;
    }
    else{
        capstone_error = cs_errno(this->handle);

        LOG_CAPSTONE("[x] Error while decoding chunk: [%x]:%s\n", capstone_error_code, cs_strerror(capstone_error));

        ((capstone_instruction_t*) *i)->insn = NULL;
        (*i)->bytes = chunk_empty;
        (*i)->str= chunk_empty;

        status = FAILED;
    }

    return status;
}

static status_t encode(private_disass_capstone_t *this, chunk_t *c, instruction_t *i)
{
    LOG_CAPSTONE("Capstone does not encode ... %x %x %x\n", this, c, i);

    return SUCCESS;
}

static instruction_t *alloc_instruction(private_disass_capstone_t *this)
{
    capstone_instruction_t *new_insn;

    if (!this)
        return NULL;

    if ((new_insn = malloc(sizeof(*new_insn))) == NULL)
    {
        logging("Error while allocating instruction in disassembler_capstone\n");
        return NULL;
    }

    new_insn->interface.bytes = chunk_empty;
    new_insn->interface.str = chunk_empty;

    return (instruction_t*) new_insn;
}

static void *clone_instruction(void *instruction)
{
    capstone_instruction_t *new_instruction;
    capstone_instruction_t *insn;

    if ((new_instruction = malloc(sizeof(*new_instruction))) == NULL)
    {
        LOG_CAPSTONE("Error while allocating instruction in clone_instruction from chain.c\n");
        return NULL;
    }

    memcpy(new_instruction, instruction, sizeof(*new_instruction));

    insn = ((capstone_instruction_t*) instruction);

    new_instruction->interface.bytes = chunk_clone(insn->interface.bytes);
    new_instruction->interface.str = chunk_clone(insn->interface.str);

    if ((new_instruction->insn = malloc(sizeof(cs_insn))) == NULL)
    {
        LOG_CAPSTONE("Error while allocating cs_insn in clone_instruction from chain.c\n");
        free(new_instruction);
        return NULL;
    }

    memcpy(new_instruction->insn, insn->insn, sizeof(cs_insn));

    return new_instruction;
}

static void destroy_instruction(void *instruction)
{
    cs_insn *capstone_insn;
    capstone_instruction_t *insn;

    insn = ((capstone_instruction_t*) instruction);
    //capstone_insn = insn->insn;

    //if (capstone_insn)
    //    cs_free(capstone_insn, 1);

    chunk_clear(&insn->interface.bytes);
    chunk_clear(&insn->interface.str);

    free(instruction);
    instruction = NULL;
}

static uint64_t get_instruction_size(private_disass_capstone_t *this)
{
    if (!this)
        return FAILED;

    return sizeof(capstone_instruction_t);
}

static void destroy(private_disass_capstone_t *this)
{
    cs_close(&this->handle);

    free(this);
    this = NULL;
}

disass_capstone_t *create_capstone()
{
    private_disass_capstone_t *this = malloc_thing(private_disass_capstone_t);

    /* Interface implementation */
    this->public.interface.initialize = (status_t (*)(disassembler_t*, chunk_t)) initialize;
    this->public.interface.get_category = (category_t (*)(disassembler_t*, instruction_t*)) get_category;
    this->public.interface.get_length = (uint64_t (*)(disassembler_t*, instruction_t*)) get_length;
    this->public.interface.format = (status_t (*)(disassembler_t*, instruction_t *, chunk_t)) format;
    this->public.interface.dump_intel = (status_t (*)(disassembler_t*, instruction_t *, chunk_t *, uint64_t)) dump_intel;
    this->public.interface.decode = (status_t (*)(disassembler_t*, instruction_t **, chunk_t)) decode;
    this->public.interface.encode = (status_t (*)(disassembler_t*, chunk_t *, instruction_t *)) encode;
    this->public.interface.alloc_instruction = (instruction_t *(*)(disassembler_t*)) alloc_instruction;
    this->public.interface.clone_instruction = (void *(*)(void *)) clone_instruction;
    this->public.interface.destroy_instruction = (void (*)(void *)) destroy_instruction;
    this->public.interface.get_instruction_size = (uint64_t (*)(disassembler_t*)) get_instruction_size;
    this->public.interface.destroy = (void (*)(disassembler_t*)) destroy;

    return &this->public;
}
