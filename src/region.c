/*
 * Region implementation
 */

#include "region.h"

typedef struct private_region_t private_region_t;

struct private_region_t
{
    region_t public;

    int (*map)(private_region_t *, char *);

    char *filename;
    void *start;
    unsigned int len;
};

static int map(private_region_t *this, char *filename)
{
#if defined(_WIN32) 
    FILE* f;
    size_t t,ilen;
    xed_uint8_t* p;
#if defined(XED_MSVC8_OR_LATER)
    errno_t err;
    (stderr,"#Opening %s\n", path);
    err = fopen_s(&f,path,"rb");
#else
    int err=0;
    fprintf(stderr,"#Opening %s\n", path);
    f = fopen(path,"rb");
    err = (f==0);
#endif
    if (err != 0) {
        logging("ERROR: Could not open %s\n", path);
        fclose(f);
        return 1;
    }
    err =  fseek(f, 0, SEEK_END);
    if (err != 0) {
        logging("ERROR: Could not fseek %s\n", path);
        return 1;
    }
    ilen = ftell(f);
    fprintf(stderr,"#Trying to read " XED_FMT_SIZET "\n", ilen);
    p = (xed_uint8_t*)malloc(ilen);
    t=0;
    err = fseek(f,0, SEEK_SET);
    if (err != 0) {
        logging("ERROR: Could not fseek to (this->start) of file %s\n", path);
        free(p);
        return 1;
    }
    
    while(t < ilen) {
        size_t n;
        if (feof(f)) {
            fprintf(stderr, "#Read EOF. Stopping.\n");
            break;
        }
        n = fread(p+t, 1, ilen-t,f);
        t = t+n;
        logging("#Read " XED_FMT_SIZET " of " XED_FMT_SIZET " bytes\n", 
                t, ilen);
        if (ferror(f)) {
            logging("Error in file read. Stopping.\n");
            break;
        }
    }
    fclose(f);
    *(this->start) = p;
    *(this->len) = (unsigned int)ilen;
    
#else 
    int ilen,fd;
    fd = open(filename, O_RDONLY);
    if (fd == -1)   {
        logging("Could not open file: %s\n" , this->filename);
        return 1;
    }
    ilen = lseek(fd, 0, SEEK_END); // find the size.
    if (ilen == -1)
        logging("lseek failed");
    else 
        this->len = (unsigned int) ilen;

    lseek(fd, 0, SEEK_SET); // go to the beginning
    this->start = mmap(0,
                  this->len,
                  PROT_READ|PROT_WRITE,
                  MAP_PRIVATE,
                  fd,
                  0);
    if (this->start == (void*) -1)
        logging("could not map region");
    if (close(fd) != 0)
        logging("error closing fd for region mapping");
#endif
    return 0;
}

static chunk_t get_chunk(private_region_t *this)
{
    return chunk_create(this->start, this->len);
}

static void destroy(private_region_t *this)
{
    free(this);
}

region_t *region_create_file(char *filename)
{
    private_region_t *this = malloc_thing(private_region_t);

    this->public.get_chunk = (chunk_t (*)(region_t *)) get_chunk;
    this->public.destroy = (void (*)(region_t *)) destroy;

    this->map = (int (*)(private_region_t *, char *)) map;

    this->filename = filename;
    this->start = NULL;
    this->len = 0;

    if (this->filename)
    {
        if (this->map(this, this->filename))
        {
            logging("Error while mapping filename into region_create_file\n");
        }
    }

    return &this->public;
}

region_t *region_create()
{
    return region_create_file(NULL);
}
