/**
 * Implementation of chain_t
 */

#include "chain.h"
#include <setjmp.h>
#include "qemu/include/exec/exec-all.h"
#include "qemu/tcg/tcg.h"
#include "converter.h"

typedef struct private_chain_t private_chain_t;

struct private_chain_t
{
    chain_t public;

    uint64_t addr;
    char *str;
    chunk_t chunk;
    linked_list_t *instructions;
    Z3_context ctx;
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

void *clone_instruction(void *instruction)
{
    xed_decoded_inst_t *new_instruction;

    if ((new_instruction = malloc(sizeof(xed_decoded_inst_t))) == NULL)
    {
        logging("Error while allocating instruction in clone_instruction from chain.c\n");
        return NULL;
    }

    memcpy(new_instruction, instruction, sizeof(xed_decoded_inst_t));

    return new_instruction;
}

void destroy_instruction(void *instruction)
{
    free(instruction);
    instruction = NULL;
}

static void set_instructions(private_chain_t *this, linked_list_t *instructions)
{
    this->instructions = instructions->clone_function(instructions, clone_instruction);
}

static linked_list_t *get_instructions(private_chain_t *this)
{
    return this->instructions;
}

static void set_chunk(private_chain_t *this, chunk_t chunk)
{
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

pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;

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
    /*    
    TranslationBlock *tb;
    */
    TCGContext *s;

    converter_t *converter;

    hexdump(this->chunk.ptr, this->chunk.len);
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

    pthread_mutex_lock(&map_lock);

    env = cpu_init("qemu64");
    cpu = ENV_GET_CPU(env); 
    //tb = tb_gen_code(cpu, (uint64_t) this->chunk.ptr, 0, 0x40c0b3, 0);
    tb_gen_code(cpu, (uint64_t) this->chunk.ptr, 0, 0x40c0b3, 0);

    s = get_tcg_ctx();
    tcg_dump_ops(s);

    if (this->ctx == NULL)
        logging("The Z3 context is NULL, will segfault\n");

    converter = converter_create(s, this->ctx);
    converter->set_prefix(converter, prefix);
    converter->tcg_to_llvm(converter);
    converter->dump(converter);

    map = converter->llvm_to_z3(converter);

    converter->destroy(converter);

    pthread_mutex_unlock(&map_lock);

    /*
    chain_list = this->public.get_instructions(&this->public);
    chain_i = chain_list->create_enumerator(chain_list);

     * logging("Dumping insns:\n");

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
    free(this->str);
    this->str = NULL;

    chunk_clear(&this->chunk);
    this->chunk = chunk_empty;

    this->instructions->destroy_function(this->instructions, destroy_instruction);

    free(this);
    this = NULL;
}

chain_t *chain_create_from_string(uint64_t addr, chunk_t chunk_str)
{
    linked_list_t *instructions;
    chain_t *res;
    chunk_t chunk_hex;

    xed_decoded_inst_t *xedd;
    xed_error_enum_t xed_error;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];

    size_t count;
    size_t bytes;

    instructions = linked_list_create();
    chunk_hex = chunk_from_hex(chunk_str, NULL);
    count = 0;

    xed_tables_init();

    while (count < chunk_hex.len)
    {
        if ((xedd = malloc(sizeof(*xedd))) == NULL)
        {
            logging("Error while allocating xedd value in chain_create_from_string from chain.c\n");
            return NULL;
        }

        xed_decoded_inst_zero(xedd);
        xed_decoded_inst_set_mode(xedd, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);

        if ((chunk_hex.len - count) >= XED_MAX_INSTRUCTION_BYTES)
            bytes = XED_MAX_INSTRUCTION_BYTES;
        else
            bytes = chunk_hex.len - count;

        memset(itext, 0, sizeof(itext));
        memcpy(itext, chunk_hex.ptr + count, bytes);

        hexdump(chunk_hex.ptr + count, bytes);

        xed_error = xed_decode(xedd, itext, bytes);

        switch(xed_error)
        {
          case XED_ERROR_NONE:
            break;
          case XED_ERROR_BUFFER_TOO_SHORT:
            logging("Not enough bytes provided\n");
            break;
          case XED_ERROR_GENERAL_ERROR:
            logging("Could not decode given input.\n");
            break;
          default:
            logging("Unhandled error code %s\n",
                    xed_error_enum_t2str(xed_error));
            break;
        }

        count+= xed_decoded_inst_get_length(xedd);

        char buf[4096];
        unsigned int buflen = 4096;

        xed_decoded_inst_dump_intel_format(xedd, buf, buflen, 0);
        logging("%s\n", buf);

        instructions->insert_last(instructions, xedd);
    }

    chunk_clear(&chunk_hex);

    res = chain_create_from_insn(addr, instructions);

    instructions->destroy_function(instructions, free);

    return res;
}

chain_t *chain_create_from_insn(uint64_t addr, linked_list_t *instructions)
{
    char *insns_str;
    chunk_t insns_chunk;
    enumerator_t *e;
    xed_decoded_inst_t *x;
    uint64_t offset_addr;
    chain_t *ret_chain;
    unsigned int ilen, olen;
    unsigned char *itext;
    xed_error_enum_t xed_error;

    if ((insns_str = calloc(4096, 1)) == NULL)
    {
        logging("Error while allocating insns_chunk in chain_create_from_insn\n");
        return NULL;
    }

    insns_chunk = chunk_empty;
    offset_addr = addr;
    ilen = XED_MAX_INSTRUCTION_BYTES;
    olen = 0;

    e = instructions->create_enumerator(instructions);

    while (e->enumerate(e, &x))
    {
        char new_str[4096];
        chunk_t new_chunk;

        xed_decoded_inst_dump_intel_format(x, new_str, sizeof(new_str), offset_addr);

        if ((itext = calloc(sizeof(itext), 1)) == NULL)
        {
            logging("Error while allocating itext in chain_create_from_insn\n");
            return NULL;
        }

        xed_encoder_request_init_from_decode(x);
        xed_error = xed_encode(x, itext, ilen, &olen);

        if (xed_error != XED_ERROR_NONE) {
            fprintf(stderr,"ENCODE ERROR: %s\n",
                xed_error_enum_t2str(xed_error));
        }

        new_chunk = chunk_create(itext, olen);

        if (strlen(insns_str) > 0)
        {
            strcat(insns_str, " ");
            strcat(insns_str, new_str);
            strcat(insns_str, " ;");
        }
        else
            sprintf(insns_str, "%s ;", new_str);
        insns_chunk = chunk_cat("mm", insns_chunk, new_chunk);

        offset_addr+= xed_decoded_inst_get_length(x);
    }

    e->destroy(e);

    ret_chain = chain_create(addr, insns_str, insns_chunk, instructions);

    free(insns_str);
    chunk_clear(&insns_chunk);

    return ret_chain;
}

chain_t *chain_create(uint64_t addr, char *str, chunk_t chunk, linked_list_t *instructions)
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
    this->public.set_str(&this->public, str);
    this->public.set_chunk(&this->public, chunk);
    this->public.set_instructions(&this->public, instructions);

    return &this->public;
}
