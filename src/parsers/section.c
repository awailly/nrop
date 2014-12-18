/*
 * Code section
 */
#include "section.h"

typedef struct private_section_t private_section_t;

struct private_section_t
{
    section_t public;

    chunk_t header_chunk;
    chunk_t data_chunk;

    Elf64_Shdr (*get_formatted_header)(private_section_t *);
};

static Elf64_Shdr get_formatted_header(private_section_t *this)
{
    Elf64_Shdr shdr;

    memcpy(&shdr, this->header_chunk.ptr, sizeof(shdr));

    return shdr;
}

static uint32_t get_sh_name(private_section_t *this)
{
    return this->get_formatted_header(this).sh_name;
}

static uint32_t get_sh_type(private_section_t *this)
{
    return this->get_formatted_header(this).sh_type;
}

static uint64_t get_sh_size(private_section_t *this)
{
    return this->get_formatted_header(this).sh_size;
}

static Elf64_Off get_sh_offset(private_section_t *this)
{
    return this->get_formatted_header(this).sh_offset;
}

static void set_sh_offset(private_section_t *this, Elf64_Off offset)
{
    chunk_t new_header_chunk;
    Elf64_Shdr *shdr;

    new_header_chunk = chunk_clone(this->header_chunk);

    shdr = ((Elf64_Shdr*) new_header_chunk.ptr);
    shdr->sh_offset = offset;

    this->header_chunk = new_header_chunk;
}

static Elf64_Addr get_sh_addr(private_section_t *this)
{
    return this->get_formatted_header(this).sh_addr;
}

static void set_sh_addr(private_section_t *this, Elf64_Addr address)
{
    chunk_t new_header_chunk;
    Elf64_Shdr *shdr;

    new_header_chunk = chunk_clone(this->header_chunk);

    shdr = ((Elf64_Shdr*) new_header_chunk.ptr);
    shdr->sh_addr = address;

    this->header_chunk = new_header_chunk;
}

static uint64_t get_sh_addralign(private_section_t *this)
{
    return this->get_formatted_header(this).sh_addralign;
}

static uint64_t get_sh_flags(private_section_t *this)
{
    return this->get_formatted_header(this).sh_flags;
}

static uint64_t get_sh_entsize(private_section_t *this)
{
    return this->get_formatted_header(this).sh_entsize;
}

static void add_dynamic_info(private_section_t *this, chunk_t new_dynamic_infos)
{
    this->data_chunk = chunk_cat("mc", this->data_chunk, new_dynamic_infos);
}

static chunk_t get_dynamic_infos_by_tag(private_section_t *this, Elf64_Sxword tag)
{
    Elf64_Dyn *dyn;

    dyn = (Elf64_Dyn *) this->data_chunk.ptr;

    for(; dyn->d_tag; dyn++)
    {
        if (dyn->d_tag == tag)
            return chunk_create((u_char*) dyn, sizeof(Elf64_Dyn));
    }

    return chunk_empty;
}

static void set_tag_value(private_section_t *this, Elf64_Sxword tag, Elf64_Addr value)
{
    Elf64_Dyn *dyn;

    dyn = (Elf64_Dyn *) this->data_chunk.ptr;

    for(; dyn->d_tag; dyn++)
    {
        if (dyn->d_tag == tag)
        {
            dyn->d_un.d_ptr = value;
        }
    }
}

static void set_symtab_value(private_section_t *this, uint32_t st_name, Elf64_Addr st_value)
{
    Elf64_Sym *sym;

    sym = (Elf64_Sym *) this->data_chunk.ptr;

    for(; sym->st_name; sym++)
    {
        if (sym->st_name == st_name)
        {
            sym->st_value = st_value;
        }
    }
}

linked_list_t *create_symtab_linked_list(private_section_t *this)
{
    Elf64_Sym *sym;
    linked_list_t *symtab_list;
    unsigned int index;
    unsigned int symtab_item_number;
    uint64_t entsize;
    
    entsize = this->public.get_sh_entsize(&this->public);

    symtab_list = linked_list_create();

    sym = (Elf64_Sym *) this->data_chunk.ptr;
    symtab_item_number = this->public.get_sh_size(&this->public) / entsize;

    for(index = 0; index < symtab_item_number; index++, sym++)
    {
        chunk_t symtab_value;
        chunk_t *symtab_item;

        symtab_value = chunk_create((u_char*) sym, entsize);
        symtab_item = (chunk_t*) malloc(sizeof(chunk_t));
        memcpy(symtab_item, &symtab_value, sizeof(symtab_value));
        symtab_list->insert_last(symtab_list, symtab_item);
    }

    return symtab_list;
}

linked_list_t *create_rela_linked_list(private_section_t *this)
{
    Elf64_Rela *rela;
    linked_list_t *rela_list;
    unsigned int index;
    unsigned int item_number;
    uint64_t entsize;
    
    entsize = this->public.get_sh_entsize(&this->public);

    rela_list = linked_list_create();

    rela = (Elf64_Rela *) this->data_chunk.ptr;
    item_number = this->public.get_sh_size(&this->public) / entsize;

    for(index = 0; index < item_number; index++, rela++)
    {
        chunk_t rela_value;
        chunk_t *rela_item;

        rela_value = chunk_create((u_char*) rela, entsize);
        rela_item = (chunk_t*) malloc(sizeof(chunk_t));
        memcpy(rela_item, &rela_value, sizeof(rela_value));
        rela_list->insert_last(rela_list, rela_item);
    }
    
    return rela_list;
}

