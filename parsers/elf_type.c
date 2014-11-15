/*
 * Struct to store elf informations (private)
 */

#include "elf_type.h"

typedef struct private_elf_t private_elf_t;

struct private_elf_t
{
    elf_t public;

    chunk_t type;
    region_t *region;
    linked_list_t *section_list;
    linked_list_t *program_header_list;

    Elf32_Ehdr *elf32_ehdr;
    Elf64_Ehdr *elf64_ehdr;
    Elf32_Phdr *elf32_phdr;
    Elf64_Phdr *elf64_phdr;
    Elf32_Section *elf32_scn;
    Elf64_Section *elf64_scn;
    Elf32_Shdr *elf32_shdr;
    Elf64_Shdr *elf64_shdr;

    Elf32_Off strtab32_off;
    Elf32_Off dynstr32_off;
    Elf64_Off strtab64_off;
    Elf64_Off dynstr64_off;
    section_t *shstr_section;
    section_t *strtab_section;

    status_t (*check_region_32b)(region_t *);
    status_t (*check_region_64b)(region_t *);
    status_t (*check_region)(region_t *);
    status_t (*symbols_elf64)(private_elf_t *);
    status_t (*map_sections)(private_elf_t *);
    status_t (*map_program_headers)(private_elf_t *);
    void (*update_symbols_generic)(private_elf_t *, section_t *, Elf64_Off, char *, size_t);

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
    status_t (*update_start_offsets)(private_elf_t *this, chunk_t start_function);
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
    Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*) start;
    if (elf_hdr->e_machine == EM_386) 
        return 1;
    return 0;
}

static status_t check_binary_64b(void* start) {
#if !defined(EM_L1OM) /* Oh, not zero */
# define EM_L1OM  180
#endif
    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*) start;
    if (elf_hdr->e_machine == EM_X86_64 ||
            elf_hdr->e_machine == EM_L1OM) 
        return 1;
    return 0;
}

static status_t check_region_32b(region_t *region)
{
    return check_binary_32b(region->get_chunk(region).ptr);
}

static status_t check_region_64b(region_t *region)
{
    return check_binary_64b(region->get_chunk(region).ptr);
}

static char* lookup32(Elf32_Word stoffset, void* start, Elf32_Off offset)
{
    char* p = (char*)start + offset;
    char* q = p + stoffset;
    return q;
}

static char* lookup64(Elf64_Word stoffset, void* start, Elf64_Off offset)
{
  char* p = (char*)start + offset;
  char* q = p + stoffset;
  return q;
}

void read_symbols64(void* start, 
                    Elf64_Off offset,
                    Elf64_Word size,
                    Elf64_Off string_table_offset,
                    xed_symbol_table_t symtab) {
    char* a = (char*) start;
    Elf64_Sym* p = (Elf64_Sym*) (a + offset);
    Elf64_Sym* q = (Elf64_Sym*) (a + offset + size);
    while(p<q) {
        if (ELF64_ST_TYPE(p->st_info) == STT_FUNC) {
            char* name = lookup64(p->st_name, start, string_table_offset);
            if (xed_strlen(name) > 0) {
                /*
                 * symtab.add_local_symbol((xed_uint64_t)(p->st_value), 
                                        name, p->st_shndx);
                                        */
                printf("lala %p\n", (void*) &symtab);
            }
        }
        p++; 
    }
}

