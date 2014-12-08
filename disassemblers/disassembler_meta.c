/*
 * Implementation of disassembler
 */

#include "disassembler_meta.h"
#include "disassembler_xed.h"
#include "disassembler_capstone.h"

#define BUFLEN  1000

#define DEBUG_META
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

    cat = ERR;
    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (cat == ERR))
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

    result = -1;
    e = this->disassemblers->create_enumerator(this->disassemblers);

    while ((e->enumerate(e, &d)) && (result == (uint64_t)-1))
        result = d->get_length(d, i);
    
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
    int num;
    chunk_t str;

    if (!this)
        return 0;

    str = chunk_calloc(1024);

    num = 0;
    status = FAILED;

    e = this->disassemblers->create_enumerator(this->disassemblers);

    while (e->enumerate(e, &d))
    {
        if (num == 0)
        {
            status = d->decode(d, i, c);
            if (status == SUCCESS)
            {
                d->dump_intel(d, *i, &str, 0);
            }
        }
        else
        {
            status_t s;
            instruction_t *i2;

            s = d->decode(d, &i2, c);
            if (s != status)
            {
                LOG_META("********************************\n");
                LOG_META("Different decoding d%x:%x and d%x:%x\n", 0, status, num, s);

                if (s == SUCCESS)
                {
                    chunk_t str2;

                    str2 = chunk_calloc(1024);
                    d->dump_intel(d, i2, &str2, 0);

                    LOG_META(":: %s :: ", str2.ptr);

                    chunk_clear(&str2);
                }
                else
                {
                    LOG_META(":: %s :: ", str.ptr);
                }

#ifdef DEBUG_META
                hexdump(c.ptr, c.len);
#endif
                LOG_META("\n");
            }

            i2->destroy(i2);
        }

        num++;
    }

    chunk_clear(&str);
    
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
    this->public.interface.destroy = (void (*)(disassembler_t*)) destroy;

    return &this->public;
}
