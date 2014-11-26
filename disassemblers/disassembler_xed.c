/*
 * Implementation of disassembler
 */

#include "disassembler_xed.h"

#define BUFLEN  1000

//#define DEBUG_XED
#ifdef DEBUG_XED
#  define LOG_XED(...) logging(__VA_ARGS__)
#else
#  define LOG_XED(...) do { } while (0)
#endif

typedef struct private_disass_xed_t private_disass_xed_t;

struct private_disass_xed_t
{
    disass_xed_t public;

    xed_machine_mode_enum_t mmode;
    xed_address_width_enum_t stack_addr_width;
};

typedef struct xed_instruction_t xed_instruction_t;

struct xed_instruction_t
{
    instruction_t interface;

    xed_decoded_inst_t xedd;
};

static status_t initialize(private_disass_xed_t *this, chunk_t c)
{
    xed_bool_t long_mode;

    long_mode = (strncmp((char*) c.ptr, "ELF64", 5) == 0);

    if (long_mode) {
        this->mmode=XED_MACHINE_MODE_LONG_64;
        this->stack_addr_width =XED_ADDRESS_WIDTH_64b;
    }
    else {
        this->mmode=XED_MACHINE_MODE_LEGACY_32;
        this->stack_addr_width =XED_ADDRESS_WIDTH_32b;
    }

    return SUCCESS;
}

static category_t get_category(private_disass_xed_t *this, instruction_t *i)
{
    xed_category_enum_t cat;

    if (!this)
        return NO_CAT;

    cat = xed_decoded_inst_get_category(&((xed_instruction_t*) i)->xedd);

    switch (cat)
    {
        case XED_CATEGORY_COND_BR:
            return COND_BR;
        case XED_CATEGORY_UNCOND_BR:
            return UNCOND_BR;
        case XED_CATEGORY_SYSCALL:
            return SYSCALL;
        case XED_CATEGORY_CALL:
            return CALL;
        case XED_CATEGORY_RET:
            return RET;
        default:
            return NO_CAT;
    }
}

static uint64_t get_length(private_disass_xed_t *this, instruction_t *i)
{
    if (!this)
        return 0;

    return xed_decoded_inst_get_length(&((xed_instruction_t*) i)->xedd);
}

static status_t format(private_disass_xed_t *this, instruction_t *i, chunk_t format)
{
    xed_bool_t ok;
    xed_decoded_inst_t *xedd;

    if (!this)
        return FAILED;

    xedd = &((xed_instruction_t*) i)->xedd;

    ok = xed_format(XED_SYNTAX_INTEL, xedd, (char*)format.ptr, format.len, 0);

    if (ok)
        return SUCCESS;
    else
    {
        logging("[x] Error while format in disassembler_xed.c : DISASSEMBLY ERROR: %p\n", xedd);
        return FAILED;
    }

    return SUCCESS;
}

static status_t dump_intel(private_disass_xed_t *this, instruction_t *i, chunk_t buffer, uint64_t offset_addr)
{
    xed_decoded_inst_t *x;
    xed_bool_t xed_error;

    if (!this)
        return FAILED;

    x = &((xed_instruction_t*) i)->xedd;

    LOG_XED("[XED] dump_intel(%x, %x, %x, %x): %x\n", x, buffer.ptr, buffer.len, offset_addr, *x);

    xed_error = xed_decoded_inst_dump_intel_format(x, (char*)buffer.ptr, buffer.len, offset_addr);

    LOG_XED("   = %x\n", xed_error);

    if (!xed_error)
        return FAILED;
    else
        return SUCCESS;
}

static status_t decode(private_disass_xed_t *this, instruction_t **i, chunk_t c)
{
    xed_decoded_inst_t *xedd;
    xed_error_enum_t xed_error;
    status_t status;

    if ((*i = (instruction_t*) malloc_thing(xed_instruction_t)) == NULL)
        logging("[x] Error while allocating xed_instruction_t in disassembler_xed.c\n");

    xedd = &((xed_instruction_t*) *i)->xedd;
    LOG_XED("instruction @%x xedd @%x\n", *i, xedd);

    xed_decoded_inst_zero(xedd);
    xed_decoded_inst_set_mode(xedd, this->mmode, this->stack_addr_width);

    xed_error = xed_decode(xedd, c.ptr, c.len);

    switch(xed_error)
    {
      case XED_ERROR_NONE:
        status = SUCCESS;
        break;
      case XED_ERROR_BUFFER_TOO_SHORT:
        LOG_XED("Error in decode : Not enough bytes provided\n");
        status = FAILED;
        break;
      case XED_ERROR_GENERAL_ERROR:
        LOG_XED("Error in decode : Could not decode given input.\n");
        status = FAILED;
        break;
      default:
        LOG_XED("Error in decode : Unhandled error code %s\n",
                xed_error_enum_t2str(xed_error));
        status = FAILED;
        break;
    }

    if (status == FAILED)
        return status;

    /* FIXME
     * Why is this code here??
     *
    char buffer[BUFLEN];
    xed_bool_t ok;

    ok = xed_format(XED_SYNTAX_INTEL, xedd, buffer, BUFLEN, 0);
    LOG_XED("formatting\n");

    if (ok)
        LOG_XED("\t%s\n", buffer);
    else
        logging("DISASSEMBLY ERROR: %x\n", xedd);
     */

    return SUCCESS;
}

