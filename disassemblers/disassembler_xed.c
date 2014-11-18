/*
 * Implementation of disassembler
 */

#include "disassembler_xed.h"

#define BUFLEN  1000

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
        printf("\t%s\n", format.ptr);
    else
        fprintf(stderr,"DISASSEMBLY ERROR\n");

    return SUCCESS;
}

static status_t decode(private_disass_xed_t *this, instruction_t *i, chunk_t c)
{
    xed_bool_t ok;
    char buffer[BUFLEN];
    xed_decoded_inst_t *xedd;
    xed_error_enum_t xed_error;

    if ((i = (instruction_t*) malloc_thing(xed_instruction_t)) == NULL)
        logging("[x] Error while allocating xed_instruction_t in disassembler_xed.c\n");

    xedd = &((xed_instruction_t*) i)->xedd;

    xed_decoded_inst_zero(xedd);
    xed_decoded_inst_set_mode(xedd, this->mmode, this->stack_addr_width);

    xed_error = xed_decode(xedd, c.ptr, c.len);

    switch(xed_error)
    {
      case XED_ERROR_NONE:
        xed_decoded_inst_dump(xedd, buffer, BUFLEN);
        printf("%s\n",buffer);
        break;
      case XED_ERROR_BUFFER_TOO_SHORT:
        fprintf(stderr,"Not enough bytes provided\n");
        break;
      case XED_ERROR_GENERAL_ERROR:
        fprintf(stderr,"Could not decode given input.\n");
        break;
      default:
        fprintf(stderr,"Unhandled error code %s\n",
                xed_error_enum_t2str(xed_error));
        break;
    }

    ok = xed_format(XED_SYNTAX_INTEL, xedd, buffer, BUFLEN, 0);

    if (ok)
        printf("\t%s\n", buffer);
    else
        fprintf(stderr,"DISASSEMBLY ERROR\n");

    return SUCCESS;
}

static status_t encode(private_disass_xed_t *this, chunk_t c, instruction_t *i)
{
    if (!this)
        return FAILED;

    if (!i)
        return FAILED;

    if (!c.ptr)
        return FAILED;

    return SUCCESS;
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
    this->public.interface.decode = (status_t (*)(disassembler_t*, instruction_t *, chunk_t)) decode;
    this->public.interface.encode = (status_t (*)(disassembler_t*, chunk_t, instruction_t *)) encode;
    this->public.interface.destroy = (void (*)(disassembler_t*)) destroy;

    return &this->public;
}
