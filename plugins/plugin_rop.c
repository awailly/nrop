/*
 * Implementation of rop insertion plugin
 */

#include "plugin_rop.h"

#define DEBUG_ROP
#ifdef DEBUG_ROP
#  define LOG_ROP(...) logging(__VA_ARGS__)
#else
#  define LOG_ROP(...) do { } while (0)
#endif

//#define DEBUG_ROP_DEBUG
#ifdef DEBUG_ROP_DEBUG
#  define LOG_ROP_DEBUG(...) logging(__VA_ARGS__)
#else
#  define LOG_ROP_DEBUG(...) do { } while (0)
#endif

#define BUFLEN  1000

typedef struct private_plugin_rop_t private_plugin_rop_t;

struct private_plugin_rop_t
{
    plugin_rop_t public;
    elf_t *code;
    char *constraints;
    chain_t *target;
    thpool_t* threadpool;
    disassembler_t *d;
    chunk_t code_type;

    status_t (*disassemble)(private_plugin_rop_t *, chunk_t);
    bool (*is_last_inst)(private_plugin_rop_t *, instruction_t*);
    bool (*bad_insn)(private_plugin_rop_t *, unsigned char *);
    status_t (*pack)(private_plugin_rop_t *, Elf64_Addr, chunk_t);
    linked_list_t *(*find_rop_chains)(private_plugin_rop_t *, chunk_t, Elf64_Addr);
    status_t (*reverse_disass_ret)(private_plugin_rop_t *, chunk_t, Elf64_Addr, uint64_t, linked_list_t*);
};

typedef struct job_reverse_disass_ret_th_arg job_reverse_disass_ret_th_arg;

struct job_reverse_disass_ret_th_arg
{
    private_plugin_rop_t *this;
    chunk_t function_chunk;
    Elf64_Addr addr;
    uint64_t byte;
    linked_list_t *inst_list;
};

static status_t disassemble(private_plugin_rop_t *this, chunk_t function_chunk)
{
    chunk_t code_chunk;
    instruction_t *instruction;

    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    unsigned int i;

    disassembler_t *d;

    if (!this)
        return FAILED;

    d = this->d;
    instruction = NULL;

    hexdump(function_chunk.ptr, function_chunk.len);

    for (i=0; i<function_chunk.len; )
    {
        bytes = 15;
        memcpy(itext, function_chunk.ptr + i, bytes);

        code_chunk = chunk_create(itext, 15);

        /*hexdump(&itext, 15);*/

        d->decode(d, &instruction, code_chunk);

        i+= d->get_length(d, instruction);

        instruction->destroy(instruction);
    }

    return SUCCESS;
}

static bool is_last_inst(private_plugin_rop_t *this, instruction_t *instruction)
{
    category_t cat;

    cat = this->d->get_category(this->d, instruction);

    return (
            (cat == COND_BR) ||
            (cat == UNCOND_BR) ||
            (cat == SYSCALL) ||
            (cat == CALL) ||
            (cat == RET)
           );
}

static bool bad_insn(private_plugin_rop_t *this, unsigned char *itext)
{
    bool is_bad;

    is_bad = 0;

    /*
     * Pacify compiler
     */
    if (this->code)
    {
        if (itext[0] == 0xf4)
            is_bad = 1;
        else if (itext[0] == 0xfb)
            is_bad = 1;
        else if (itext[0] == 0xcc)
            is_bad = 1;
    }

    return is_bad;
}

uint64_t job_reverse_disass_ret_count;
pthread_mutex_t job_reverse_disass_ret_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t job_reverse_disass_ret_count_mutex = PTHREAD_MUTEX_INITIALIZER;

static void job_reverse_disass_ret(job_reverse_disass_ret_th_arg *th)
{
    th->this->reverse_disass_ret(th->this, th->function_chunk, th->addr, th->byte+1, th->inst_list);

    pthread_mutex_lock(&job_reverse_disass_ret_count_mutex);
    job_reverse_disass_ret_count++;
    pthread_mutex_unlock(&job_reverse_disass_ret_count_mutex);

    free(th);
    th = NULL;
}

