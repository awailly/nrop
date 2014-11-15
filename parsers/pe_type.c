/*
 * Struct to store pe informations (private)
 */

#include "pe_type.h"

typedef struct private_pe_t private_pe_t;

struct private_pe_t
{
    pe_t public;

    chunk_t type;
    region_t *region;
    linked_list_t *section_list;
    linked_list_t *program_header_list;

    section_t *shstr_section;
    section_t *strtab_section;

    status_t (*check_region_32b)(region_t *);
    status_t (*check_region_64b)(region_t *);
    status_t (*check_region)(region_t *);

    /**
     * Create a chunk to correctly pad section s.
     *
     * @param           section to pad
     * @param           current offset in the file
     * @return          chunk to add before section chunk
     */
    chunk_t (*create_alignment_chunk)(section_t *s, uint64_t offset);

    /**
     * Correct offsets into _start function
     */
    status_t (*update_start_offsets)(private_pe_t *this, chunk_t start_function);
};

typedef struct xed_symbol_table_t xed_symbol_table_t;

struct xed_symbol_table_t
{
    int foo;
};

/*
 * Private functions
 */
static status_t check_binary_32b(void* start) {
    return start == 0;
}

static status_t check_binary_64b(void* start) {
    return start == 0;
}

static status_t check_region_32b(region_t *region)
{
    return check_binary_32b(region->get_chunk(region).ptr);
}

static status_t check_region_64b(region_t *region)
{
    return check_binary_64b(region->get_chunk(region).ptr);
}

static status_t check_region(region_t *region)
{
    if (check_binary_32b(region->get_chunk(region).ptr) || 
        check_binary_64b(region->get_chunk(region).ptr))
    {
        return SUCCESS;
    }
    else
    {
        return FAILED;
    }
}

static chunk_t create_alignment_chunk(section_t *s, uint64_t offset)
{
    chunk_t chunk_data;

    chunk_data = chunk_empty;

    /*
     * Normal PE should verify (sh_addr == 0 (sh_addralign))
     */
    if ((s->get_sh_addralign(s) > 1) && (s->get_sh_type(s) != SHT_NOBITS))
    {
        uint64_t align = s->get_sh_addralign(s);
        uint64_t padding = (align - (offset % align)) % align;
        unsigned int align_offset;

        for (align_offset = 0; align_offset < padding; align_offset++)
        {
            chunk_t align_chunk;
            u_char zero[] = { 0 };

            align_chunk = chunk_from_thing(zero);
            chunk_data = chunk_cat("mc", chunk_data, align_chunk);
        }
    }

    return chunk_data;
}


/*
 * Public functions
 */
static status_t add_section(private_pe_t *this, section_t *section)
{
    this->section_list->insert_last(this->section_list, section);
    return SUCCESS;
}

static bool compare_sections(void *a, void *b)
{
    return a == b;
}

static status_t remove_section(private_pe_t *this, section_t *section)
{
    this->section_list->remove(this->section_list, section, compare_sections);
    return SUCCESS;
}

static enumerator_t *get_section_enumerator(private_pe_t *this)
{
    return this->section_list->create_enumerator(this->section_list);
}

static linked_list_t *get_section_list(private_pe_t *this)
{
    return this->section_list;
}

static section_t *get_shstr_section(private_pe_t *this)
{
    return this->shstr_section;
}

static section_t *get_strtab_section(private_pe_t *this)
{
    return this->strtab_section;
}

static char *get_section_name(private_pe_t *this, section_t *section)
{
    section_t *section_shstrndx;
    Elf64_Shdr shdr;
    Elf64_Shdr shstrhdr;
    chunk_t section_memory;
    chunk_t shstr_memory;

    region_t *region;
    chunk_t chunk;

    region = ((code_t*) this)->get_region((code_t*) this);
    chunk = region->get_chunk(region);
    section_shstrndx = this->public.get_shstr_section(&this->public);

    section_memory = section->get_header(section);
    shstr_memory = section_shstrndx->get_header(section_shstrndx);

    memcpy(&shdr, section_memory.ptr, section_memory.len);
    memcpy(&shstrhdr, shstr_memory.ptr, shstr_memory.len);

    chunk_free(&section_memory);
    chunk_free(&shstr_memory);

    return (char*) chunk.ptr + shstrhdr.sh_offset + shdr.sh_name;
}

static section_t *get_section_by_name(private_pe_t *this, char *name)
{
    enumerator_t *e;
    section_t *s;

    e = this->public.get_section_enumerator(&this->public);

    while(e->enumerate(e, &s))
    {
        char *section_name;

        section_name = this->public.get_section_name(&this->public, s);

        if (!strcmp(section_name, name))
        {
            e->destroy(e);
            return s;
        }
    }

    e->destroy(e);

    return NULL;
}

