/*
 * Code program_header
 */

#ifndef __PROGRAM_HEADER_H__
#define __PROGRAM_HEADER_H__

#include "utils.h"
#include "chunk.h"

/*
 * Move in program_header_elf.h during refactor
 */
#include <elf.h>

typedef struct program_header_t program_header_t;

struct program_header_t
{
    /**
     * Program Header Structure getters and setters
     */
    Elf64_Off (*get_p_offset)(program_header_t *this);
    void (*set_p_offset)(program_header_t *this, Elf64_Off offset);

    uint32_t (*get_p_type)(program_header_t *this);

    uint64_t (*get_p_vaddr)(program_header_t *this);
    void (*set_p_vaddr)(program_header_t *this, Elf64_Addr address);

    void (*set_p_paddr)(program_header_t *this, Elf64_Addr address);

    uint32_t (*get_p_flags)(program_header_t *this);

    uint64_t (*get_p_filesz)(program_header_t *this);
    void (*set_p_filesz)(program_header_t *this, uint64_t filesz);

    uint64_t (*get_p_memsz)(program_header_t *this);
    void (*set_p_memsz)(program_header_t *this, uint64_t memsz);

    uint64_t (*get_p_align)(program_header_t *this);

    chunk_t (*get_header)(program_header_t* this);
    chunk_t (*get_chunk)(program_header_t* this);

    void (*destroy)(program_header_t* this);
};

program_header_t *create_program_header(chunk_t program_header, chunk_t data);

#endif
