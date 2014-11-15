/*
 * Struct to store pe informations
 *
 * Following the man:
 *
 * ElfN_Ehdr
 * [
 *   
 * ]
 */

#ifndef __PE_TYPE_H__
#define __PE_TYPE_H__

#include "code.h"
#include "utils.h"
#include "chunk.h"
#include "region.h"
#include "section.h"
#include "program_header.h"
#include "xed-portability.h"

typedef struct pe_t pe_t;

/**
 * Implementation of code_t interface to parse and manipulate pe files.
 */
struct pe_t
{
    /**
     * Allocation of code_t struct at the beginning of our new pe_t file.
     */
    code_t interface;

    /**
     * Add a section header to the code_t object.
     *
     * @param section   the section_t to add
     * @return          SUCCESS if the section was successfully added, FAILED otherwise
     */
    status_t (*add_section)(pe_t*, section_t*);

    /**
     * Delete a section header to the code_t object.
     *
     * @param section   the section_t to remove
     * @return          SUCCESS if the section was successfully removed, FAILED otherwise
     */
    status_t (*remove_section)(pe_t*, section_t*);

    /**
     * Get the linked list containing all the section of the code.
     *
     * @return          section_t linked list object
     */
    linked_list_t *(*get_section_list)(pe_t*);

    /**
     * Create an enumerator over previously added sections.
     *
     * @return          enumerator_t over section_t
     */
    enumerator_t *(*get_section_enumerator)(pe_t*);

    /**
     * Find the .shstr section in the section_t linked list.
     *
     * @return          shstr section_t if found, NULL otherwise
     */
    section_t *(*get_shstr_section)(pe_t*);

    /**
     * Find the .strtab section in the section_t linked list.
     *
     * @return          strtab section_t if found, NULL otherwise
     */
    section_t *(*get_strtab_section)(pe_t*);

    /**
     * Find the name of a section by using the .shstr section
     *
     * @param           section to find name
     * @returm          section name
     */
    char *(*get_section_name)(pe_t*, section_t*);

    /**
     * Find section into the code by its name.
     *
     * It uses the shstr facility to resolve the sh_name index.
     *
     * @param name      the name of the section to find
     * @return          name section_t if found, NULL otherwise
     */
    section_t *(*get_section_by_name)(pe_t*, char*);

    /**
     * Find chunk represented by the section_t header.
     *
     * @param section   section header containing data offsets
     * @return          chunk_t containing associated bytes
     */
    chunk_t (*get_section_data_chunk)(pe_t *, section_t *section);

    /**
     * Find the Elf64_Sxword tag (DT_XXXX) of a section based on usual mappings.
     * Returns 0 if no mapping is found.
     *
     * @param           the section which contains the tag
     * @return          section tag
     */
    Elf64_Sxword (*get_section_tag)(pe_t *, section_t *section);

    /**
     * Test section tag against known DT_XXXX tags to find if this is a d_ptr pointer.
     *
     * @param           sect
     * @return          True is this is a d_ptr, False otherwise.
     */
    bool (*is_section_tag_d_ptr)(pe_t *, Elf64_Sxword tag);

    /**
     * Update the .symtab, .rela.dyn and .rela.plt sections with the new offset for the section
     *
     * @param           section to modify
     * @param           offset to apply
     */
    void (*update_symbols_offsets)(pe_t *this, section_t *section, Elf64_Off offset);

    /**
     * Add a program_header header to the code_t object.
     *
     * @param program_header   the program_header_t to add
     * @return          SUCCESS if the program_header was successfully added, FAILED otherwise
     */
    status_t (*add_program_header)(pe_t*, program_header_t*);

    /**
     * Delete a program_header header to the code_t object.
     *
     * @param program_header   the program_header_t to remove
     * @return          SUCCESS if the program_header was successfully removed, FAILED otherwise
     */
    status_t (*remove_program_header)(pe_t*, program_header_t*);

    /**
     * Get the linked list containing all the program_header of the code.
     *
     * @return          program_header_t linked list object
     */
    linked_list_t *(*get_program_header_list)(pe_t*);

    /**
     * Create an enumerator over previously added program_headers.
     *
     * @return          enumerator_t over program_header_t
     */
    enumerator_t *(*get_program_header_enumerator)(pe_t*);

    /**
     * Find chunk represented by the program_header_t header.
     *
     * @param program_header   program_header header containing data offsets
     * @return          chunk_t containing associated bytes
     */
    chunk_t (*get_program_header_data_chunk)(pe_t *, program_header_t *program_header);

    /**
     * Find mapped offset of a function name.
     *
     * @param           name of the function to find
     * @return          offset of the function
     */
    Elf64_Addr (*get_function_offset)(pe_t *, char *function_name);

    /**
     * Return a chunk pointing to the function name inside the section.
     *
     * @param           name of the function to find
     * @return          chunk containing function mapped on section
     */
    chunk_t (*get_function_chunk)(pe_t *this, char *function_name);

    /**
     * Destroys an pe_t object.
     */
    void (*destroy)(pe_t *);
};

/**
 * Creates an pe file object based on a region.
 *
 * @return          pe_t object
 */
pe_t *create_pe(chunk_t type, region_t *region);

#endif
