/*
 * Implementation of rop insertion plugin
 */

#include "plugin_rop.h"

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
    status_t (*reverse_disass_ret)(private_plugin_rop_t *, chunk_t, Elf64_Addr, instruction_t**, uint64_t, linked_list_t*);
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

        free(instruction);
        instruction = NULL;
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

static status_t reverse_disass_ret(private_plugin_rop_t *this, chunk_t chunk, Elf64_Addr addr, instruction_t **inst, uint64_t ret_byte, linked_list_t *inst_list)
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
    char *item;

    disassembler_t *d;

    chain_insns = linked_list_create();
    /*
     * Use last_byte = ret_byte - 1 for ret only and to remove last
     * instruction
     */
    last_byte = ret_byte;
    current_byte = last_byte;
    last_decoded_byte = 0;
    byte_to_disass = true;

    if (!this)
    {
        return FAILED;
    }

    d = this->d;

    /*hexdump(chunk.ptr + ret_byte - 19, 20);*/

    while (byte_to_disass)
    {
        if ((current_byte == 0) && (last_decoded_byte == last_byte))
        {
            /*logging("start of chunk reached\n");*/
            byte_to_disass = false;
        }
        else if /*((last_decoded_byte > 0) &&*/ (((ret_byte - current_byte > 50)))
        {
            /*logging("max decoding size reached\n");*/
            byte_to_disass = false;
        }
        else if ((last_decoded_byte > 0) && (((last_decoded_byte - current_byte) > 15) || (current_byte < 0)))
        {
            /*logging("no instruction found within maximum instruction length range\n");*/
            current_byte = last_decoded_byte;

            if (last_decoded_byte == last_byte)
            {
                byte_to_disass = false;
            }
            else
            {
                /*
                 * Removing last decoded instruction and continue to explore
                 */
                uint64_t inst_size;
                chunk_t code_chunk;

                if ((last_byte - current_byte) >= XED_MAX_INSTRUCTION_BYTES)
                    bytes = XED_MAX_INSTRUCTION_BYTES;
                else
                    bytes = last_byte - current_byte;

                memcpy(itext, chunk.ptr + current_byte, bytes);
                code_chunk = chunk_create(itext, bytes);

                /* No error as we already tested it */
                d->decode(d, inst, code_chunk);

                /* Recovering offsets and chain string */
                inst_size = d->get_length(d, *inst);
                last_decoded_byte+= inst_size;
                current_byte--;

                chain_insns->remove_first(chain_insns, (void**)&item);
                free(item);
                item = NULL;
            }
        }

        status = FAILED;

        /* Quick fix TODO
         * The memcpy while current_byte<0
         */
        if (current_byte >=0)
        {
        chunk_t code_chunk;

        if ((last_byte - current_byte) >= XED_MAX_INSTRUCTION_BYTES)
            bytes = XED_MAX_INSTRUCTION_BYTES;
        else
            bytes = last_byte - current_byte;

        memcpy(itext, chunk.ptr + current_byte, bytes);
        code_chunk = chunk_create(itext, bytes);

        status = d->decode(d, inst, code_chunk);
        }

        if (status == SUCCESS)
        {
            /*
            xed_bool_t ok;

            ok = xed_format(XED_SYNTAX_INTEL, &xedd, buffer, BUFLEN, 0);

            if (!ok)
            {
                logging("Error while using xed_format in reverse_disass_ret\n");
            }
            else
            */
            if ((last_decoded_byte > 0) && (d->get_length(d, *inst) != (last_decoded_byte - current_byte)))
            {
                /*
                logging("instruction length is not valid\n");
                logging("decoded:%u vs. l-c:%u\n", xed_decoded_inst_get_length(&xedd), last_decoded_byte - current_byte);
                xed_decoded_inst_dump(&xedd, buf, sizeof(buf));
                logging("Invalid:\n%s\n", buf);
                */
            }
            else if ((last_decoded_byte == 0) && (d->get_length(d, *inst) != (last_byte - current_byte)))
            {}
            else if ((this->is_last_inst(this, *inst)) && (chain_insns->get_last(chain_insns, (void**)&item) != NOT_FOUND))
            {
                /*logging("found ending instruction, skipping as position != end (ret; bla; ret)\n");*/
            }
            else if ((this->is_last_inst(this, *inst) == 0) && (chain_insns->get_last(chain_insns, (void**)&item) == NOT_FOUND))
            {
                /*logging("found last instruction not being an ending instruction, skipping\n");*/
            }
            /*
             * Filtering not wanted insns. HLT/STI
             * XXX Feed using cmd line
             *
             */
            else if ((d->get_length(d, *inst) == 1) && (this->bad_insn(this, itext)))
            {
            }
            else
            {
                instruction_t *new_insn;

                last_decoded_byte = current_byte;
                /*current_byte-= xed_decoded_inst_get_length(&xedd);*/

                new_insn = d->alloc_instruction(d);

                memcpy(new_insn, *inst, d->get_instruction_size(d));

                chain_insns->insert_first(chain_insns, new_insn);

                chain = chain_create_from_insn(this->code_type, addr + last_decoded_byte, chain_insns);

                inst_list->insert_last(inst_list, chain);
            }
        }
        else
        {
            /*logging("wrong decoding\n");*/
        }

        current_byte--;
    }
    /*logging("%08x: %s\n", addr + last_decoded_byte, chain_str);*/

    chain_insns->destroy_function(chain_insns, free);

    //d->destroy(d);

    return SUCCESS;
}