static status_t symbols_elf64(private_elf_t *this)
{
    region_t *region = ((code_t*) this)->get_region((code_t*) this);
    chunk_t chunk = region->get_chunk(region);
    void *start = chunk.ptr;

    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*) start;
    if (this->check_region_64b(region)==0) {
        printf("Not an x86-64  binary. Consider not using the -64 switch.");
        return FAILED;
    }

    Elf64_Off shoff = elf_hdr->e_shoff;  // section hdr table offset
    Elf64_Shdr* shp = (Elf64_Shdr*) ((char*)start + shoff);
    Elf64_Half nsect = elf_hdr->e_shnum;
    unsigned int i;
    Elf64_Half sect_strings  = elf_hdr->e_shstrndx;
    Elf64_Half sect_strtab= 0;
    Elf64_Off string_table_offset=0;
    Elf64_Off dynamic_string_table_offset=0;

    /* find the string_table_offset and the dynamic_string_table_offset */
    for( i=0;i<nsect;i++)  {
        if (shp[i].sh_type == SHT_STRTAB) {
            char* name = lookup32(shp[i].sh_name, start, 
                                  shp[sect_strings].sh_offset);
            if (strcmp(name,".strtab")==0) {
                sect_strtab = i;
                string_table_offset = shp[i].sh_offset;
            }
            if (strcmp(name,".dynstr")==0) {
                dynamic_string_table_offset = shp[i].sh_offset;
            }
        }
    }

    /* now read the symbols */
    /*
    for( i=0;i<nsect;i++)  {
        if (shp[i].sh_type == SHT_SYMTAB) {
            read_symbols64(start,shp[i].sh_offset, shp[i].sh_size, 
                           string_table_offset,symtab);
        }
        else if (shp[i].sh_type == SHT_DYNSYM) {
            read_symbols64(start,shp[i].sh_offset, shp[i].sh_size,
                           dynamic_string_table_offset, symtab);
        }
    }
    */

    this->elf64_ehdr = elf_hdr;
    this->elf64_phdr = (Elf64_Phdr*) ((char*) start + elf_hdr->e_phoff);
    this->elf64_shdr = shp;
    this->strtab64_off = string_table_offset;
    this->dynstr64_off = dynamic_string_table_offset;

    if (sect_strings == 0)
    {
        logging("Beware, shstr index is 0\n");
        this->shstr_section = NULL;
    }
    else
        this->shstr_section = create_section(
                                chunk_create(
                                    (u_char*) &shp[sect_strings],
                                    sizeof(Elf64_Shdr)),
                                chunk_create(
                                    ((Elf64_Shdr*) &shp[sect_strings])->sh_offset + (u_char*) start,
                                    ((Elf64_Shdr*) &shp[sect_strings])->sh_size));

    if (sect_strtab == 0)
    {
        logging("Beware, strtab index is 0\n");
        this->strtab_section = NULL;
    }
    else
        this->strtab_section = create_section(
                                chunk_create(
                                    (u_char*) &shp[sect_strtab],
                                    sizeof(Elf64_Shdr)),
                                chunk_create(
                                    ((Elf64_Shdr*) &shp[sect_strtab])->sh_offset + (u_char*) start,
                                    ((Elf64_Shdr*) &shp[sect_strtab])->sh_size));

    return SUCCESS;
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

static status_t map_sections(private_elf_t *this)
{
    region_t *region;
    chunk_t chunk;
    void *start;
    unsigned int i;

    region = ((code_t*) this)->get_region((code_t*) this);
    chunk = region->get_chunk(region);
    start = chunk.ptr;

    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*) start;
    if (this->check_region_64b(region)==0)
    {
        printf("Not an x86-64  binary. Consider not using the -64 switch.");
        return FAILED;
    }

    Elf64_Off shoff = elf_hdr->e_shoff;  // section hdr table offset
    Elf64_Shdr* shp = (Elf64_Shdr*) ((char*)start + shoff);
    Elf64_Half nsect = elf_hdr->e_shnum;

    for( i=0;i<nsect;i++)
    {
        chunk_t header;
        chunk_t data;
        section_t *current_section;
        uint64_t data_size;

        header = chunk_create((u_char*) &shp[i], sizeof(shp[i]));

        /*
         * Handling .bss NOBITS, with sh_size defined but not really
         * occupied into the program file.
         */
        if (((Elf32_Shdr*) &shp[i])->sh_type == SHT_NOBITS)
        {
            data_size = 0;
        }
        else
        {
            data_size = ((Elf64_Shdr*) &shp[i])->sh_size;
        }

        data = chunk_create(
                    ((Elf64_Shdr*) &shp[i])->sh_offset + (u_char*) start,
                    data_size);
        current_section = create_section(header, data);
        this->public.add_section(&this->public, current_section);
    }

    return SUCCESS;
}

static status_t map_program_headers(private_elf_t *this)
{
    region_t *region;
    chunk_t chunk;
    void *start;
    unsigned int i;

    region = ((code_t*) this)->get_region((code_t*) this);
    chunk = region->get_chunk(region);
    start = chunk.ptr;

    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*) start;
    if (this->check_region_64b(region)==0)
    {
        printf("Not an x86-64  binary. Consider not using the -64 switch.");
        return FAILED;
    }

    Elf64_Off phoff = elf_hdr->e_phoff;  // section hdr table offset
    Elf64_Phdr* php = (Elf64_Phdr*) ((char*)start + phoff);
    Elf64_Half nsect = elf_hdr->e_phnum;

    for( i=0;i<nsect;i++)
    {
        chunk_t header;
        chunk_t data;
        uint64_t data_size;

        header = chunk_create((u_char*) &php[i], sizeof(php[i]));
        data_size = ((Elf64_Phdr*) &php[i])->p_memsz;
        data = chunk_create(
                ((Elf64_Phdr*) &php[i])->p_offset + (u_char*) start,
                data_size);
        program_header_t *current_program_header = create_program_header(header, data);
        /**
         * Debug alignment, not taken in account yet
         *
        printf("pvaddr == poffset %% palign | %"PRIx64" == %"PRIx64" %% %"PRIx64"\n",
                current_program_header->get_p_vaddr(current_program_header),
                current_program_header->get_p_offset(current_program_header),
                current_program_header->get_p_align(current_program_header));
        */
        this->public.add_program_header(&this->public, current_program_header);
    }

    return SUCCESS;
}

/**
 * FIXME
 * This function can factorize update_symbols
 *
 * - Two flows into enumerate depending on section_name
 * - Casting parameter + value
 **/
