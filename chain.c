/**
 * Implementation of chain_t
 */

#include "chain.h"
#include <setjmp.h>
#include "qemu/include/exec/exec-all.h"
#include "qemu/tcg/tcg.h"
#include "converter.h"

//#define DEBUG_CHAIN
#ifdef DEBUG_CHAIN
#  define LOG_CHAIN(...) logging(__VA_ARGS__)
#else
#  define LOG_CHAIN(...) do { } while (0)
#endif

typedef struct private_chain_t private_chain_t;

struct private_chain_t
{
    chain_t public;

    uint64_t addr;
    char *str;
    chunk_t chunk;
    linked_list_t *instructions;
    Z3_context ctx;
    disassembler_t *d;
};

TCGContext *get_tcg_ctx(void);

static void set_addr(private_chain_t *this, uint64_t addr)
{
    this->addr = addr;
}

static uint64_t get_addr(private_chain_t *this)
{
    return this->addr;
}

static void set_str(private_chain_t *this, char *str)
{
    this->str = malloc(strlen(str)+1);

    memcpy(this->str, str, strlen(str)+1);
}

static char *get_str(private_chain_t *this)
{
    return this->str;
}

static void set_instructions(private_chain_t *this, linked_list_t *instructions)
{
    enumerator_t *e;
    instruction_t *instruction;

	linked_list_t *clone = linked_list_create();

    e = instructions->create_enumerator(instructions);

    while (e->enumerate(e, &instruction))
        clone->insert_last(clone, instruction->clone(instruction));

    e->destroy(e);

    //this->instructions = instructions->clone_function(instructions, this->d->clone_instruction);
    this->instructions = clone;
}

static linked_list_t *get_instructions(private_chain_t *this)
{
    return this->instructions;
}

static void set_chunk(private_chain_t *this, chunk_t chunk)
{
    if ((chunk.ptr == NULL) || (chunk.len == 0))
        logging("Setting NULL chunk\n");

    this->chunk = chunk_clone(chunk);
}

static chunk_t get_chunk(private_chain_t *this)
{
    return this->chunk;
}

static void set_Z3_context(private_chain_t *this, Z3_context ctx)
{
    this->ctx = ctx;
}

pthread_mutex_t qemu_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t llvm_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t z3_lock = PTHREAD_MUTEX_INITIALIZER;

static map_t *get_map(private_chain_t *this)
{
    return this->public.get_map_prefix(&this->public, chunk_empty);
}

static map_t *get_map_prefix(private_chain_t *this, chunk_t prefix)
{
    map_t *map;
    /*
    enumerator_t *chain_i;
    linked_list_t *chain_list;
    xed_decoded_inst_t *xedd;
    */

    CPUArchState *env;
    CPUState *cpu;
    TranslationBlock *tb;
    TCGContext *s;

    converter_t *converter;

    //hexdump(this->chunk.ptr, this->chunk.len);
/*
#0  tcg_gen_code (s=s@entry=0x62362ea0 <tcg_ctx>, gen_code_buf=0x6035f820 <static_code_gen_buffer> "") at /home/dad/Outils/packerLLVM/qemu/tcg/tcg.c:2585
#1  0x00000000600d03c7 in cpu_x86_gen_code (env=env@entry=0x623c6950, tb=tb@entry=0x7ffff2875010, gen_code_size_ptr=gen_code_size_ptr@entry=0x7fffffffd304)
    at /home/dad/Outils/packerLLVM/qemu/translate-all.c:179
#2  0x00000000600d0b93 in tb_gen_code (env=env@entry=0x623c6950, pc=0x4000b0, cs_base=0x0, flags=<optimized out>, cflags=cflags@entry=0x0)
    at /home/dad/Outils/packerLLVM/qemu/translate-all.c:969
#3  0x0000000060035935 in tb_find_slow (flags=<optimized out>, cs_base=<optimized out>, pc=<optimized out>, env=0x623c6950)
    at /home/dad/Outils/packerLLVM/qemu/cpu-exec.c:163
#4  tb_find_fast (env=0x623c6950) at /home/dad/Outils/packerLLVM/qemu/cpu-exec.c:190
#5  cpu_x86_exec (env=env@entry=0x623c6950) at /home/dad/Outils/packerLLVM/qemu/cpu-exec.c:607
#6  0x000000006004d820 in cpu_loop (env=env@entry=0x623c6950) at /home/dad/Outils/packerLLVM/qemu/linux-user/main.c:287
#7  0x00000000600085ff in main (argc=<optimized out>, argv=<optimized out>, envp=<optimized out>) at /home/dad/Outils/packerLLVM/qemu/linux-user/main.c:4367
#8  0x00007ffff66fdb05 in __libc_start_main () from /usr/lib/libc.so.6
#9  0x00000000600087a5 in _start ()
    */

    pthread_mutex_lock(&qemu_lock);

    env = cpu_init("qemu64");
    cpu = ENV_GET_CPU(env); 
    tb = tb_gen_code(cpu, (uint64_t) this->chunk.ptr, 0, 0x40c0b3, 0);
    //tb_gen_code(cpu, (uint64_t) this->chunk.ptr, 0, 0x40c0b3, 0);

    s = get_tcg_ctx();
    //tcg_dump_ops(s);

    if (this->ctx == NULL)
        LOG_CHAIN("The Z3 context is NULL, will segfault\n");

    pthread_mutex_unlock(&qemu_lock);
    pthread_mutex_lock(&llvm_lock);

    converter = converter_create(s, this->ctx);
    converter->set_prefix(converter, prefix);
    converter->tcg_to_llvm(converter);
    //converter->dump(converter);

    pthread_mutex_unlock(&llvm_lock);
    //pthread_mutex_lock(&z3_lock);

    map = converter->llvm_to_z3(converter);

    converter->destroy(converter);

    tb_free(tb);

    //pthread_mutex_unlock(&z3_lock);

    /*
    chain_list = this->public.get_instructions(&this->public);
    chain_i = chain_list->create_enumerator(chain_list);

     * LOG_CHAIN("Dumping insns:\n");

    while(chain_i->enumerate(chain_i, &xedd))
    {
        hexdump(&xedd->_byte_array._enc, xed_decoded_inst_get_length(xedd));
    }

    chain_i->destroy(chain_i);
    */

    return map;
}