static linked_list_t* find_rop_chains(private_plugin_rop_t *this, chunk_t function_chunk, Elf64_Addr addr)
{
    /*char buffer[BUFLEN];*/
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    uint64_t byte;
    linked_list_t *inst_list;

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

        if (status == SUCCESS)
        {
            if (this->is_last_inst(this, instruction))
            {
                /*
                logging("Found a RET ins @%x\n", instruction);
                char buffer[4096];
                xed_decoded_inst_dump(&xedd,buffer, sizeof(buffer));
                printf("%s\n",buffer);
                */
                this->reverse_disass_ret(this, function_chunk, addr, &instruction, byte+1, inst_list);
            }
        }
    }

    //d->destroy(d);

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

    logging("***************************************************************************************************\n");
    logging("***************************************************************************************************\n");

    pthread_mutex_lock(&job_mutex);
    job_count++;
    pthread_mutex_unlock(&job_mutex);

    cfg = Z3_mk_config();
    ctx = Z3_mk_context(cfg);

    Z3_del_config(cfg);

    c = t->c;

    logging("%08x: %s\n", c->get_addr(c), c->get_str(c));
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

    g = target_map->compare(target_map, map);
    if (g != BAD)
    {
        logging("Found equivalent! %u\n", g);
        logging("   [X] %s\n", c->get_str(c));
        logging("   [X] %s\n", t->target->get_str(t->target));
        map->dump(map);
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

status_t pack(private_plugin_rop_t *this, Elf64_Addr addr, chunk_t chunk)
{
    linked_list_t *inst_list;

    enumerator_t *e;
    chain_t *c;
    th_arg *ta;
    int jc;


    if ((!this) || (!this->code))
        return FAILED;

    logging("Getting target map\n");

    /*
     * It is more efficient to get the target map here and share.
     * However Z3 needs a global context, and we have to include it
     * explicitly each time FIXME
    target_map = this->target->get_map(this->target);
    target_map->dump(target_map);
     */

    logging("Finding rop chains\n");

    inst_list = this->find_rop_chains(this, chunk, addr);

    logging("Sorting %i elements\n", inst_list->get_count(inst_list));

    inst_list->bsort(inst_list);
    inst_list->unique(inst_list);

    logging("Unique %i elements\n", inst_list->get_count(inst_list));

    e = inst_list->create_enumerator(inst_list);
    pthread_mutex_lock(&job_mutex);
    jc = job_count;
    job_count = 0;
    pthread_mutex_unlock(&job_mutex);

    while(e->enumerate(e, &c))
    {
        ta = malloc_thing(th_arg);
        //ta->target_map = target_map;
        ta->target = this->target;
        ta->c = c;
        //thpool_add_work(this->threadpool, (void*)job_chain, (void*)ta);
        job_chain(ta);
    }

    while(jc < inst_list->get_count(inst_list))
    {
        pthread_mutex_lock(&job_mutex);
        jc = job_count;
        pthread_mutex_unlock(&job_mutex);

        logging("j:%i i:%i\n", jc, inst_list->get_count(inst_list));
        usleep(10000);
    }

    e->destroy(e);

    inst_list->destroy(inst_list);

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
        logging("Section %s not found.\n", ".text");
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

    this->threadpool=thpool_init(8);
    this->d = (disassembler_t*) create_xed();

    this->code = (elf_t *) code;
    this->constraints = constraints;
    this->code_type = ((code_t*) this->code)->get_type((code_t*) this->code);
    this->target = chain_create_from_string(this->code_type, 0x400000, target);

    this->d->initialize(this->d, this->code_type);

    this->public.interface.apply = (status_t (*)(plugin_t *)) apply;
    this->public.interface.destroy = (void (*)(plugin_t *)) destroy;

    this->disassemble = (status_t (*)(private_plugin_rop_t *, chunk_t)) disassemble;
    this->is_last_inst = (bool (*)(private_plugin_rop_t *, instruction_t*)) is_last_inst;
    this->bad_insn = (bool (*)(private_plugin_rop_t *, unsigned char *)) bad_insn;
    this->pack = (status_t (*)(private_plugin_rop_t *, Elf64_Addr, chunk_t)) pack;
    this->find_rop_chains = (linked_list_t *(*)(private_plugin_rop_t *, chunk_t, Elf64_Addr)) find_rop_chains;
    this->reverse_disass_ret = (status_t (*)(private_plugin_rop_t *, chunk_t, Elf64_Addr, instruction_t **, uint64_t, linked_list_t *)) reverse_disass_ret;

    return &this->public;
}
