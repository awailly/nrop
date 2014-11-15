/*
 * Implementation of junk insertion plugin
 */

#include "plugin_junk.h"

#define BUFLEN  1000

typedef struct private_plugin_junk_t private_plugin_junk_t;

struct private_plugin_junk_t
{
    plugin_junk_t public;
    elf_t *code;

    status_t (*disassemble)(private_plugin_junk_t *, chunk_t);
    bool (*is_last_inst)(private_plugin_junk_t *, xed_decoded_inst_t);
    status_t (*pack)(private_plugin_junk_t *, section_t *);
    linked_list_t *(*find_rop_chains)(private_plugin_junk_t *, chunk_t, Elf64_Addr);
    status_t (*reverse_disass_ret)(private_plugin_junk_t *, chunk_t, Elf64_Addr, xed_decoded_inst_t, uint64_t, linked_list_t*);
};

static status_t disassemble(private_plugin_junk_t *this, chunk_t function_chunk)
{
    xed_error_enum_t xed_error;
    xed_bool_t long_mode = 1;
    xed_decoded_inst_t xedd;
    xed_format_options_t format_options;
    xed_machine_mode_enum_t mmode;
    xed_address_width_enum_t stack_addr_width;

#define BUFLEN  1000
    char buffer[BUFLEN];
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    unsigned int i;

    // one time initialization 
    xed_tables_init();
    xed_set_verbosity( 99 );
    memset(&format_options,0, sizeof(format_options));
    format_options.hex_address_before_symbolic_name=0;
    format_options.xml_a=0;
    format_options.omit_unit_scale=0;
    format_options.no_sign_extend_signed_immediates=0;

    xed_format_set_options( format_options );

    if (long_mode) {
        mmode=XED_MACHINE_MODE_LONG_64;
        stack_addr_width =XED_ADDRESS_WIDTH_64b;
    }
    else {
        mmode=XED_MACHINE_MODE_LEGACY_32;
        stack_addr_width =XED_ADDRESS_WIDTH_32b;
    }

    if (!this)
    {
        return FAILED;
    }

    hexdump(function_chunk.ptr, function_chunk.len);

    for (i=0; i<function_chunk.len; )
    {
        xed_decoded_inst_zero(&xedd);
        xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);
        xed_bool_t ok;
        unsigned int len;

        memcpy(itext, function_chunk.ptr + i, 15);
        bytes = 15;

        /*hexdump(&itext, 15);*/
        xed_error = xed_decode(&xedd, itext, bytes);

        switch(xed_error)
        {
          case XED_ERROR_NONE:
            xed_decoded_inst_dump(&xedd,buffer, BUFLEN);
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

        ok = xed_format(XED_SYNTAX_INTEL, &xedd, buffer, BUFLEN, 0);

        if (ok)
            printf("\t%s\n", buffer);
        else
            fprintf(stderr,"DISASSEMBLY ERROR\n");

        len = xed_decoded_inst_get_length(&xedd);
        i+= len;
    }

    return SUCCESS;
}

static bool is_last_inst(private_plugin_junk_t *this, xed_decoded_inst_t xedd)
{
    return (
            (xed_decoded_inst_get_category(&xedd) == XED_CATEGORY_COND_BR) ||
            (xed_decoded_inst_get_category(&xedd) == XED_CATEGORY_UNCOND_BR) ||
            (xed_decoded_inst_get_category(&xedd) == XED_CATEGORY_SYSCALL) ||
            (xed_decoded_inst_get_category(&xedd) == XED_CATEGORY_CALL) ||
            (xed_decoded_inst_get_category(&xedd) == XED_CATEGORY_RET)
           );
    (void)this;
}

