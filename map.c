/*
 * Implementation of map_t
 */

#include "map.h"

#define DEBUG_Z3_SOLVE
#ifdef DEBUG_Z3_SOLVE
#  define LOG_Z3_SOLVE(...) logging(__VA_ARGS__)
#else
#  define LOG_Z3_SOLVE(...) do { } while (0)
#endif

typedef struct private_map_t private_map_t;

struct private_map_t
{
    map_t public;

    Z3_context ctx;
    Z3_ast ast;
    linked_list_t *symbols;
};

static Z3_ast get_ast(private_map_t *this)
{
    return this->ast;
}

static linked_list_t *get_symbols(private_map_t *this)
{
    return this->symbols;
}

static void dump(private_map_t *this)
{
    logging("Ctx:%x Ast:%x\n", this->ctx, this->ast);
}

/**
   \brief Create a variable using the given name and type.
*/
static Z3_ast mk_var(Z3_context ctx, const char * name, Z3_sort ty) 
{
    Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
    return Z3_mk_const(ctx, s, ty);
}

static gadget_type compare(private_map_t *this, map_t *other)
{
    Z3_solver solver, solvernot, solvernot_other;
    Z3_model model;
    enumerator_t *e, *e_other;
    Z3_symbol_cell *c, *c_other;
    linked_list_t *ll_other;

    int solver_res, solver_resnot, solver_resnot_other;
    gadget_type result;
    bool found_target_register;


    /*
    Z3_ast target, Z3_rdx, Z3_0;
    Z3_ast and, eq, not, args[2];
    Z3_sort bv_sort;
    bv_sort = Z3_mk_bv_sort(this->ctx, 32);

    Z3_rdx = Z3_mk_const(this->ctx, Z3_mk_string_symbol(this->ctx, "rax1"), bv_sort);
    Z3_0 = Z3_mk_unsigned_int(this->ctx, 0x1234, bv_sort);
    target = Z3_mk_eq(this->ctx, Z3_rdx, Z3_0);

    LOG_Z3_SOLVE("ATT: %s\n", Z3_ast_to_string(this->ctx, target));
    */

    /*
     * Idea from http://stackoverflow.com/questions/13930013/equivalence-checking-with-z3
    solver = Z3_mk_solver(this->ctx);
    eq = Z3_mk_eq(this->ctx, target, this->formula);
    not = Z3_mk_not(this->ctx, eq);
    args[0] = not;
    and = Z3_mk_and(this->ctx, 1, args);
    Z3_solver_assert(this->ctx, solver, and);
    solver_res = Z3_solver_check(this->ctx, solver);
    LOG_Z3_SOLVE("Sat result: %x\n", solver_res);

    model = Z3_solver_get_model(this->ctx, solver);
    LOG_Z3_SOLVE("Model: %s\n", Z3_model_to_string(this->ctx, model));
     */

    result = UNKN;

    /*
     * Idea from 0vercl0k
     * <over> pkwa tu fais pas ça en fait
     */
    LOG_Z3_SOLVE("Full and:::\n");
    solver = Z3_mk_solver(this->ctx);
    Z3_solver_inc_ref(this->ctx, solver);
    solvernot = Z3_mk_solver(this->ctx);
    Z3_solver_inc_ref(this->ctx, solvernot);
    solvernot_other = Z3_mk_solver(this->ctx);
    Z3_solver_inc_ref(this->ctx, solvernot_other);

    /*
    LOG_Z3_SOLVE("Making and\n");
    args[0] = this->ast;
    args[1] = other->get_ast(other);

    //LOG_Z3_SOLVE("Dumping AST:%s\n", Z3_ast_to_string(this->ctx, this->ast));
    //LOG_Z3_SOLVE("Dumping ATT:%s\n", Z3_ast_to_string(this->ctx, other->get_ast(other)));

    LOG_Z3_SOLVE("ctx:%x 0:%x 1:%x\n", this->ctx, args[0], args[1]);
    and = Z3_mk_and(this->ctx, 2, args);
    */

    LOG_Z3_SOLVE("Asserting\n");
    Z3_solver_assert(this->ctx, solver, this->ast);
    Z3_solver_assert(this->ctx, solver, other->get_ast(other));

    Z3_solver_assert(this->ctx, solvernot, this->ast);
    Z3_solver_assert(this->ctx, solvernot, other->get_ast(other));

    Z3_solver_assert(this->ctx, solvernot_other, this->ast);
    Z3_solver_assert(this->ctx, solvernot_other, Z3_mk_not(this->ctx, other->get_ast(other)));

    //Z3_set_ast_print_mode(this->ctx, Z3_PRINT_SMTLIB2_COMPLIANT);
    LOG_Z3_SOLVE("Dumping AST:%s\n", Z3_ast_to_string(this->ctx, this->ast));
    LOG_Z3_SOLVE("Dumping ATT:%s\n", Z3_ast_to_string(this->ctx, other->get_ast(other)));

    e = this->symbols->create_enumerator(this->symbols);
    ll_other = other->get_symbols(other);

    while(e->enumerate(e, &c))
    {
        /* For now, must remove after
         * XXX */
        if ((c->name.ptr) && ((strcmp((char*)c->name.ptr, "rip") == 0) ||
            (strcmp((char*)c->name.ptr, "eip") == 0)))
            continue;

        if (!c->is_global)
            continue;

        found_target_register = false;

        e_other = ll_other->create_enumerator(ll_other);

        while(e_other->enumerate(e_other, &c_other))
        {
            /*
            if ((c->name.ptr) && (c_other->name.ptr))
                logging("Trying [%x]:%s [%x]:%s\n", c->index, c->name.ptr, c_other->index, c_other->name);
            */

            if ((c->name.ptr) && (c_other->name.ptr) && (c->is_global) && (strcmp((char*)c->name.ptr, (char*)c_other->name.ptr) == 0))
            {
                found_target_register = true;

                Z3_sort bv_sort;
                Z3_ast a, b, eq;
                chunk_t new_a, new_b, name_a, name_b;
                bool need_free_name_a, need_free_name_b;

                /*
                 * Linking inputs
                 */
                if ((c->read_access) && (c_other->read_access))
                {
                    new_a = chunk_calloc(255);
                    new_b = chunk_calloc(255);

                    /*
                     * There is something better to do by using c->name.ptr directly.
                     */
                    name_a = c->name;
                    name_b = c_other->name;
                    need_free_name_a = false;
                    need_free_name_b = false;

                    if (c->prefix.ptr)
                    {
                        name_a = chunk_cat("cc", c->prefix, c->name);
                        need_free_name_a = true;
                    }

                    snprintf((char*)new_a.ptr, new_a.len, "%s", name_a.ptr);

                    if (c_other->prefix.ptr)
                    {
                        name_b = chunk_cat("cc", c_other->prefix, c_other->name);
                        need_free_name_b = true;
                    }

                    snprintf((char*)new_b.ptr, new_b.len, "%s", name_b.ptr);
                    
                    bv_sort = Z3_mk_bv_sort(this->ctx, 64);

                    a = mk_var(this->ctx, (char*)new_a.ptr, bv_sort);
                    b = mk_var(this->ctx, (char*)new_b.ptr, bv_sort);
                    eq = Z3_mk_eq(this->ctx, a, b);

                    Z3_solver_assert(this->ctx, solver, eq);
                    LOG_Z3_SOLVE("Dumping TTI:%s\n", Z3_ast_to_string(this->ctx, eq));

                    Z3_solver_assert(this->ctx, solvernot, eq);
                    Z3_solver_assert(this->ctx, solvernot_other, eq);

                    if (need_free_name_a)
                        chunk_free(&name_a);

                    if (need_free_name_b)
                        chunk_free(&name_b);

                    chunk_clear(&new_a);
                    chunk_clear(&new_b);
                }

                /*
                 * Linking outputs
                 */
                if ((c->write_access) && (c_other->write_access))
                {
                    new_a = chunk_calloc(255);
                    new_b = chunk_calloc(255);

                    name_a = c->name;
                    name_b = c_other->name;
                    need_free_name_a = false;
                    need_free_name_b = false;

                    if (c->prefix.ptr)
                    {
                        name_a = chunk_cat("cc", c->prefix, c->name);
                        need_free_name_a = true;
                    }

                    if (c->index > 0)
                        snprintf((char*)new_a.ptr, new_a.len, "%s%i", name_a.ptr, c->index);
                    else
                        snprintf((char*)new_a.ptr, new_a.len, "%s", name_a.ptr);

                    if (c_other->prefix.ptr)
                    {
                        name_b = chunk_cat("cc", c_other->prefix, c_other->name);
                        need_free_name_b = true;
                    }

                    if (c_other->index > 0)
                        snprintf((char*)new_b.ptr, new_b.len, "%s%i", name_b.ptr, c_other->index);
                    else
                        snprintf((char*)new_b.ptr, new_b.len, "%s", name_b.ptr);
                    
                    bv_sort = Z3_mk_bv_sort(this->ctx, 64);

                    a = mk_var(this->ctx, (char*)new_a.ptr, bv_sort);
                    b = mk_var(this->ctx, (char*)new_b.ptr, bv_sort);
                    eq = Z3_mk_eq(this->ctx, a, b);

                    Z3_solver_assert(this->ctx, solver, eq);
                    LOG_Z3_SOLVE("Dumping TTO:%s\n", Z3_ast_to_string(this->ctx, eq));

                    Z3_solver_assert(this->ctx, solvernot, Z3_mk_not(this->ctx, eq));
                    Z3_solver_assert(this->ctx, solvernot_other, eq);

                    if (need_free_name_a)
                        chunk_free(&name_a);

                    if (need_free_name_b)
                        chunk_free(&name_b);

                    chunk_clear(&new_a);
                    chunk_clear(&new_b);
                }

                break;
            }
        }

        e_other->destroy(e_other);

        if (!found_target_register)
        {
            LOG_Z3_SOLVE("Found bad [%s::%s]\n", c->name.ptr, c_other->name.ptr);
            result = BAD;
            break;
        }
    }

    e->destroy(e);

    if (result == BAD)
        return result;

    LOG_Z3_SOLVE("Checking\n");
    solver_res = Z3_solver_check(this->ctx, solver);

    if (solver_res == Z3_L_TRUE)
    {
        solver_resnot = Z3_solver_check(this->ctx, solvernot);

        if (solver_resnot == Z3_L_FALSE)
        {
            printf("Found PN2\n");
            result = PN2;
        }
        else
        {
            printf("Found PN1\n");
            result = PN1;
            model = Z3_solver_get_model(this->ctx, solvernot);
            LOG_Z3_SOLVE("Model: %s\n", Z3_model_to_string(this->ctx, model));
            //Z3_model_dec_ref(this->ctx, model);
        }


        LOG_Z3_SOLVE("Sat result: %x|%x\n", solver_res, solver_resnot);
    }

    LOG_Z3_SOLVE("Checking othernot:%x\n", Z3_solver_check(this->ctx, solvernot_other));
    LOG_Z3_SOLVE("%x:True %x:False %x:Undef\n", Z3_L_TRUE, Z3_L_FALSE, Z3_L_UNDEF);

    Z3_solver_dec_ref(this->ctx, solvernot_other);
    Z3_solver_dec_ref(this->ctx, solvernot);
    Z3_solver_dec_ref(this->ctx, solver);
    
    return result;
}