static void update_symbols_generic (private_elf_t *this, section_t *section, Elf64_Off offset, char *section_name, size_t sym_len)
{
    uint32_t section_size;
    Elf64_Off section_start;
    section_t *section_to_update;
    linked_list_t *values_list;
    enumerator_t *e;
    chunk_t *c;

    section_to_update = this->public.get_section_by_name(&this->public, section_name);
    values_list = section_to_update->create_symtab_linked_list(section_to_update);
    section_size = section->get_sh_size(section);
    section_start = section->get_sh_addr(section);

    e = values_list->create_enumerator(values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Sym *sym;
        sym=(void*)sym_len; /* fix compilo (del me)*/

        sym = (Elf64_Sym*) c->ptr;

        /**
         * Hotfix for 0 values
         */
        if ((sym->st_value >= section_start) && (sym->st_value < (section_start + section_size)) && (sym->st_value))
        {
            sym->st_value+= offset - section->get_sh_offset(section);
        }
    }

    e->destroy(e);

    values_list->destroy(values_list);
}

static status_t update_start_offsets(private_elf_t *this, chunk_t start_function)
{
    chunk_t chunk;
    uint64_t current_offset;

    chunk = start_function;
    current_offset = 0;
    u_char ptr;

    /**
     * start function
     */
    for (ptr = 0; ptr < chunk.len; ptr++)
    {
        unsigned char *offset_location;

        offset_location = NULL;

        /**
         * We are only interested by \x48\xc7 and \x49\xc7 (always related to simple-echo)
         */
        if (((unsigned char) *(chunk.ptr+ptr) == 0x48) || ((unsigned char) *(chunk.ptr+ptr) == 0x49))
        {
            /**
             * Second offset is \xc7, third is register (we dont mind), then come 4o offsets
             */
            if ((unsigned char) *(chunk.ptr+ptr+1) == 0xc7)
            {
                offset_location = chunk.ptr + ptr + 3;
            }
        }

        if (offset_location)
        {
            if (current_offset == 0)
            {
                *(unsigned int *) offset_location = this->public.get_function_offset(&this->public, "__libc_csu_fini");
            }
            else if (current_offset == 1)
            {
                *(unsigned int *) offset_location = this->public.get_function_offset(&this->public, "__libc_csu_init");
            }
            else if (current_offset == 2)
            {
                *(unsigned int *) offset_location = this->public.get_function_offset(&this->public, "main");
            }

            current_offset++;
            ptr+= 4;
        }
    }

    return SUCCESS;
}

static chunk_t create_alignment_chunk(section_t *s, uint64_t offset)
{
    chunk_t chunk_data;

    chunk_data = chunk_empty;

    /*
     * Normal ELF should verify (sh_addr == 0 (sh_addralign))
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
static status_t add_section(private_elf_t *this, section_t *section)
{
    this->section_list->insert_last(this->section_list, section);
    return SUCCESS;
}

static bool compare_sections(void *a, void *b)
{
    return a == b;
}

static status_t remove_section(private_elf_t *this, section_t *section)
{
    this->section_list->remove(this->section_list, section, compare_sections);
    return SUCCESS;
}

static enumerator_t *get_section_enumerator(private_elf_t *this)
{
    return this->section_list->create_enumerator(this->section_list);
}

static linked_list_t *get_section_list(private_elf_t *this)
{
    return this->section_list;
}

static section_t *get_shstr_section(private_elf_t *this)
{
    return this->shstr_section;
}

static section_t *get_strtab_section(private_elf_t *this)
{
    return this->strtab_section;
}

static char *get_section_name(private_elf_t *this, section_t *section)
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

static section_t *get_section_by_name(private_elf_t *this, char *name)
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

static Elf64_Sxword get_section_tag(private_elf_t *this, section_t *section)
{
    char *section_name;
    
    section_name = this->public.get_section_name(&this->public, section);

    /**
     * d_ptr
     */
    if (!(strcmp(section_name, ".init")))
        return DT_INIT;
    else if (!(strcmp(section_name, ".fini")))
        return DT_FINI;
    else if (!(strcmp(section_name, ".hash")))
        return DT_HASH;
    else if (!(strcmp(section_name, ".gnu.hash")))
        return DT_GNU_HASH;
    else if (!(strcmp(section_name, ".dynstr")))
        return DT_STRTAB;
    else if (!(strcmp(section_name, ".dynsym")))
        return DT_SYMTAB;
    else if (!(strcmp(section_name, ".got.plt")))
        return DT_PLTGOT;
    else if (!(strcmp(section_name, ".rela.plt")))
        return DT_JMPREL;
    else if (!(strcmp(section_name, ".rela.dyn")))
        return DT_RELA;
    else if (!(strcmp(section_name, ".gnu.version_r")))
        return DT_VERNEED;
    else if (!(strcmp(section_name, ".gnu.version")))
        return DT_VERSYM;
    /**
     * d_val
     */
    else
        return 0;
}