static status_t reverse_disass_ret(private_plugin_junk_t *this, chunk_t chunk, Elf64_Addr addr, xed_decoded_inst_t xedd, uint64_t ret_byte, linked_list_t *inst_list)
{
    xed_error_enum_t xed_error;

    char buffer[BUFLEN];
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    uint64_t current_byte;
    uint64_t last_decoded_byte;

    char *chain_str;
    uint32_t chain_size;
    chain_t *chain;
    bool byte_to_disass;
    uint64_t offset_reccursive;

    chain_str = NULL;
    chain_size = 0;
    current_byte = ret_byte - 1;
    last_decoded_byte = 0;
    byte_to_disass = true;
    offset_reccursive = 0;

    if (!this)
    {
        return FAILED;
    }
    
    /*hexdump(chunk.ptr + ret_byte - 19, 20);*/

    while (byte_to_disass)
    {
        if (current_byte == 0)
        {
            /*logging("start of chunk reached\n");*/
            byte_to_disass = false;
        }
        else if ((last_decoded_byte > 0) && ((ret_byte - current_byte > 500)))
        {
            /*logging("max decoding size reached\n");*/
            byte_to_disass = false;
        }
        else if ((last_decoded_byte > 0) && ((last_decoded_byte - current_byte) > 15))
        {
            char *new_chain_str;

            /*logging("no instruction found within maximum instruction length range\n");*/
            offset_reccursive++;
            current_byte = last_decoded_byte;

            if (last_decoded_byte == ret_byte - 1)
            {
                byte_to_disass = false;
            }
            else
            {
                xed_decoded_inst_zero_keep_mode(&xedd);
                memcpy(itext, chunk.ptr + current_byte, 15);
                bytes = 15;

                /* No error as we already tested it */
                xed_decode(&xedd, itext, bytes);

                /* Recovering offsets and chain string */
                last_decoded_byte+= xed_decoded_inst_get_length(&xedd);
                current_byte-= offset_reccursive;

                xed_format(XED_SYNTAX_INTEL, &xedd, buffer, BUFLEN, 0);

                if ((new_chain_str = malloc(chain_size)) == NULL)
                {
                    logging("Error while allocating new_chain_str in reverse_disass_ret\n");
                    free(chain_str);
                    return FAILED;
                }

                /*
                logging("D: %s\n", chain_str);
                logging("D: %08x :: %08x\n", last_decoded_byte, ret_byte);
                */

                memcpy(new_chain_str, chain_str + strlen(buffer) + 3, strlen(chain_str) - strlen(buffer) - 3 + 1);

                free(chain_str);

                chain_str = new_chain_str;
                chain_size = strlen(new_chain_str);
            }
        }

        xed_decoded_inst_zero_keep_mode(&xedd);

        memcpy(itext, chunk.ptr + current_byte, 15);
        bytes = 15;

        xed_error = xed_decode(&xedd, itext, bytes);

        if (xed_error == XED_ERROR_NONE)
        {
            xed_bool_t ok;

            ok = xed_format(XED_SYNTAX_INTEL, &xedd, buffer, BUFLEN, 0);

            if (!ok)
            {
                logging("error while using xed_format in reverse_disass_ret\n");
            }
            else if ((last_decoded_byte > 0) && (xed_decoded_inst_get_length(&xedd) != (last_decoded_byte - current_byte)))
            {
                /*logging("instruction length is not valid\n");*/
            }
            else if ((this->is_last_inst(this, xedd)) && (chain_size > 0))
            {
                /*logging("found ending instruction\n");*/
            }
            else
            {
                char *new_chain_str;

                if ((new_chain_str = malloc(chain_size + strlen(buffer) + 4)) == NULL)
                {
                    logging("Error while allocating new_chain_str in reverse_disass_ret\n");
                    free(chain_str);
                    return FAILED;
                }

                if (chain_str == NULL)
                    sprintf(new_chain_str, "%s;", buffer);
                else
                    sprintf(new_chain_str, "%s ; %s", buffer, chain_str);

                chain_size = strlen(new_chain_str);

                last_decoded_byte = current_byte;
                /*current_byte-= xed_decoded_inst_get_length(&xedd);*/

                free(chain_str);

                chain_str = new_chain_str;
                chain = chain_create(addr + last_decoded_byte, chain_str, chunk_empty);
                inst_list->insert_last(inst_list, chain);

                offset_reccursive = 0;
            }
        }
        else
        {
            /*logging("wrong decoding\n");*/
        }

        current_byte--;
    }

    /*logging("%08x: %s\n", addr + last_decoded_byte, chain_str);*/

    free(chain_str);

    return SUCCESS;
}