static chunk_t get_section_data_chunk(private_pe_t *this, section_t *section)
{
    region_t *region;
    chunk_t data_chunk;
    chunk_t code_chunk;

    region = ((code_t*) this)->get_region((code_t*) this);
    code_chunk = region->get_chunk(region);
    data_chunk = chunk_create(code_chunk.ptr + section->get_sh_offset(section),  section->get_sh_size(section));

    return data_chunk;
}

static status_t add_program_header(private_pe_t *this, program_header_t *program_header)
{
    this->program_header_list->insert_last(this->program_header_list, program_header);
    return SUCCESS;
}

static bool compare_program_headers(void *a, void *b)
{
    return a == b;
}

static status_t remove_program_header(private_pe_t *this, program_header_t *program_header)
{
    this->program_header_list->remove(this->program_header_list, program_header, compare_program_headers);
    return SUCCESS;
}

static enumerator_t *get_program_header_enumerator(private_pe_t *this)
{
    return this->program_header_list->create_enumerator(this->program_header_list);
}

static linked_list_t *get_program_header_list(private_pe_t *this)
{
    return this->program_header_list;
}

static chunk_t get_program_header_data_chunk(private_pe_t *this, program_header_t *program_header)
{
    region_t *region;
    chunk_t data_chunk;
    chunk_t code_chunk;

    region = ((code_t*) this)->get_region((code_t*) this);
    code_chunk = region->get_chunk(region);
    data_chunk = chunk_create(code_chunk.ptr + program_header->get_p_offset(program_header), program_header->get_p_filesz(program_header));

    return data_chunk;
}

/**
 * Interface functions
 */
static chunk_t get_type(private_pe_t* this)
{
    return this->type;
}   

static region_t *get_region(private_pe_t* this)
{   
    return this->region;
}

static void destroy(private_pe_t *this)
{
    enumerator_t *e;
    section_t *s;
    program_header_t *p;

    e = this->section_list->create_enumerator(this->section_list);

    while (e->enumerate(e, &s))
    {
        s->destroy(s);
    }

    e->destroy(e);

    e = this->program_header_list->create_enumerator(this->program_header_list);

    while (e->enumerate(e, &p))
    {
        p->destroy(p);
    }

    e->destroy(e);

    this->shstr_section->destroy(this->shstr_section);
    this->strtab_section->destroy(this->strtab_section);
    this->section_list->destroy(this->section_list);
    this->program_header_list->destroy(this->program_header_list);
    chunk_free(&this->type);
    free(this);
    this = NULL;
}

pe_t *create_pe(chunk_t type, region_t *region)
{
    private_pe_t *this = malloc_thing(private_pe_t);

    this->section_list = linked_list_create();
    this->program_header_list = linked_list_create();
    this->type = chunk_clone(type);
    this->region = region;

    /* Private functions */
    this->check_region_32b = (status_t (*)(region_t *)) check_region_32b;
    this->check_region_64b = (status_t (*)(region_t *)) check_region_64b;
    this->check_region = (status_t (*)(region_t *)) check_region;
    this->create_alignment_chunk = (chunk_t (*)(section_t*,uint64_t)) create_alignment_chunk;

    /* Public functions */
    this->public.add_section = (status_t (*)(pe_t*, section_t*)) add_section;
    this->public.remove_section = (status_t (*)(pe_t*, section_t*)) remove_section;
    this->public.get_section_list = (linked_list_t *(*)(pe_t*)) get_section_list;
    this->public.get_section_enumerator = (enumerator_t *(*)(pe_t*)) get_section_enumerator;
    this->public.get_shstr_section = (section_t *(*)(pe_t*)) get_shstr_section;
    this->public.get_strtab_section = (section_t *(*)(pe_t*)) get_strtab_section;
    this->public.get_section_name = (char *(*)(pe_t*, section_t*)) get_section_name;
    this->public.get_section_by_name = (section_t *(*)(pe_t*, char*)) get_section_by_name;
    this->public.get_section_data_chunk = (chunk_t (*)(pe_t*, section_t*)) get_section_data_chunk;
    this->public.add_program_header = (status_t (*)(pe_t*, program_header_t*)) add_program_header;
    this->public.remove_program_header = (status_t (*)(pe_t*, program_header_t*)) remove_program_header;
    this->public.get_program_header_list = (linked_list_t *(*)(pe_t*)) get_program_header_list;
    this->public.get_program_header_enumerator = (enumerator_t *(*)(pe_t*)) get_program_header_enumerator;
    this->public.get_program_header_data_chunk = (chunk_t (*)(pe_t*, program_header_t*)) get_program_header_data_chunk;
    this->public.destroy = (void (*)(pe_t*)) destroy;

    /* Interface implementation */
    this->public.interface.get_type = (chunk_t (*)(code_t*)) get_type;
    this->public.interface.get_region = (region_t *(*)(code_t*)) get_region;
    this->public.interface.destroy = (void (*)(code_t*)) destroy;

    /* Feeding local object */

    return &this->public;
}