static bool is_section_tag_d_ptr(elf_t *this, Elf64_Sxword tag)
{
    uint32_t tag_ptr;

    if (!this) return 0;

    tag_ptr = DT_INIT | DT_FINI | DT_HASH | DT_GNU_HASH | DT_STRTAB | DT_SYMTAB | DT_PLTGOT | DT_JMPREL | DT_RELA | DT_VERNEED | DT_VERSYM;

    return (tag & tag_ptr);
}

static chunk_t get_section_data_chunk(private_elf_t *this, section_t *section)
{
    region_t *region;
    chunk_t data_chunk;
    chunk_t code_chunk;

    region = ((code_t*) this)->get_region((code_t*) this);
    code_chunk = region->get_chunk(region);
    data_chunk = chunk_create(code_chunk.ptr + section->get_sh_offset(section),  section->get_sh_size(section));

    return data_chunk;
}

static Elf64_Addr get_function_offset(private_elf_t *this, char *function_name)
{
    section_t *symtab_section;
    section_t *strtab_section;
    linked_list_t *symtab_values_list;
    enumerator_t *e;
    chunk_t *c;
    Elf64_Addr function_offset;

    function_offset = 0;

    /**
     * Looking for function name offset in .strtab
     */
    symtab_section = this->public.get_section_by_name(&this->public, ".symtab");
    strtab_section = this->public.get_strtab_section(&this->public);
    symtab_values_list = symtab_section->create_symtab_linked_list(symtab_section);

    e = symtab_values_list->create_enumerator(symtab_values_list);

    while (e->enumerate(e, &c) && (function_offset == 0))
    {
        Elf64_Sym *sym;

        sym = (Elf64_Sym*) c->ptr;

        if (!(strcmp((char*) strtab_section->get_chunk(strtab_section).ptr + sym->st_name, function_name)))
        {
            function_offset = sym->st_value;
        }
    }

    e->destroy(e);

    symtab_values_list->destroy(symtab_values_list);

    if (function_offset == 0)
    {
        logging("get_function_chunk(): Unable to find function_offset\n");
        return 0;
    }

    return function_offset;
}

static chunk_t get_function_chunk(private_elf_t *this, char *function_name)
{
    section_t *symtab_section;
    linked_list_t *symtab_values_list;
    enumerator_t *e;
    chunk_t *c;
    section_t *s;
    Elf64_Addr function_offset;
    /* Section containing function function_name */
    section_t *function_section;
    /* Difference with next symbol, used to find function size */
    size_t function_size;
    Elf64_Off function_section_start;
    Elf64_Off function_section_end;

    function_offset = this->public.get_function_offset(&this->public, function_name);
    logging("offset: %"PRIx64"\n", function_offset);
    function_section = NULL;
    function_size = 0;

    if ((symtab_section = this->public.get_section_by_name(&this->public, ".symtab")) == NULL)
    {
        logging("Error while getting symtab_section in get_function_chunk\n");
        return chunk_empty;
    }

    symtab_values_list = symtab_section->create_symtab_linked_list(symtab_section);

    /**
     * Find section corresponding to function offset
     */
    e = this->public.get_section_enumerator(&this->public);

    while ((function_section == NULL) && (e->enumerate(e, &s)))
    {
        function_section_start = s->get_sh_addr(s);
        function_section_end = function_section_start + s->get_sh_size(s);

        if ((function_offset >= function_section_start) && (function_offset < function_section_end))
        {
            function_section = s;
        }
    }

    e->destroy(e);

    if (function_section == NULL)
    {
        logging("get_function_chunk(): Unable to find function_section\n");
        return chunk_empty;
    }
    
    /**
     * Find function size, either by next symbols or section end.
     */
    //symtab_values_list = symtab_section->create_symtab_linked_list(symtab_section);

    e = symtab_values_list->create_enumerator(symtab_values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Sym *sym;

        sym = (Elf64_Sym*) c->ptr;

        /**
         * First time initialization
         */
        if ((function_size == 0) && (sym->st_value > function_offset))
        {
            function_size = sym->st_value - function_offset;
        }
        else if ((sym->st_value != function_offset) && ((sym->st_value - function_offset) < (function_size)))
        {
            function_size = sym->st_value - function_offset;
        }
    }

    e->destroy(e);

    symtab_values_list->destroy(symtab_values_list);

    if (function_size == 0)
    {
        logging("get_function_chunk(): Unable to find function_size\n");
        return chunk_empty;
    }

    /**
     * Create new chunk pointing to function_offset
     */
    return chunk_create(function_section->get_real_chunk(function_section).ptr + (function_offset - function_section->get_sh_addr(function_section)),
                        function_size
                        );
}

static status_t add_program_header(private_elf_t *this, program_header_t *program_header)
{
    this->program_header_list->insert_last(this->program_header_list, program_header);
    return SUCCESS;
}

static bool compare_program_headers(void *a, void *b)
{
    return a == b;
}