static status_t reverse_disass_ret(private_plugin_rop_t *this, chunk_t chunk, Elf64_Addr addr, uint64_t ret_byte, linked_list_t *inst_list)
{
    /*char buffer[BUFLEN];*/
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    int64_t current_byte;
    uint64_t last_decoded_byte;
    uint64_t last_byte;
    status_t status;

    linked_list_t *chain_insns;
    chain_t *chain;
    bool byte_to_disass;
    instruction_t *item;

    chunk_t code_chunk;

    disassembler_t *d;
    instruction_t *inst;

    if (!this)
        return FAILED;

    inst = NULL;
    chain_insns = linked_list_create();
    /*
     * Use last_byte = ret_byte - 1 for ret only and to remove last
     * instruction
     */
    last_byte = ret_byte;
    current_byte = last_byte;
    last_decoded_byte = 0;
    byte_to_disass = true;

    d = this->d;

    /*hexdump(chunk.ptr + ret_byte - 19, 20);*/

    while (byte_to_disass)
    {
        if ((current_byte == 0) && (last_decoded_byte == last_byte))
        {
            LOG_ROP_DEBUG("start of chunk reached\n");
            byte_to_disass = false;
        }
        else if /*((last_decoded_byte > 0) &&*/ (((ret_byte - current_byte > 50)))
        {
            LOG_ROP_DEBUG("max decoding size reached\n");
            byte_to_disass = false;
        }
        else if ((last_decoded_byte > 0) && (((last_decoded_byte - current_byte) > 15) || (current_byte < 0)))
        {
            LOG_ROP_DEBUG("no instruction found within maximum instruction length range\n");
            current_byte = last_decoded_byte;

            if (last_decoded_byte == last_byte)
            {
                LOG_ROP_DEBUG("reached last_decoded_byte == last_byte\n");
                byte_to_disass = false;
            }
            else
            {
                /*
                 * Removing last decoded instruction and continue to explore
                 */
                LOG_ROP_DEBUG("removing last decoded instruction\n");
                chain_insns->remove_first(chain_insns, (void**)&item);

                // Recovering offsets and chain string
                last_decoded_byte+= d->get_length(d, item);
                current_byte--;

                item->destroy(item);
            }
        }

        status = FAILED;

        /* Quick fix TODO
         * The memcpy while current_byte<0 (maybe do{}while();)
         */
        if (current_byte >=0)
        {
        if ((last_byte - current_byte) >= XED_MAX_INSTRUCTION_BYTES)
            bytes = XED_MAX_INSTRUCTION_BYTES;
        else
            bytes = last_byte - current_byte;

        memcpy(itext, chunk.ptr + current_byte, bytes);
        code_chunk = chunk_create(itext, bytes);

        status = d->decode(d, &inst, code_chunk);
        }

        if (status == SUCCESS)
        {
            /*
            xed_bool_t ok;

            ok = xed_format(XED_SYNTAX_INTEL, &xedd, buffer, BUFLEN, 0);

            if (!ok)
            {
                LOG_ROP_DEBUG("Error while using xed_format in reverse_disass_ret\n");
            }
            else
            */
            if ((last_decoded_byte > 0) && (d->get_length(d, inst) != (last_decoded_byte - current_byte)))
            {
                LOG_ROP_DEBUG("instruction length is not valid\n");
                /*
                LOG_ROP_DEBUG("decoded:%u vs. l-c:%u\n", xed_decoded_inst_get_length(&xedd), last_decoded_byte - current_byte);
                xed_decoded_inst_dump(&xedd, buf, sizeof(buf));
                LOG_ROP_DEBUG("Invalid:\n%s\n", buf);
                */
            }
            else if ((last_decoded_byte == 0) && (d->get_length(d, inst) != (last_byte - current_byte)))
            {
                LOG_ROP_DEBUG("last_decoded_byte == 0\n");
            }
            else if ((this->is_last_inst(this, inst)) && (chain_insns->get_last(chain_insns, (void**)&item) != NOT_FOUND))
            {
                LOG_ROP_DEBUG("found ending instruction, skipping as position != end (ret; bla; ret)\n");
            }
            else if ((this->is_last_inst(this, inst) == 0) && (chain_insns->get_last(chain_insns, (void**)&item) == NOT_FOUND))
            {
                LOG_ROP_DEBUG("found last instruction not being an ending instruction, skipping\n");
            }
            /*
             * Filtering not wanted insns. HLT/STI
             * XXX Feed using cmd line
             *
             */
            else if ((d->get_length(d, inst) == 1) && (this->bad_insn(this, itext)))
            {
                LOG_ROP_DEBUG("found unwanted instruction\n");
            }
            else
            {
                instruction_t *new_insn;

                LOG_ROP_DEBUG("adding new chain\n");

                last_decoded_byte = current_byte;

                new_insn = inst->clone(inst);

                chain_insns->insert_first(chain_insns, new_insn);

                chain = chain_create_from_insn_disass(this->d, addr + last_decoded_byte, chain_insns);

                pthread_mutex_lock(&job_reverse_disass_ret_mutex);
                inst_list->insert_last(inst_list, chain);
                pthread_mutex_unlock(&job_reverse_disass_ret_mutex);
            }
        }
        else 
        {
            LOG_ROP_DEBUG("wrong decoding\n");
            //hexdump(code_chunk.ptr, code_chunk.len);
        }

        if (current_byte >= 0)
        {
            inst->destroy(inst);
        }

        current_byte--;
    }
    /*LOG_ROP_DEBUG("%08x: %s\n", addr + last_decoded_byte, chain_str);*/

    //chain_insns->destroy_function(chain_insns, this->d->destroy_instruction);

    return SUCCESS;
}

