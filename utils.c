/*
 * Useful tools :)
 *
 */

#include "utils.h"

/*
 * Hooking printf
 */
void logging(char *fmt __attribute__((unused)), ...)
{
        va_list arg;
        va_start(arg, fmt);
        vprintf(fmt, arg);
        va_end(arg);
}

/*
 * Implementing non-standard strdup
char *strdup(const char *str)
{
    int n = strlen(str) + 1;
    char *dup = malloc(n);
    if(dup)
    {
        strcpy(dup, str);
    }
    return dup;
}
 */

void hexdump(void *ptr, int buflen) {
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;
    for (i=0; i<buflen; i+=16) {
        printf("%06x: ", i);
        for (j=0; j<16; j++) 
            if (i+j < buflen)
                printf("\\x%02x", buf[i+j]);
            else
                printf("   ");
        printf(" ");
        for (j=0; j<16; j++) 
            if (i+j < buflen)
                printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
        printf("\n");
    }
}

