/*
 * Parser interface
 */

#include "parser_elf.h"

typedef struct private_parser_elf_t private_parser_elf_t;

struct private_parser_elf_t
{
    parser_elf_t public;
    region_t *code;

    status_t (*check_region_32b)(region_t *);
    status_t (*check_region_64b)(region_t *);
    status_t (*check_region)(region_t *);
};

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

static status_t check_region(region_t *region)
{
    if (region == NULL)
        return FAILED;

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

static code_t *parse(private_parser_elf_t *this, region_t *region)
{
    if (this->check_region_32b(region))
    {
        elf_t *code = create_elf(chunk_create((unsigned char *)"ELF", 3), region, 32);
        return (code_t*) code;

    }
    else if (this->check_region_64b(region))
    {
        elf_t *code = create_elf(chunk_create((unsigned char *)"ELF", 3), region, 64);
        return (code_t*) code;
    }
    else
    {
        return NULL;
    }
}

static void destroy(private_parser_elf_t *this)
{
    free(this);
    this = NULL;
}

parser_elf_t *parser_elf_create()
{
    private_parser_elf_t *this = malloc_thing(private_parser_elf_t);

    this->public.interface.check_region = (status_t (*)(region_t *)) check_region;
    this->public.interface.parse = (code_t *(*)(parser_t *, region_t *)) parse;
    this->public.interface.destroy = (void (*)(parser_t *)) destroy;

    this->check_region_32b = (status_t (*)(region_t *)) check_region_32b;
    this->check_region_64b = (status_t (*)(region_t *)) check_region_64b;

    return &this->public;
}
