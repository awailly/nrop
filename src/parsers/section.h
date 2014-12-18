/*
 * Code section
 */

#ifndef __SECTION_H__
#define __SECTION_H__

#include "utils.h"
#include "chunk.h"
#include "linked_list.h"

/*
 * Move in section_elf.h during refactor
 */
#include <elf.h>

typedef struct section_t section_t;

union d_un_type
{
    Elf64_Xword d_val;
    Elf64_Addr d_ptr;
};

struct section_t
{
    /**
     * Section Header Structure getters and setters
     */
    uint32_t (*get_sh_name)(section_t *this);
    uint32_t (*get_sh_type)(section_t *this);
    uint64_t (*get_sh_flags)(section_t *this);
    Elf64_Addr (*get_sh_addr)(section_t *this);
    void (*set_sh_addr)(section_t *this, Elf64_Addr address);
    Elf64_Off (*get_sh_offset)(section_t *this);
    void (*set_sh_offset)(section_t *this, Elf64_Off offset);
    uint64_t (*get_sh_size)(section_t *this);
    uint64_t (*get_sh_addralign)(section_t *this);
    uint64_t (*get_sh_entsize)(section_t *this);

    /**
     * Provide facility to add a dynamic information chunk into the .dynamic segment.
     *
     * @param           new informations to add
     */
    void (*add_dynamic_info)(section_t *this, chunk_t new_dynamic_infos);

    /**
     * Retrieve a chunk in the .dynamic section with a specified tag.
     *
     * @param           tag chunk to find
     * @return          chunk informations
     */
    chunk_t (*get_dynamic_infos_by_tag)(section_t *this, Elf64_Sxword tag);

    /**
     * Set a dynamic section structure value to a new value
     *
     * @param           tag structure to modify
     * @param           new value to set
     */
    void (*set_tag_value)(section_t *this, Elf64_Sxword tag, Elf64_Addr value);

    /**
     * Set the value of a .symtab section object (Elf64_Sym) with st_name
     *
     * @param           st_name of the section to modify
     * @param           the value to set
     */
    void (*set_symtab_value)(section_t *this, uint32_t st_name, Elf64_Addr st_value);

    /**
     * Create symtab array linkedlist mapped on data chunk
     *
     * @return          linked list mapping data chunk
     */
    linked_list_t *(*create_symtab_linked_list)(section_t *this);

    /**
     * Create Relocation section '.rela.xxx' array linkedlist mapped on data chunk
     * Should work for .dyn and .plt
     *
     * @return          linked list mapping data chunk
     */
    linked_list_t *(*create_rela_linked_list)(section_t *this);

    /**
     * Parse plt to modify instruction flow and correctly jump to .got.plt
     *
     * Should only be applie to .plt
     */
    status_t (*apply_got_offset_to_plt)(section_t *this, section_t *got);

    /**
     * Modify an entry into the got
     *
     * Starting at 0
     */
    status_t (*set_entry)(section_t *this, unsigned int entry, chunk_t value);

    /**
     * Data access
     */
    chunk_t (*get_header)(section_t* this);
    chunk_t (*get_chunk)(section_t* this);
    chunk_t (*get_real_chunk)(section_t* this);

    void (*destroy)(section_t* this);
};

section_t *create_section(chunk_t header, chunk_t data);
section_t *create_section_empty();

#endif