static status_t apply_got_offset_to_plt(private_section_t *this, section_t *got)
{
    Elf64_Off got_offset;
    chunk_t chunk;
    unsigned int current_got_offset;
    u_char ptr;

    got_offset = got->get_sh_addr(got);

    chunk = this->data_chunk;

    current_got_offset = got_offset + 2;
    logging("got offset: %"PRIx64"\n", got_offset);

    /**
     * parsing plt memory
     */
    for (ptr = 0; ptr < chunk.len; ptr++)
    {
        unsigned char *offset_location;

        offset_location = NULL;

        /**
         * We are only interested by \xff (for now with 64b and simple-echo example rip related)
         *
         * Addresses to jump are on 4 bytes
         */
        if ((unsigned char) *(chunk.ptr+ptr) == 0xff)
        {
            /**
             * push
             */
            if ((unsigned char) *(chunk.ptr+ptr+1) == 0x35)
            {
                offset_location = chunk.ptr + ptr + 2;
            }
            /**
             * jmp
             */
            else if ((unsigned char) *(chunk.ptr+ptr+1) == 0x25)
            {
                offset_location = chunk.ptr + ptr + 2;
            }
        }

        if (offset_location)
        {
            unsigned int current_rip;

            current_rip = this->public.get_sh_addr(&this->public) + ptr;

            /**
             * original_plt_rip + offset_a = orig_got_4
             * new_plt_rip + offset_b = new_got_4
             *
             * offset_b = new_got_4 - new_plt_rip
             */
            /*printf("patched: %08x\n", *(unsigned int *) offset_location);*/
            *(unsigned int *) offset_location = current_got_offset - current_rip;

            /*
            printf("patched: -> %08x = %08x + got: %08x\n", 
                    *(unsigned int *) offset_location, 
                    current_rip,
                    current_got_offset);
            */

            /**
             * Second and third item in got are reserved for dynamic linker.
             *
             * Followed by real entries of the got. First pass contains the address of the next
             * instruction of plt jmp.
             */
            if ((current_got_offset - got_offset) > 0x10)
            {
                unsigned int current_entry = (current_got_offset - got_offset)/8 + 1;
                unsigned int value = current_rip + 6;

                got->set_entry(got, current_entry, chunk_create((u_char*)&value, sizeof(value)));
            }

            current_got_offset+= 8;
            ptr+= 5;
        }
    }

    return SUCCESS;
}

static status_t set_entry(private_section_t *this, unsigned int entry, chunk_t value)
{
    memcpy(this->data_chunk.ptr + entry*8, value.ptr, value.len);

    return SUCCESS;
}

static chunk_t get_header(private_section_t *this)
{
    return chunk_clone(this->header_chunk);
}

static chunk_t get_chunk(private_section_t *this)
{
    return chunk_clone(this->data_chunk);
}

static chunk_t get_real_chunk(private_section_t *this)
{
    return this->data_chunk;
}

static void destroy(private_section_t *this)
{
    chunk_free(&this->header_chunk);
    chunk_free(&this->data_chunk);
    free(this);
    this = NULL;
}

section_t *create_section(chunk_t header, chunk_t data)
{
    private_section_t *this = malloc_thing(private_section_t);

    this->header_chunk = chunk_clone(header);
    this->data_chunk = chunk_clone(data);

    this->public.get_sh_name = (uint32_t (*)(section_t*)) get_sh_name;
    this->public.get_sh_type = (uint32_t (*)(section_t*)) get_sh_type;
    this->public.get_sh_size = (uint64_t (*)(section_t*)) get_sh_size;
    this->public.get_sh_offset = (Elf64_Off (*)(section_t*)) get_sh_offset;
    this->public.set_sh_offset = (void (*)(section_t*,Elf64_Off)) set_sh_offset;
    this->public.get_sh_addr = (Elf64_Addr (*)(section_t*)) get_sh_addr;
    this->public.set_sh_addr = (void (*)(section_t*,Elf64_Addr)) set_sh_addr;
    this->public.get_sh_addralign = (uint64_t (*)(section_t*)) get_sh_addralign;
    this->public.get_sh_flags = (uint64_t (*)(section_t*)) get_sh_flags;
    this->public.get_sh_entsize= (uint64_t (*)(section_t*)) get_sh_entsize;
    this->public.add_dynamic_info = (void (*)(section_t*,chunk_t)) add_dynamic_info;
    this->public.get_dynamic_infos_by_tag = (chunk_t (*)(section_t*,Elf64_Sxword)) get_dynamic_infos_by_tag;
    this->public.set_tag_value = (void (*)(section_t*,Elf64_Sxword,Elf64_Addr)) set_tag_value;
    this->public.set_symtab_value = (void (*)(section_t*,uint32_t,Elf64_Addr)) set_symtab_value;
    this->public.create_symtab_linked_list = (linked_list_t *(*)(section_t*)) create_symtab_linked_list;
    this->public.create_rela_linked_list = (linked_list_t *(*)(section_t*)) create_rela_linked_list;
    this->public.apply_got_offset_to_plt = (status_t (*)(section_t*,section_t*)) apply_got_offset_to_plt;
    this->public.set_entry = (status_t (*)(section_t*,unsigned int,chunk_t)) set_entry;
    this->public.get_header = (chunk_t (*)(section_t*)) get_header;
    this->public.get_chunk = (chunk_t (*)(section_t*)) get_chunk;
    this->public.get_real_chunk = (chunk_t (*)(section_t*)) get_real_chunk;
    this->public.destroy = (void (*)(section_t*)) destroy;
    this->get_formatted_header = (Elf64_Shdr (*)(private_section_t *)) get_formatted_header;

    return &this->public;
}

section_t *create_section_empty()
{
    return create_section(chunk_empty, chunk_empty);
}
