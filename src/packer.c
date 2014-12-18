/*
 * Command line parser
 *
 */

#include "packer.h"
#include "region.h"
#include "code.h"
#include "constraints.h"
#include "parsers/parser_builder.h"
#include "plugins/plugin_builder.h"

/* Temp soz */
#include "parsers/parser_elf.h"
#include "parsers/parser_pe.h"
#include "plugins/plugin_rop.h"

int usage(char *s)
{
    printf("Usage: %s [OPTIONS] file\n", s);
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    char *file;
    char *outfile;

    /*
     * Specific to rop plugin
     */
    char *constraints;
    chunk_t target;

    region_t *region;
    parser_builder_t *parser_builder;
    plugin_builder_t *plugin_builder;
    enumerator_t *e;
    code_t *code;

    parser_t *p, *parser;
    plugin_t *plugin;

    parser_elf_t *parser_elf;
    parser_pe_t *parser_pe;
    plugin_rop_t *plugin_rop;

    if (argc == 1)
    {
        usage(argv[0]);
        return 0;
    }

    /*
     * Parsing options
     */
    outfile = NULL;
    constraints = NULL;
    target = chunk_empty;

    while ((opt = getopt(argc, argv, "o:c:t:")) != -1) 
    {
        switch (opt)
        {
            case 'o':
                outfile = optarg;
                break;
            case 'c':
                constraints = optarg;
                break;
            case 't':
                target = chunk_create((unsigned char*) optarg, strlen(optarg));
                break;
            default: /* '?' */
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /*
     * Mapping input file
     */
    file = argv[argc - 1];
    region = region_create_file(file);

    if (region == NULL)
        goto error_region;

    /*
     * Parsing file
     */
    parser_builder = parser_builder_create();

    parser_elf = parser_elf_create();
    parser_builder->add_parser(parser_builder, (parser_t *) parser_elf);
    parser_pe = parser_pe_create();
    parser_builder->add_parser(parser_builder, (parser_t *) parser_pe);

    e = parser_builder->get_enumerator(parser_builder);
    parser = NULL;

    while(e->enumerate(e, &p) && (parser == NULL))
    {
        if (p->check_region(region) == SUCCESS)
        {
            parser = p;
        }
    }

    e->destroy(e);

    if (!(parser))
    {
        logging("[-] No valid parser found.\n");
        goto error_parser;
    }

    code = parser->parse(parser, region);

    /*
    hexdump(code->get_region(code)->get_chunk(code->get_region(code)).ptr, 32);
    */

    if (code) logging("Found Entry @%"PRIx64"\n", code->entry);

    /*
     * Loading plugins
     */
    plugin_builder = plugin_builder_create();

    plugin_rop = plugin_rop_create(code, constraints, target);
    plugin_builder->add_plugin(plugin_builder, (plugin_t *) plugin_rop);

    e = plugin_builder->get_enumerator(plugin_builder);

    while(e->enumerate(e, &plugin))
    {
        if (plugin->apply(plugin) != SUCCESS)
        {
            logging("[-] Error with plugin\n");
        }
    }

    e->destroy(e);

    /*
     * After plugins actions (e.g. saving file...)
     */
    if ((outfile) && (code))
    {
        code->write_to_file(code, outfile);
    }

    /*
     * Freeing memory
     */
    if (code) code->destroy(code);

    e = plugin_builder->get_enumerator(plugin_builder);

    while(e->enumerate(e, &plugin))
    {
        plugin->destroy(plugin);
    }

    e->destroy(e);
    plugin_builder->destroy(plugin_builder);

error_parser:
    e = parser_builder->get_enumerator(parser_builder);

    while(e->enumerate(e, &p))
    {
        p->destroy(p);
    }

    e->destroy(e);
    parser_builder->destroy(parser_builder);
    region->destroy(region);

error_region:
    /* Not needed as target just point to the stack, not malloc'd
    chunk_clear(&target);
    */
    return 0;
}