static status_t encode(private_disass_xed_t *this, chunk_t *c, instruction_t *i)
{
    unsigned int ilen, olen;
    xed_error_enum_t xed_error;
    unsigned char *itext;
    xed_decoded_inst_t *x;

    if (!this)
        return FAILED;

    x = &((xed_instruction_t*) i)->xedd;

    ilen = XED_MAX_INSTRUCTION_BYTES;
    olen = 0;
    
    if ((itext = calloc(XED_MAX_INSTRUCTION_BYTES + 1, 1)) == NULL)
    {
        logging("Error while allocating itext in chain_create_from_insn\n");
        return FAILED;
    }

    xed_encoder_request_init_from_decode(x);
    xed_error = xed_encode(x, itext, ilen, &olen);

    if (xed_error != XED_ERROR_NONE) {
        logging("Error in encode : ENCODE ERROR: %s\n",
            xed_error_enum_t2str(xed_error));
        return FAILED;
    }

    c->ptr = itext;
    c->len = olen;

    return SUCCESS;
}

static instruction_t *alloc_instruction(private_disass_xed_t *this)
{
    xed_instruction_t *new_insn;

    if (!this)
        return NULL;

    if ((new_insn = malloc(sizeof(*new_insn))) == NULL)
    {
        logging("Error while allocating instruction in disassembler_xed\n");
        return NULL;
    }

    return (instruction_t*) new_insn;
}

static void *clone_instruction(void *instruction)
{
    xed_decoded_inst_t *new_instruction;

    if ((new_instruction = malloc(sizeof(xed_decoded_inst_t))) == NULL)
    {
        LOG_XED("Error while allocating instruction in clone_instruction from chain.c\n");
        return NULL;
    }

    memcpy(new_instruction, instruction, sizeof(xed_decoded_inst_t));

    return new_instruction;
}

static void destroy_instruction(void *instruction)
{
    free(instruction);
    instruction = NULL;
}

static uint64_t get_instruction_size(private_disass_xed_t *this)
{
    if (!this)
        return FAILED;

    return sizeof(xed_instruction_t);
}

static void destroy(private_disass_xed_t *this)
{
    free(this);
    this = NULL;
}

disass_xed_t *create_xed()
{
    private_disass_xed_t *this = malloc_thing(private_disass_xed_t);

    xed_format_options_t format_options;

    xed_tables_init();

    xed_set_verbosity( 99 );
    memset(&format_options,0, sizeof(format_options));
    format_options.hex_address_before_symbolic_name=0;
    format_options.xml_a=0;
    format_options.omit_unit_scale=0;
    format_options.no_sign_extend_signed_immediates=0;

    xed_format_set_options( format_options );

    /* Interface implementation */
    this->public.interface.initialize = (status_t (*)(disassembler_t*, chunk_t)) initialize;
    this->public.interface.get_category = (category_t (*)(disassembler_t*, instruction_t*)) get_category;
    this->public.interface.get_length = (uint64_t (*)(disassembler_t*, instruction_t*)) get_length;
    this->public.interface.format = (status_t (*)(disassembler_t*, instruction_t *, chunk_t)) format;
    this->public.interface.dump_intel = (status_t (*)(disassembler_t*, instruction_t *, chunk_t, uint64_t)) dump_intel;
    this->public.interface.decode = (status_t (*)(disassembler_t*, instruction_t **, chunk_t)) decode;
    this->public.interface.encode = (status_t (*)(disassembler_t*, chunk_t *, instruction_t *)) encode;
    this->public.interface.alloc_instruction = (instruction_t *(*)(disassembler_t*)) alloc_instruction;
    this->public.interface.clone_instruction = (void *(*)(void *)) clone_instruction;
    this->public.interface.destroy_instruction = (void (*)(void *)) destroy_instruction;
    this->public.interface.get_instruction_size = (uint64_t (*)(disassembler_t*)) get_instruction_size;
    this->public.interface.destroy = (void (*)(disassembler_t*)) destroy;

    return &this->public;
}
