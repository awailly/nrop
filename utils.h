/*
 * Useful tools :)
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>

#define malloc_thing(thing) ((thing*)malloc(sizeof(thing)))

typedef unsigned char u_char;
typedef unsigned int u_int32_t;
typedef unsigned int u_int;

#define memeq(x,y,len) (memcmp(x, y, len) == 0)
#define max(x,y) ((x) > (y) ? (x):(y))
#define min(x,y) ((x) < (y) ? (x):(y))
#define streq(x,y) (strcmp(x, y) == 0)

typedef enum status_e {
    SUCCESS,
    FAILED,
    OUT_OF_RES,
    ALREADY_DONE,
    NOT_SUPPORTED,
    INVALID_ARG,
    NOT_FOUND,
    PARSE_ERROR,
    VERIFY_ERROR,
    INVALID_STATE
} status_t;

/*char *strdup(const char *str);*/
void logging(char *fmt __attribute__((unused)), ...);
void hexdump(void *ptr, int buflen);

typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;
#define DT_GNU_HASH 0x6ffffef5  /* GNU-style hash table.  */
#define PT_GNU_EH_FRAME 0x6474e550  /* GCC .eh_frame_hdr segment */

#endif
