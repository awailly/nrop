/*
 * Parser interface
 */

#include "plugin_builder.h"

typedef struct private_plugin_builder_t private_plugin_builder_t;

struct private_plugin_builder_t
{
    plugin_builder_t public;

    linked_list_t *plugins_list;
};

static void add_plugin(private_plugin_builder_t *this, plugin_t *plugin)
{
    this->plugins_list->insert_last(this->plugins_list, plugin);
}

static bool compare_plugins(void *a, void *b)
{
    return a == b;
}

static void remove_plugin(private_plugin_builder_t *this, plugin_t *plugin)
{
    this->plugins_list->remove(this->plugins_list, plugin, compare_plugins);
}

static enumerator_t *get_enumerator(private_plugin_builder_t *this)
{
    return this->plugins_list->create_enumerator(this->plugins_list);
}

static void destroy(private_plugin_builder_t *this)
{
    this->plugins_list->destroy(this->plugins_list);
    free(this);
    this = NULL;
}

plugin_builder_t *plugin_builder_create()
{
    private_plugin_builder_t *this = malloc_thing(private_plugin_builder_t);

    this->public.add_plugin = (void (*)(plugin_builder_t *, plugin_t *)) add_plugin;
    this->public.remove_plugin = (void (*)(plugin_builder_t *, plugin_t *)) remove_plugin;
    this->public.get_enumerator = (enumerator_t *(*)(plugin_builder_t *)) get_enumerator;
    this->public.destroy = (void (*)(plugin_builder_t *)) destroy;

    this->plugins_list = linked_list_create();

    return &this->public;
}