static void *clone_target_cell(void *target_cell)
{
    Z3_symbol_cell *c, *t;

    t = (Z3_symbol_cell*) target_cell;

    if ((c = malloc(sizeof(Z3_symbol_cell))) == NULL)
    {
        logging("Error while allocating instruction in clone_instruction from chain.c\n");
        return NULL;
    }

    memcpy(c, t, sizeof(*t));

    c->name = chunk_clone(t->name);
    c->prefix = chunk_clone(t->prefix);

    return c;
}

static void destroy_target_cell(void *target_cell)
{
    Z3_symbol_cell *c;

    c = (Z3_symbol_cell*) target_cell;

    if ((c) && (c->name.ptr))
        chunk_clear(&c->name);

    if ((c) && (c->prefix.ptr))
        chunk_clear(&c->prefix);

    free(target_cell);
    target_cell = NULL;
}

static void destroy(private_map_t *this)
{
    this->symbols->destroy_function(this->symbols, destroy_target_cell);

    free(this);
    this = NULL;
}

map_t *map_create(Z3_context ctx, Z3_ast ast, linked_list_t *symbols)
{
    private_map_t *this = malloc_thing(private_map_t);

    this->ctx = ctx;
    this->ast = ast;
    this->symbols = symbols->clone_function(symbols, clone_target_cell);

    this->public.get_ast = (Z3_ast (*)(map_t *)) get_ast;
    this->public.get_symbols = (linked_list_t *(*)(map_t *)) get_symbols;
    this->public.dump = (void (*)(map_t *)) dump;
    this->public.compare = (gadget_type (*)(map_t *, map_t *)) compare;
    this->public.destroy = (void (*)(map_t *)) destroy;

    return &this->public;
}