static linked_list_t* find_rop_chains(private_plugin_rop_t *this, chunk_t function_chunk, Elf64_Addr addr)
{
    /*char buffer[BUFLEN];*/
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    uint64_t byte;
    linked_list_t *inst_list;
    uint64_t job_reverse_disass_ret_count_local;
    uint64_t job_reverse_disass_ret_total;
    job_reverse_disass_ret_th_arg *ta;

    disassembler_t *d;
    instruction_t *instruction;
    status_t status;

    inst_list = linked_list_create();

    if (!this)
    {
        return inst_list;
    }

    d = this->d;
    instruction = NULL;
    job_reverse_disass_ret_count = 0;
    job_reverse_disass_ret_count_local = 0;
    job_reverse_disass_ret_total = 0;

    for (byte = 0; byte < function_chunk.len; byte++)
    {
        chunk_t code_chunk;

        if ((function_chunk.len - byte) >= XED_MAX_INSTRUCTION_BYTES)
            bytes = XED_MAX_INSTRUCTION_BYTES;
        else
            bytes = function_chunk.len - byte;

        memcpy(itext, function_chunk.ptr + byte, bytes);
        code_chunk = chunk_create(itext, bytes);

        status = d->decode(d, &instruction, code_chunk);

        if ((status == SUCCESS) && (this->is_last_inst(this, instruction)))
        {
            /*
            chunk_t a = chunk_calloc(1024);
            d->dump_intel(d, instruction, &a, byte);
            LOG_ROP_DEBUG("[x] found last (%x) %s\n", d->get_length(d, instruction), a.ptr);
            */

            job_reverse_disass_ret_total++;
            ta = malloc_thing(job_reverse_disass_ret_th_arg);
            
            ta->this = this;
            ta->function_chunk = function_chunk;
            ta->addr = addr;
            ta->byte = byte + d->get_length(d, instruction) - 1;
            ta->inst_list = inst_list;

#ifdef MULTITHREAD
            thpool_add_work(this->threadpool, (void*)job_reverse_disass_ret, (void*)ta);
#else
            job_reverse_disass_ret(ta);
#endif
        }

        instruction->destroy(instruction);
    }

    while(job_reverse_disass_ret_count_local < job_reverse_disass_ret_total)
    {
        LOG_ROP_DEBUG("jr:%i ir:%i\n", job_reverse_disass_ret_count_local, job_reverse_disass_ret_total);
        pthread_mutex_lock(&job_reverse_disass_ret_count_mutex);
        job_reverse_disass_ret_count_local = job_reverse_disass_ret_count;
        pthread_mutex_unlock(&job_reverse_disass_ret_count_mutex);

        LOG_ROP_DEBUG("jr:%i ir:%i\n", job_reverse_disass_ret_count_local, job_reverse_disass_ret_total);
        usleep(10000);
    }

    return inst_list;
}