static status_t remove_program_header(private_elf_t *this, program_header_t *program_header)
{
    this->program_header_list->remove(this->program_header_list, program_header, compare_program_headers);
    return SUCCESS;
}

static enumerator_t *get_program_header_enumerator(private_elf_t *this)
{
    return this->program_header_list->create_enumerator(this->program_header_list);
}

static linked_list_t *get_program_header_list(private_elf_t *this)
{
    return this->program_header_list;
}

static chunk_t get_program_header_data_chunk(private_elf_t *this, program_header_t *program_header)
{
    region_t *region;
    chunk_t data_chunk;
    chunk_t code_chunk;

    region = ((code_t*) this)->get_region((code_t*) this);
    code_chunk = region->get_chunk(region);
    data_chunk = chunk_create(code_chunk.ptr + program_header->get_p_offset(program_header), program_header->get_p_filesz(program_header));

    return data_chunk;
}

static void update_symbols_offsets(private_elf_t *this, section_t *section, Elf64_Off offset)
{
    uint32_t section_size;
    Elf64_Off section_start;
    section_t *section_to_update;
    linked_list_t *values_list;
    enumerator_t *e;
    chunk_t *c;

    /**
     * .symtab update
     */
    section_to_update = this->public.get_section_by_name(&this->public, ".symtab");
    values_list = section_to_update->create_symtab_linked_list(section_to_update);
    section_size = section->get_sh_size(section);
    section_start = section->get_sh_addr(section);

    e = values_list->create_enumerator(values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Sym *sym;

        sym = (Elf64_Sym*) c->ptr;

        /**
         * Hotfix for 0 values
         */
        if ((sym->st_value >= section_start) && (sym->st_value < (section_start + section_size)) && (sym->st_value))
        {
            sym->st_value+= offset - section->get_sh_offset(section);
        }
    }

    e->destroy(e);

    values_list->destroy(values_list);

    /**
     * .rela.dyn update
     */
    section_to_update = this->public.get_section_by_name(&this->public, ".rela.dyn");
    values_list = section_to_update->create_symtab_linked_list(section_to_update);
    section_size = section->get_sh_size(section);
    section_start = section->get_sh_addr(section);

    e = values_list->create_enumerator(values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Rela *sym;

        sym = (Elf64_Rela*) c->ptr;

        /**
         * Hotfix for 0 values
         */
        if ((sym->r_offset >= section_start) && (sym->r_offset < (section_start + section_size)) && (sym->r_offset))
        {
            sym->r_offset+= offset - section->get_sh_offset(section);
        }
    }

    e->destroy(e);

    values_list->destroy(values_list);
    
    /**
     * .rela.plt update
     */
    section_to_update = this->public.get_section_by_name(&this->public, ".rela.plt");
    values_list = section_to_update->create_symtab_linked_list(section_to_update);
    section_size = section->get_sh_size(section);
    section_start = section->get_sh_addr(section);

    e = values_list->create_enumerator(values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Rela *sym;

        sym = (Elf64_Rela*) c->ptr;

        /**
         * Hotfix for 0 values
         */
        if ((sym->r_offset >= section_start) && (sym->r_offset < (section_start + section_size)) && (sym->r_offset))
        {
            sym->r_offset+= offset - section->get_sh_offset(section);
        }
    }

    e->destroy(e);

    values_list->destroy(values_list);
}


/**
 * Interface functions
 */
static chunk_t get_type(private_elf_t* this)
{
    return this->type;
}   

static region_t *get_region(private_elf_t* this)
{   
    return this->region;
}