static linked_list_t* find_rop_chains(private_plugin_junk_t *this, chunk_t function_chunk, Elf64_Addr addr)
{
    xed_error_enum_t xed_error;
    xed_bool_t long_mode = 1;
    xed_decoded_inst_t xedd;
    xed_format_options_t format_options;
    xed_machine_mode_enum_t mmode;
    xed_address_width_enum_t stack_addr_width;

    /*char buffer[BUFLEN];*/
    unsigned int bytes = 0;
    unsigned char itext[XED_MAX_INSTRUCTION_BYTES];
    uint64_t byte;
    linked_list_t *inst_list;

    // one time initialization 
    xed_tables_init();
    xed_set_verbosity( 99 );
    memset(&format_options,0, sizeof(format_options));
    format_options.hex_address_before_symbolic_name=0;
    format_options.xml_a=0;
    format_options.omit_unit_scale=0;
    format_options.no_sign_extend_signed_immediates=0;

    xed_format_set_options( format_options );

    inst_list = linked_list_create();

    if (long_mode) {
        mmode=XED_MACHINE_MODE_LONG_64;
        stack_addr_width =XED_ADDRESS_WIDTH_64b;
    }
    else {
        mmode=XED_MACHINE_MODE_LEGACY_32;
        stack_addr_width =XED_ADDRESS_WIDTH_32b;
    }

    if (!this)
    {
        return inst_list;
    }

    for (byte = 0; byte < function_chunk.len; byte++)
    {
        xed_decoded_inst_zero(&xedd);
        xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);

        memcpy(itext, function_chunk.ptr + byte, 15);
        bytes = 15;

        xed_error = xed_decode(&xedd, itext, bytes);

        if (xed_error == XED_ERROR_NONE)
        {
            if (this->is_last_inst(this, xedd))
            {
                /*
                logging("Found a RET ins\n");
                xed_decoded_inst_dump(&xedd,buffer, BUFLEN);
                printf("%s\n",buffer);
                */
                this->reverse_disass_ret(this, function_chunk, addr, xedd, byte+1, inst_list);
            }
        }
    }

    return inst_list;
}

static status_t pack(private_plugin_junk_t *this, section_t *section)
{
    chunk_t chunk;
    Elf64_Addr addr;
    linked_list_t *inst_list;

    enumerator_t *e;
    chain_t *c;

    if ((!this) || (!this->code))
    {
        return FAILED;
    }

    /*
    chunk = this->code->get_function_chunk(this->code, "main");
    this->disassemble(this, function_chunk);
    */
    chunk = this->code->get_section_data_chunk(this->code, section);
    addr = section->get_sh_addr(section);

    inst_list = this->find_rop_chains(this, chunk, addr);

    logging("Sorting %i elements...\n", inst_list->get_count(inst_list));

    /*inst_list->bsort(inst_list);*/

    e = inst_list->create_enumerator(inst_list);

    while(e->enumerate(e, &c))
    {
        /*logging("%08x: %s\n", c->get_addr(c), c->get_str(c));*/
        
        c->destroy(c);
    }

    e->destroy(e);

    inst_list->destroy(inst_list);

    return SUCCESS;
}

static status_t apply(private_plugin_junk_t *this)
{
    section_t *text_section;

    if ((text_section = this->code->get_section_by_name(this->code, ".text")) == NULL)
    {
        logging("Section %s not found.\n", ".text");
        return FAILED;
    }

    printf("Size of .text: %"PRIx64"\n", text_section->get_sh_size(text_section));

    return this->pack(this, text_section);
}

static void destroy(private_plugin_junk_t *this)
{
    free(this);
    this = NULL;
}

plugin_junk_t *plugin_junk_create(code_t *code)
{
    private_plugin_junk_t *this = malloc_thing(private_plugin_junk_t);

    this->code = (elf_t *) code;

    this->public.interface.apply = (status_t (*)(plugin_t *)) apply;
    this->public.interface.destroy = (void (*)(plugin_t *)) destroy;

    this->disassemble = (status_t (*)(private_plugin_junk_t *, chunk_t)) disassemble;
    this->is_last_inst = (bool (*)(private_plugin_junk_t *, xed_decoded_inst_t)) is_last_inst;
    this->pack = (status_t (*)(private_plugin_junk_t *, section_t *)) pack;
    this->find_rop_chains = (linked_list_t *(*)(private_plugin_junk_t *, chunk_t, Elf64_Addr)) find_rop_chains;
    this->reverse_disass_ret = (status_t (*)(private_plugin_junk_t *, chunk_t, Elf64_Addr, xed_decoded_inst_t, uint64_t, linked_list_t *)) reverse_disass_ret;

    return &this->public;
}
