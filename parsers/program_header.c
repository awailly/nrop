/*
 * Code program_header
 */
#include "program_header.h"

typedef struct private_program_header_t private_program_header_t;

struct private_program_header_t
{
    program_header_t public;

    chunk_t bytes_chunk;
    chunk_t data_chunk;

    Elf64_Phdr (*get_formatted_header)(private_program_header_t *);
};

static Elf64_Phdr get_formatted_header(private_program_header_t *this)
{
    Elf64_Phdr phdr;

    memcpy(&phdr, this->bytes_chunk.ptr, sizeof(phdr));

    return phdr;
}

static uint64_t get_p_filesz(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_filesz;
}

static void set_p_filesz(private_program_header_t *this, uint64_t filesz)
{
    chunk_t new_bytes_chunk;
    Elf64_Phdr *phdr;

    new_bytes_chunk = chunk_clone(this->bytes_chunk);

    phdr = ((Elf64_Phdr*) new_bytes_chunk.ptr);
    phdr->p_filesz = filesz;

    this->bytes_chunk = new_bytes_chunk;
}

static void set_p_memsz(private_program_header_t *this, uint64_t memsz)
{
    chunk_t new_bytes_chunk;
    Elf64_Phdr *phdr;

    new_bytes_chunk = chunk_clone(this->bytes_chunk);

    phdr = ((Elf64_Phdr*) new_bytes_chunk.ptr);
    phdr->p_memsz = memsz;

    this->bytes_chunk = new_bytes_chunk;
}

static uint64_t get_p_align(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_align;
}

static Elf64_Off get_p_offset(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_offset;
}

static void set_p_offset(private_program_header_t *this, Elf64_Off offset)
{
    chunk_t new_bytes_chunk;
    Elf64_Phdr *phdr;

    new_bytes_chunk = chunk_clone(this->bytes_chunk);

    phdr = ((Elf64_Phdr*) new_bytes_chunk.ptr);
    phdr->p_offset = offset;

    this->bytes_chunk = new_bytes_chunk;
}

static uint32_t get_p_type(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_type;
}

static uint64_t get_p_vaddr(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_vaddr;
}

static void set_p_vaddr(private_program_header_t *this, Elf64_Addr address)
{
    chunk_t new_bytes_chunk;
    Elf64_Phdr *phdr;

    new_bytes_chunk = chunk_clone(this->bytes_chunk);

    phdr = ((Elf64_Phdr*) new_bytes_chunk.ptr);
    phdr->p_vaddr = address;

    this->bytes_chunk = new_bytes_chunk;
}

static void set_p_paddr(private_program_header_t *this, Elf64_Addr address)
{
    chunk_t new_bytes_chunk;
    Elf64_Phdr *phdr;

    new_bytes_chunk = chunk_clone(this->bytes_chunk);

    phdr = ((Elf64_Phdr*) new_bytes_chunk.ptr);
    phdr->p_paddr = address;

    this->bytes_chunk = new_bytes_chunk;
}

static chunk_t get_header(private_program_header_t *this)
{
    return chunk_clone(this->bytes_chunk);
}

static chunk_t get_chunk(private_program_header_t *this)
{
    return chunk_clone(this->data_chunk);
}

static uint32_t get_p_flags(private_program_header_t *this)
{
    return this->get_formatted_header(this).p_flags;
}

static void destroy(private_program_header_t *this)
{
    free(this);
    this = NULL;
}

program_header_t *create_program_header(chunk_t program_header, chunk_t data)
{
    private_program_header_t *this = malloc_thing(private_program_header_t);

    this->bytes_chunk = program_header;
    this->data_chunk = data;

    this->public.get_p_type = (uint32_t (*)(program_header_t*)) get_p_type;
    this->public.get_p_flags= (uint32_t (*)(program_header_t*)) get_p_flags;
    this->public.get_p_offset = (Elf64_Off (*)(program_header_t*)) get_p_offset;
    this->public.set_p_offset = (void (*)(program_header_t*, Elf64_Off offset)) set_p_offset;
    this->public.get_p_vaddr= (uint64_t (*)(program_header_t*)) get_p_vaddr;
    this->public.set_p_vaddr= (void (*)(program_header_t*, Elf64_Addr address)) set_p_vaddr;
    this->public.set_p_paddr = (void (*)(program_header_t*, Elf64_Addr address)) set_p_paddr;
    this->public.get_p_filesz = (uint64_t (*)(program_header_t*)) get_p_filesz;
    this->public.set_p_filesz = (void (*)(program_header_t*, uint64_t)) set_p_filesz;
    this->public.set_p_memsz = (void (*)(program_header_t*, uint64_t)) set_p_memsz;
    this->public.get_p_align= (uint64_t (*)(program_header_t*)) get_p_align;
    this->public.destroy = (void (*)(program_header_t*)) destroy;

    this->public.get_header = (chunk_t (*)(program_header_t*)) get_header;
    this->public.get_chunk = (chunk_t (*)(program_header_t*)) get_chunk;
    this->get_formatted_header = (Elf64_Phdr (*)(private_program_header_t *)) get_formatted_header;

    return &this->public;
}