static status_t write_to_file(private_elf_t *this, char *name)
{
    enumerator_t *e;
    section_t *s;
    program_header_t *p;
    chunk_t *c;

    chunk_t new_file;
    chunk_t chunk_ehdr;
    chunk_t chunk_phdr;
    chunk_t chunk_data;
    chunk_t chunk_shdr;

    section_t *dynamic_section;
    section_t *new_dynamic_section;
    Elf64_Sxword section_tag;

    section_t *symtab_section;
    linked_list_t *symtab_values_list;

    /**
     * Usual base for elf
     */
    Elf64_Off base_address;

    base_address = 0x400000;

    /**
     * Write Ehdr chunk
     */
    chunk_ehdr = chunk_create((u_char*) this->elf64_ehdr, sizeof(*this->elf64_ehdr));

    /**
     * Write Phdr chunk
     *
     * First pass, to calculate chunk size
     */
    chunk_phdr = chunk_empty;

    e = this->public.get_program_header_enumerator(&this->public);

    while(e->enumerate(e, &p))
    {
        chunk_phdr = chunk_cat("mm", chunk_phdr, p->get_header(p));
    }

    e->destroy(e);

    /*
     * Write Sections chunk
     */
    chunk_data = chunk_empty;
    dynamic_section = this->public.get_section_by_name(&this->public, ".dynamic");
    new_dynamic_section = create_section(
                            dynamic_section->get_header(dynamic_section), 
                            dynamic_section->get_chunk(dynamic_section));

    /**
     * First pass to feed dynamic sections :
     *  - .dynamic
     *  - .symtab
     *
     *  WARNING: This pass have to be used in a feed way
     */
    e = this->public.get_section_enumerator(&this->public);

    while(e->enumerate(e, &s))
    {
        Elf64_Off offset;
        chunk_t chunk_align;

        chunk_align = chunk_empty;

        if (s->get_sh_type(s) == (SHT_NULL))
        {
            offset = 0;
        }
        else
        {
            offset = chunk_length("ccc", chunk_ehdr, chunk_phdr, chunk_data);

            chunk_align = create_alignment_chunk(s, offset);

            offset+= chunk_length("c", chunk_align);
        }

        /**
         * Update symbols offsets
         */
        this->public.update_symbols_offsets(&this->public, s, offset);
        s->set_sh_offset(s, offset);

        /**
         * Adjusting address to map.
         * Sections with flags (excepted .comment) have an address !0.
         */
        if (s->get_sh_flags(s) & SHF_WRITE)
        {
            s->set_sh_addr(s, base_address + offset + 0x200000);
        }
        else if (s->get_sh_flags(s) & SHF_ALLOC)
        {
            s->set_sh_addr(s, base_address + offset);
        }

        /**
         * Rewriting .dynamic entries to correctly map linking informations
         */
        section_tag = this->public.get_section_tag(&this->public, s);

        if (section_tag)
        {
            Elf64_Addr dynamic_value;

            dynamic_value = s->get_sh_addr(s);
            new_dynamic_section->set_tag_value(new_dynamic_section, section_tag, dynamic_value);
        }

        if (s->get_sh_type(s) != (SHT_NOBITS))
            chunk_data = chunk_cat("mmm", chunk_data, chunk_align, s->get_chunk(s));
    }

    e->destroy(e);

    /**
     * Second pass
     *
     * - replacing .dynamic section
     * - adjusting plt
     *
     * Modifications are made here based on first pass results
     */
    chunk_data = chunk_empty;

    e = this->public.get_section_enumerator(&this->public);

    while(e->enumerate(e, &s))
    {
        chunk_t chunk_to_add;
        chunk_t chunk_align;

        if (!(strcmp(this->public.get_section_name(&this->public, s), ".dynamic")))
        {
            chunk_to_add = new_dynamic_section->get_chunk(new_dynamic_section);
        }
        /**
         * If section is .plt, ensure that offsets are correctly mapped to the .got.plt
         *
         * Should be done after new sh_offset setting.
         */
        else if (!(strcmp(this->public.get_section_name(&this->public, s), ".plt")))
        {
            s->apply_got_offset_to_plt(s, this->public.get_section_by_name(&this->public, ".got.plt"));

            chunk_to_add = s->get_chunk(s);
        }
        /**
         * Need to modify _start with new arguments to libc_start_main
         *
         *   extern int BP_SYM (__libc_start_main) (int (*main) (int, char **, char **),
         *                   int argc,
         *                   char *__unbounded *__unbounded ubp_av,
         *                   void (*init) (void),
         *                   void (*fini) (void),
         *                   void (*rtld_fini) (void),
         *                   void *__unbounded stack_end)
         *   __attribute__ ((noreturn));
         *
         * => mov r8,  @__libc_csu_fini
         *    mov rcx, @__libc_csu_init
         *    mov rdi, @main
         *    call __libc_start_main@plt
         */
        else if (!(strcmp(this->public.get_section_name(&this->public, s), ".text")))
        {
            chunk_t start_function;

            start_function = this->public.get_function_chunk(&this->public, "_start");
            this->update_start_offsets(this, start_function);

            chunk_to_add = s->get_chunk(s);
        }
        else
        {
            chunk_to_add = s->get_chunk(s);
        }

        chunk_align = create_alignment_chunk(s, chunk_length("ccc", chunk_ehdr, chunk_phdr, chunk_data));

        if (s->get_sh_type(s) != (SHT_NOBITS))
            chunk_data = chunk_cat("mmm", chunk_data, chunk_align, chunk_to_add);
    }

    e->destroy(e);

    /**
     * Write Phdr
     */
    chunk_phdr = chunk_empty;
    ((Elf64_Ehdr*) chunk_ehdr.ptr)->e_phoff = chunk_length("cc", chunk_ehdr, chunk_phdr);

    e = this->public.get_program_header_enumerator(&this->public);

    while(e->enumerate(e, &p))
    {
        /**
         * Adjusting program headers to match new_file chunk offsets and section addresses
         */
        if (p->get_p_type(p) == PT_DYNAMIC)
        {
            section_t *dynamic_section;

            dynamic_section = this->public.get_section_by_name(&this->public, ".dynamic");
            p->set_p_offset(p, dynamic_section->get_sh_offset(dynamic_section));
            p->set_p_vaddr(p, dynamic_section->get_sh_addr(dynamic_section));
            p->set_p_paddr(p, dynamic_section->get_sh_addr(dynamic_section));
        }
        else if(p->get_p_type(p) == PT_NOTE)
        {
            section_t *note_section;

            note_section = this->public.get_section_by_name(&this->public, ".note.ABI-tag");
            p->set_p_offset(p, note_section->get_sh_offset(note_section));
            p->set_p_vaddr(p, note_section->get_sh_addr(note_section));
            p->set_p_paddr(p, note_section->get_sh_addr(note_section));
        }
        else if(p->get_p_type(p) == PT_GNU_EH_FRAME)
        {
            section_t *eh_frame_section;

            eh_frame_section = this->public.get_section_by_name(&this->public, ".eh_frame_hdr");
            p->set_p_offset(p, eh_frame_section->get_sh_offset(eh_frame_section));
            p->set_p_vaddr(p, eh_frame_section->get_sh_addr(eh_frame_section));
            p->set_p_paddr(p, eh_frame_section->get_sh_addr(eh_frame_section));
        }
        /**
         * To construct the R_E LOAD segment, we aggregate the SHF_ALLOC shdr.
         * To construct the RW_ LOAD segment, we aggregate the SHF_WRITE shdr.
         *
         * We suppose that they are contiguous.
         */
        else if (p->get_p_type(p) == PT_LOAD)
        {
            enumerator_t *section_enumerator;
            chunk_t segment_chunk;
            uint64_t segment_start;
            uint64_t segment_base;
            uint32_t flags_1;
            uint32_t flags_0;

            section_enumerator = this->public.get_section_enumerator(&this->public);
            segment_chunk = chunk_empty;
            segment_start = 0;
            segment_base = base_address;

            /**
             * Filtering segments based on their flags (better idea?)
             */
            if (p->get_p_flags(p) == (PF_R | PF_X))
            {
                flags_1 = SHF_ALLOC;
                flags_0 = SHF_WRITE;
            }
            else if (p->get_p_flags(p) == (PF_R | PF_W))
            {
                flags_1 = SHF_ALLOC | SHF_WRITE;
                flags_0 = 0;
                segment_base+= 0x200000;
            }
            else
            {
                logging("Weird case detected, aborting\n");
                return FAILED;
            }

            /**
             * Calculating segment start offset and segment size
             */
            while(section_enumerator->enumerate(section_enumerator, &s))
            {
                chunk_t section_data_chunk;
                chunk_t chunk_align;

                if (((s->get_sh_flags(s) & flags_1) == flags_1) &&
                    ((s->get_sh_flags(s) & flags_0) == 0))
                {
                    if (segment_start == 0)
                    {
                        segment_start = s->get_sh_offset(s);
                    }

                    //section_data_chunk = this->public.get_section_data_chunk(&this->public, s);
                    section_data_chunk = s->get_chunk(s);
                    //hexdump(section_data_chunk.ptr, section_data_chunk.len);

                    chunk_align = create_alignment_chunk(s, chunk_length("ccc", chunk_ehdr, chunk_phdr, segment_chunk));

                    if (s->get_sh_type(s) != (SHT_NOBITS))
                        segment_chunk = chunk_cat("mmm", segment_chunk, chunk_align, section_data_chunk);
                }
            }

            section_enumerator->destroy(section_enumerator);

            /**
             * Adjusting segment values
             */
            p->set_p_offset(p, segment_start);
            p->set_p_vaddr(p, segment_base + segment_start);
            p->set_p_paddr(p, segment_base + segment_start);
            p->set_p_filesz(p, chunk_length("c", segment_chunk));
            p->set_p_memsz(p, chunk_length("c", segment_chunk));
        }

        /**
         * Building Program header chunk by concatening all program headers
         */
        chunk_phdr = chunk_cat("mm", chunk_phdr, p->get_header(p));
    }

    e->destroy(e);

    /* Write Shdr */
    chunk_shdr = chunk_empty;
    ((Elf64_Ehdr*) chunk_ehdr.ptr)->e_shoff = chunk_length("ccc", chunk_ehdr, chunk_phdr, chunk_data);

    e = this->public.get_section_enumerator(&this->public);

    while(e->enumerate(e, &s))
    {
        chunk_shdr = chunk_cat("mm", chunk_shdr, s->get_header(s));
    }

    e->destroy(e);

    /**
     * Fix entry point
     */
    symtab_section = this->public.get_section_by_name(&this->public, ".symtab");
    symtab_values_list = symtab_section->create_symtab_linked_list(symtab_section);

    e = symtab_values_list->create_enumerator(symtab_values_list);

    while (e->enumerate(e, &c))
    {
        Elf64_Sym *sym;
        section_t *strtab_section;

        sym = (Elf64_Sym*) c->ptr;
        strtab_section = this->public.get_strtab_section(&this->public);

        if (!(strcmp((char*) strtab_section->get_chunk(strtab_section).ptr + sym->st_name, "_start")))
        {
            printf("Rewriting entry %lx->%lx\n", ((code_t*) this)->entry, sym->st_value);
            ((code_t*) this)->entry = sym->st_value;
            ((Elf64_Ehdr*) (this->elf64_ehdr))->e_entry = sym->st_value;
        }
    }

    e->destroy(e);

    symtab_values_list->destroy(symtab_values_list);

    chunk_ehdr = chunk_create((u_char*) this->elf64_ehdr, sizeof(*this->elf64_ehdr));

    new_file = chunk_cat("cccc", chunk_ehdr, chunk_phdr, chunk_data, chunk_shdr);

    chunk_write(new_file, name, 0000, 1);
    return SUCCESS;
}