static void destroy(private_chain_t *this)
{
    enumerator_t *e;
    instruction_t *instruction;

    free(this->str);
    this->str = NULL;

    chunk_clear(&this->chunk);
    this->chunk = chunk_empty;

    e = this->instructions->create_enumerator(this->instructions);

    while (e->enumerate(e, &instruction))
        instruction->destroy(instruction);

    this->instructions->destroy(this->instructions);

    free(this);
    this = NULL;
}

chain_t *chain_create_from_string(chunk_t type, uint64_t addr, chunk_t chunk_str)
{
    disassembler_t *d;

    d = (disassembler_t*) DISASSINSTANCE();
    d->initialize(d, type);

    LOG_CHAIN("Creating chain of type %s[%u] @%x : %s\n", type.ptr, type.len, addr, chunk_str.ptr);

    return chain_create_from_string_disass(d, addr, chunk_str);
}

chain_t *chain_create_from_string_disass(disassembler_t *d, uint64_t addr, chunk_t chunk_str)
{
    linked_list_t *instructions;
    chain_t *res;
    chunk_t chunk_hex;

    instruction_t *instruction;

    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];

    size_t count;
    size_t bytes;

    instructions = linked_list_create();
    chunk_hex = chunk_from_hex(chunk_str, NULL);
    count = 0;

    while (count < chunk_hex.len)
    {
        chunk_t code_chunk;
        chunk_t format_chunk;

        if ((chunk_hex.len - count) >= XED_MAX_INSTRUCTION_BYTES)
            bytes = XED_MAX_INSTRUCTION_BYTES;
        else
            bytes = chunk_hex.len - count;

        memset(itext, 0, sizeof(itext));
        memcpy(itext, chunk_hex.ptr + count, bytes);
        code_chunk = chunk_create(itext, bytes);

        hexdump(chunk_hex.ptr + count, bytes);

        d->decode(d, &instruction, code_chunk);

        count+= d->get_length(d, instruction);

        /*
        char buf[4096];
        unsigned int buflen = 4096;

        xed_decoded_inst_dump_intel_format(xedd, buf, buflen, 0);
        LOG_CHAIN("%s\n", buf);
        */
        format_chunk = chunk_calloc(4096);
        if (d->dump_intel(d, instruction, &format_chunk, addr) == FAILED)
            LOG_CHAIN("[x] Error while dump_intel in chain.c\n");

        LOG_CHAIN("create from string_disass %s\n", format_chunk.ptr);

        chunk_free(&format_chunk);

        instructions->insert_last(instructions, instruction);
    }

    chunk_clear(&chunk_hex);

    res = chain_create_from_insn_disass(d, addr, instructions);

    instructions->destroy_function(instructions, free);

    return res;
}