int job_count;
pthread_mutex_t job_mutex = PTHREAD_MUTEX_INITIALIZER;

static void job_chain(th_arg *t)
{
    map_t *map;
    map_t *target_map;
    chain_t *c;
    Z3_config cfg;
    Z3_context ctx;
    chunk_t prefix;
    gadget_type g;

    LOG_ROP_DEBUG("***************************************************************************************************\n");
    LOG_ROP_DEBUG("***************************************************************************************************\n");

    cfg = Z3_mk_config();
    ctx = Z3_mk_context(cfg);

    Z3_del_config(cfg);

    c = t->c;

    pthread_mutex_lock(&job_mutex);
    job_count++;

    LOG_ROP_DEBUG("%08x: %s\n", c->get_addr(c), c->get_str(c));
    /*hexdump(c->get_chunk(c).ptr, c->get_chunk(c).len);*/

    //target_map = t->target_map;
    t->target->set_Z3_context(t->target, ctx);

    target_map = t->target->get_map(t->target);

    /*
     * Maybe something better to do with Z3_translate().
     */
    c->set_Z3_context(c, ctx);

    /* Why calloc(2) and set_prefix with calloc(len) does not work?
     */
    prefix = chunk_calloc(3);
    snprintf((char*)prefix.ptr, prefix.len, "t_");
    map = c->get_map_prefix(c, prefix);

    pthread_mutex_unlock(&job_mutex);

    g = target_map->compare(target_map, map);
    if (g != BAD)
    {
        LOG_ROP("Found equivalent! %u\n", g);
        LOG_ROP("   [X] %s\n", c->get_str(c));
        LOG_ROP("   [X] %s\n", t->target->get_str(t->target));
        if (g == PN2)
            printf("   [X] %s\n", c->get_str(c));
    }

    target_map->destroy(target_map);
    map->destroy(map);

    c->destroy(c);

    Z3_del_context(ctx);
    ctx = NULL;

    chunk_free(&prefix);
    // XXX
    free(t);
    t = NULL;
}

void destroy_inst_list(void *chain)
{
    chain_t *c;

    c = (chain_t*) chain;

    c->destroy(c);
}

status_t pack(private_plugin_rop_t *this, Elf64_Addr addr, chunk_t chunk)
{
    linked_list_t *inst_list;

    enumerator_t *e;
    chain_t *c;
    th_arg *ta;
    int jc;


    if ((!this) || (!this->code))
        return FAILED;

    LOG_ROP("Getting target map\n");

    /*
     * It is more efficient to get the target map here and share.
     * However Z3 needs a global context, and we have to include it
     * explicitly each time FIXME
    target_map = this->target->get_map(this->target);
    target_map->dump(target_map);
     */

    LOG_ROP("Finding rop chains\n");

    inst_list = this->find_rop_chains(this, chunk, addr);

    LOG_ROP("Sorting %i elements\n", inst_list->get_count(inst_list));

    inst_list->bsort(inst_list);
    inst_list->unique(inst_list);

    LOG_ROP("Unique %i elements\n", inst_list->get_count(inst_list));

    e = inst_list->create_enumerator(inst_list);
    pthread_mutex_lock(&job_mutex);
    jc = job_count;
    job_count = 0;
    pthread_mutex_unlock(&job_mutex);

    while(e->enumerate(e, &c))
    {
        if (this->target->get_chunk(this->target).ptr == NULL)
        {
            printf("0x%016lx: %s\n", c->get_addr(c), c->get_str(c));
            job_count++;
        }
        else
        {
            ta = malloc_thing(th_arg);
            //ta->target_map = target_map;
            ta->target = this->target;
            ta->c = c;
#ifdef MULTITHREAD
            thpool_add_work(this->threadpool, (void*)job_chain, (void*)ta);
#else
            job_chain(ta);
#endif
        }
    }

    while(jc < inst_list->get_count(inst_list))
    {
        pthread_mutex_lock(&job_mutex);
        jc = job_count;
        pthread_mutex_unlock(&job_mutex);

        LOG_ROP_DEBUG("j:%i i:%i\n", jc, inst_list->get_count(inst_list));
        usleep(10000);
    }

    e->destroy(e);

    //inst_list->destroy_function(inst_list, destroy_inst_list);

    return SUCCESS;
}