static void destroy(private_elf_t *this)
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

    if (this->shstr_section)
        this->shstr_section->destroy(this->shstr_section);
    if (this->strtab_section)
        this->strtab_section->destroy(this->strtab_section);
    this->section_list->destroy(this->section_list);
    this->program_header_list->destroy(this->program_header_list);
    chunk_free(&this->type);
    free(this);
    this = NULL;
}

elf_t *create_elf(chunk_t type, region_t *region)
{
    private_elf_t *this = malloc_thing(private_elf_t);

    this->section_list = linked_list_create();
    this->program_header_list = linked_list_create();
    this->type = chunk_clone(type);
    this->region = region;

    /* Private functions */
    this->check_region_32b = (status_t (*)(region_t *)) check_region_32b;
    this->check_region_64b = (status_t (*)(region_t *)) check_region_64b;
    this->check_region = (status_t (*)(region_t *)) check_region;
    this->symbols_elf64 = (status_t (*)(private_elf_t*)) symbols_elf64;
    this->map_sections = (status_t (*)(private_elf_t*)) map_sections;
    this->map_program_headers = (status_t (*)(private_elf_t*)) map_program_headers;
    this->update_symbols_generic = (void (*)(private_elf_t*,section_t*,Elf64_Off,char*,size_t)) update_symbols_generic;
    this->update_start_offsets = (status_t (*)(private_elf_t*, chunk_t)) update_start_offsets;
    this->create_alignment_chunk = (chunk_t (*)(section_t*,uint64_t)) create_alignment_chunk;

    /* Public functions */
    this->public.add_section = (status_t (*)(elf_t*, section_t*)) add_section;
    this->public.remove_section = (status_t (*)(elf_t*, section_t*)) remove_section;
    this->public.get_section_list = (linked_list_t *(*)(elf_t*)) get_section_list;
    this->public.get_section_enumerator = (enumerator_t *(*)(elf_t*)) get_section_enumerator;
    this->public.get_shstr_section = (section_t *(*)(elf_t*)) get_shstr_section;
    this->public.get_strtab_section = (section_t *(*)(elf_t*)) get_strtab_section;
    this->public.get_section_name = (char *(*)(elf_t*, section_t*)) get_section_name;
    this->public.get_section_by_name = (section_t *(*)(elf_t*, char*)) get_section_by_name;
    this->public.get_section_data_chunk = (chunk_t (*)(elf_t*, section_t*)) get_section_data_chunk;
    this->public.get_section_tag = (Elf64_Sxword (*)(elf_t*, section_t*)) get_section_tag;
    this->public.is_section_tag_d_ptr = (bool (*)(elf_t*, Elf64_Sxword)) is_section_tag_d_ptr;
    this->public.update_symbols_offsets = (void (*)(elf_t*,section_t*,Elf64_Off)) update_symbols_offsets;
    this->public.add_program_header = (status_t (*)(elf_t*, program_header_t*)) add_program_header;
    this->public.remove_program_header = (status_t (*)(elf_t*, program_header_t*)) remove_program_header;
    this->public.get_program_header_list = (linked_list_t *(*)(elf_t*)) get_program_header_list;
    this->public.get_program_header_enumerator = (enumerator_t *(*)(elf_t*)) get_program_header_enumerator;
    this->public.get_program_header_data_chunk = (chunk_t (*)(elf_t*, program_header_t*)) get_program_header_data_chunk;
    this->public.get_function_offset = (Elf64_Addr (*)(elf_t*,char*)) get_function_offset;
    this->public.get_function_chunk = (chunk_t (*)(elf_t*,char*)) get_function_chunk;
    this->public.destroy = (void (*)(elf_t*)) destroy;

    /* Interface implementation */
    this->public.interface.get_type = (chunk_t (*)(code_t*)) get_type;
    this->public.interface.get_region = (region_t *(*)(code_t*)) get_region;
    this->public.interface.write_to_file = (status_t (*)(code_t*, char*)) write_to_file;
    this->public.interface.destroy = (void (*)(code_t*)) destroy;

    /* Feeding local object */
    this->symbols_elf64(this);
    this->map_sections(this);
    this->map_program_headers(this);
    this->public.interface.entry = this->elf64_ehdr->e_entry;

    return &this->public;
}