chain_t *chain_create_from_insn(chunk_t type, uint64_t addr, linked_list_t *instructions)
{
    disassembler_t *d;

    d = (disassembler_t*) DISASSINSTANCE();
    d->initialize(d, type);

    return chain_create_from_insn_disass(d, addr, instructions);
}

chain_t *chain_create_from_insn_disass(disassembler_t *d, uint64_t addr, linked_list_t *instructions)
{
    char *insns_str;
    chunk_t insns_chunk;
    enumerator_t *e;
    uint64_t offset_addr;
    chain_t *ret_chain;

    instruction_t *instruction;

    if ((insns_str = calloc(4096, 1)) == NULL)
    {
        LOG_CHAIN("Error while allocating insns_chunk in chain_create_from_insn\n");
        return NULL;
    }

    insns_chunk = chunk_empty;
    offset_addr = addr;

    e = instructions->create_enumerator(instructions);

    while (e->enumerate(e, &instruction))
    {
        chunk_t new_str;
        chunk_t new_chunk;
        bool need_free_str;
        bool need_free_chunk;

        /**
         * Create string representation
         */
        need_free_str = false;
        new_str = instruction->str;

        LOG_CHAIN("new str is %x:%x\n", new_str.ptr, new_str.len);

        if (new_str.ptr == chunk_empty.ptr)
        {
            new_str = chunk_calloc(4096);

            LOG_CHAIN("dumping @instruction %x:%x\n", instruction, offset_addr);
            d->dump_intel(d, instruction, &new_str, offset_addr);

            need_free_str = true;
        }

        LOG_CHAIN("new str is now %x:%x\n", new_str.ptr, new_str.len);

        if (strlen(insns_str) > 0)
        {
            strcat(insns_str, " ");
            strcat(insns_str, (char*) new_str.ptr);
            strcat(insns_str, " ;");
        }
        else
            sprintf(insns_str, "%s ;", (char*) new_str.ptr);

        /**
         * Create bytes representing the instruction
         */
        need_free_chunk = false;
        new_chunk = instruction->bytes;
        LOG_CHAIN("new chunk is %x:%x\n", new_chunk.ptr, new_chunk.len);

        if (new_chunk.ptr == chunk_empty.ptr)
        {
            if (d->encode(d, &new_chunk, instruction) == FAILED)
            {
                LOG_CHAIN("Error while encoding chunk in chain_create_from_insn\n");
                break;
            }

            need_free_chunk = true;
        }

        LOG_CHAIN("new chunk is %x:%x\n", new_chunk.ptr, new_chunk.len);

        /**
         * FIXME
         * Terrible mem leak here, should replace with tmpvar
         */
        insns_chunk = chunk_cat("cc", insns_chunk, new_chunk);

        if (need_free_str)
            chunk_clear(&new_str);
        if (need_free_chunk)
            chunk_clear(&new_chunk);

        offset_addr+= d->get_length(d, instruction);
    }

    e->destroy(e);

    if ((insns_chunk.ptr == NULL) || (insns_chunk.len == 0))
        LOG_CHAIN("Creating chain with NULL insns_chunk\n");

    ret_chain = chain_create(d, addr, insns_str, insns_chunk, instructions);

    free(insns_str);
    chunk_clear(&insns_chunk);

    return ret_chain;
}

chain_t *chain_create(disassembler_t *d, uint64_t addr, char *str, chunk_t chunk, linked_list_t *instructions)
{
    private_chain_t *this = malloc_thing(private_chain_t);

    this->public.set_addr = (void (*)(chain_t *, uint64_t)) set_addr;
    this->public.set_str = (void (*)(chain_t *, char *)) set_str;
    this->public.set_chunk = (void (*)(chain_t *, chunk_t)) set_chunk;
    this->public.set_instructions = (void (*)(chain_t *, linked_list_t *)) set_instructions;
    this->public.set_Z3_context = (void (*)(chain_t *, Z3_context)) set_Z3_context;
    this->public.get_addr = (uint64_t (*)(chain_t *)) get_addr;
    this->public.get_str = (char *(*)(chain_t *)) get_str;
    this->public.get_chunk = (chunk_t (*)(chain_t *)) get_chunk;
    this->public.get_instructions = (linked_list_t *(*)(chain_t *)) get_instructions;
    this->public.get_map = (map_t *(*)(chain_t *)) get_map;
    this->public.get_map_prefix = (map_t *(*)(chain_t *, chunk_t)) get_map_prefix;
    this->public.destroy = (void (*)(chain_t *)) destroy;

    this->addr = addr;
    this->d = d;
    this->public.set_str(&this->public, str);
    this->public.set_chunk(&this->public, chunk);
    this->public.set_instructions(&this->public, instructions);

    return &this->public;
}
