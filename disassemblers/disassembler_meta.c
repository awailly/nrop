/*
 * Implementation of disassembler
 */

#include "disassembler_meta.h"
#include "disassembler_xed.h"
#include "disassembler_capstone.h"

#define BUFLEN  1000

//#define DEBUG_META
#ifdef DEBUG_META
#  define LOG_META(...) logging(__VA_ARGS__)
#else
#  define LOG_META(...) do { } while (0)
#endif

typedef struct private_disass_meta_t private_disass_meta_t;

struct private_disass_meta_t
{
    disass_meta_t public;

    linked_list_t *disassemblers;
};

typedef struct meta_instruction_t meta_instruction_t;

struct meta_instruction_t
{
    instruction_t interface;
};

static status_t initialize(private_disass_meta_t *this, chunk_t c)
{
    enumerator_t *e;
    disassembler_t *d;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while (e->enumerate(e, &d))
        d->initialize(d, c);
    
    e->destroy(e);

    return SUCCESS;
}

static category_t get_category(private_disass_meta_t *this, instruction_t *i)
{
    category_t cat;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return NO_CAT;

    cat = NO_CAT;
    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (cat == NO_CAT))
        cat = d->get_category(d, i);
    
    e->destroy(e);

    return cat;
}

static uint64_t get_length(private_disass_meta_t *this, instruction_t *i)
{
    uint64_t result;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return 0;

    result = 0;
    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (result == 0))
        result = d->get_category(d, i);
    
    e->destroy(e);

    return result;
}

static status_t format(private_disass_meta_t *this, instruction_t *i, chunk_t *format)
{
    status_t status;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return 0;

    status = FAILED;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (status == FAILED))
        status = d->format(d, i, format);
    
    e->destroy(e);
    
    return status;
}

static status_t dump_intel(private_disass_meta_t *this, instruction_t *i, chunk_t *buffer, uint64_t offset_addr)
{
    status_t status;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return 0;

    status = FAILED;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (status == FAILED))
        status = d->dump_intel(d, i, buffer, offset_addr);
    
    e->destroy(e);
    
    return status;
}

static status_t decode(private_disass_meta_t *this, instruction_t **i, chunk_t c)
{
    status_t status;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return 0;

    status = FAILED;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (status == FAILED))
        status = d->decode(d, i, c);
    
    e->destroy(e);
    
    return status;
}

static status_t encode(private_disass_meta_t *this, chunk_t *c, instruction_t *i)
{
    status_t status;

    enumerator_t *e;
    disassembler_t *d;

    if (!this)
        return 0;

    status = FAILED;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (status == FAILED))
        status = d->encode(d, c, i);
    
    e->destroy(e);
    
    return status;
}

static void *clone_instruction(void *instruction)
{
    meta_instruction_t *new_instruction;
    meta_instruction_t *insn;

    if ((new_instruction = malloc(sizeof(*new_instruction))) == NULL)
    {
        LOG_META("Error while allocating instruction in clone_instruction from chain.c\n");
        return NULL;
    }

    memcpy(new_instruction, instruction, sizeof(*new_instruction));

    insn = ((meta_instruction_t*) instruction);

    new_instruction->interface.bytes = chunk_clone(insn->interface.bytes);
    new_instruction->interface.str = chunk_clone(insn->interface.str);

    return new_instruction;
}

static void destroy_instruction(void *instruction)
{
    meta_instruction_t *insn;

    insn = ((meta_instruction_t*) instruction);

    chunk_clear(&insn->interface.bytes);
    chunk_clear(&insn->interface.str);

    free(instruction);
    instruction = NULL;
}

static instruction_t *alloc_instruction(private_disass_meta_t *this)
{
    meta_instruction_t *new_insn;

    if (!this)
        return NULL;

    if ((new_insn = malloc(sizeof(*new_insn))) == NULL)
    {
        logging("Error while allocating instruction in disassembler_meta\n");
        return NULL;
    }

    new_insn->interface.bytes = chunk_empty;
    new_insn->interface.str = chunk_empty;
    new_insn->interface.clone = (instruction_t *(*)(instruction_t *)) clone_instruction;
    new_insn->interface.destroy = (void (*)(instruction_t *)) destroy_instruction;

    return (instruction_t*) new_insn;
}

static void destroy(private_disass_meta_t *this)
{
    enumerator_t *e;
    disassembler_t *d;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while (e->enumerate(e, &d))
        d->destroy(d);
    
    e->destroy(e);

    this->disassemblers->destroy(this->disassemblers);

    free(this);
    this = NULL;
}

disass_meta_t *create_meta()
{
    private_disass_meta_t *this = malloc_thing(private_disass_meta_t);

    /* Adding disassemblers */
    this->disassemblers = linked_list_create();
    this->disassemblers->insert_last(this->disassemblers, create_xed());
    this->disassemblers->insert_last(this->disassemblers, create_capstone());

    /* Interface implementation */
    this->public.interface.initialize = (status_t (*)(disassembler_t*, chunk_t)) initialize;
    this->public.interface.get_category = (category_t (*)(disassembler_t*, instruction_t*)) get_category;
    this->public.interface.get_length = (uint64_t (*)(disassembler_t*, instruction_t*)) get_length;
    this->public.interface.format = (status_t (*)(disassembler_t*, instruction_t *, chunk_t *)) format;
    this->public.interface.dump_intel = (status_t (*)(disassembler_t*, instruction_t *, chunk_t *, uint64_t)) dump_intel;
    this->public.interface.decode = (status_t (*)(disassembler_t*, instruction_t **, chunk_t)) decode;
    this->public.interface.encode = (status_t (*)(disassembler_t*, chunk_t *, instruction_t *)) encode;
    this->public.interface.alloc_instruction = (instruction_t *(*)(disassembler_t*)) alloc_instruction;
    this->public.interface.destroy = (void (*)(disassembler_t*)) destroy;

    return &this->public;
}