static status_t apply(private_plugin_rop_t *this)
{
    Elf64_Addr addr;
    chunk_t chunk;

    enumerator_t *e;
    program_header_t *p;

    /*
    section_t *text_section;
    if ((text_section = this->code->get_section_by_name(this->code, ".text")) == NULL)
    {
        LOG_ROP_DEBUG("Section %s not found.\n", ".text");
        return FAILED;
    }

    printf("Size of .text: %"PRIx64"\n", text_section->get_sh_size(text_section));

    chunk = this->code->get_section_data_chunk(this->code, text_section);
    addr = text_section->get_sh_addr(text_section);

    this->pack(this, addr, chunk);
    */

    /*
    chunk = this->code->get_function_chunk(this->code, "main");
    this->disassemble(this, function_chunk);
    */

    e = this->code->get_program_header_enumerator(this->code);

    while (e->enumerate(e, &p))
    {
        if (p->get_p_flags(p) & PF_X)
        {
            addr = p->get_p_vaddr(p);
            chunk = p->get_chunk(p);
            this->pack(this, addr, chunk);
            chunk_clear(&chunk);
        }
    }

    e->destroy(e);

    return SUCCESS;
}

static void destroy(private_plugin_rop_t *this)
{
    thpool_destroy(this->threadpool);

    this->target->destroy(this->target);

    this->d->destroy(this->d);

    free(this);
    this = NULL;
}

plugin_rop_t *plugin_rop_create(code_t *code, char *constraints, chunk_t target)
{
    private_plugin_rop_t *this = malloc_thing(private_plugin_rop_t);

    module_call_init(MODULE_INIT_QOM);

    tcg_exec_init(0);
    cpu_exec_init_all();

    this->threadpool=thpool_init(32);
    //this->d = (disassembler_t*) create_xed();
    this->d = (disassembler_t*) DISASSINSTANCE();

    this->code = (elf_t *) code;
    this->constraints = constraints;

    this->code_type = ((code_t*) this->code)->get_type((code_t*) this->code);
    this->d->initialize(this->d, this->code_type);
    this->target = chain_create_from_string_disass(this->d, 0x400000, target);

    this->public.interface.apply = (status_t (*)(plugin_t *)) apply;
    this->public.interface.destroy = (void (*)(plugin_t *)) destroy;

    this->disassemble = (status_t (*)(private_plugin_rop_t *, chunk_t)) disassemble;
    this->is_last_inst = (bool (*)(private_plugin_rop_t *, instruction_t*)) is_last_inst;
    this->bad_insn = (bool (*)(private_plugin_rop_t *, unsigned char *)) bad_insn;
    this->pack = (status_t (*)(private_plugin_rop_t *, Elf64_Addr, chunk_t)) pack;
    this->find_rop_chains = (linked_list_t *(*)(private_plugin_rop_t *, chunk_t, Elf64_Addr)) find_rop_chains;
    this->reverse_disass_ret = (status_t (*)(private_plugin_rop_t *, chunk_t, Elf64_Addr, uint64_t, linked_list_t *)) reverse_disass_ret;

    return &this->public;
}
