#include "compiler/sema.h"
#include "compiler/comptime.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/type.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool sema_trace_enabled(void)
{
    const char *v = getenv("MACH_SEMA_TRACE");
    return v && v[0] != '\0';
}

typedef struct SemaModule SemaModule;

typedef struct ModuleImport
{
    SemaModule          *module;
    struct ModuleImport *next;
} ModuleImport;

typedef struct ModuleAlias
{
    char               *alias;
    SemaModule         *module;
    struct ModuleAlias *next;
} ModuleAlias;

// track loaded modules to avoid circular imports and redundant work.
// each module has its own global symbol table.
struct SemaModule
{
    char        *module_path; // fully qualified module path
    char        *source;      // source text (kept for error reporting)
    char        *file_path;   // file path for error messages
    AstNode     *ast;         // parsed AST
    SymbolTable *table;       // module-level global symbol table

    ModuleImport *imports; // unaliased `use module.path;` imports
    ModuleAlias  *aliases; // aliased `use a: module.path;` imports

    struct SemaModule *next;
};

// additional module roots (prefix -> src_root)
// used by single-file compilation mode via `-I prefix=/abs/dir`.
typedef struct ModuleRoot
{
    char *prefix;
    char *src_root;
} ModuleRoot;

// semantic error record
typedef struct SemaError
{
    Token *token;
    char  *message;
    char  *file_path; // which file the error is in
    char  *source;    // source text for that file
} SemaError;

// semantic analyzer context
struct Sema
{
    // module state
    SemaModule *modules;        // loaded modules (including entry module)
    SemaModule *main_module;    // entry module
    SemaModule *current_module; // module currently being analyzed

    SymbolTable *root_table; // used only for nested scopes that are not module globals (kept for compatibility)
    SymbolTable *current_table;
    int          error_count;
    const char  *module_path; // borrowed from current_module->module_path
    Type        *current_function_return_type;
    bool         current_function_is_variadic;

    // error list
    SemaError *errors;
    int        errors_count;
    int        errors_capacity;

    // current file context for error reporting
    char *current_file_path;
    char *current_source;

    // module resolution
    char       *project_id; // project id (module prefix)
    char       *src_root;   // source directory path
    char       *dep_root;   // dependencies directory path
    ConfigDep **deps;       // dependencies for module resolution
    int         dep_count;  // number of dependencies
    // modules are tracked in `modules`

    // extra module roots (for single-file mode)
    ModuleRoot *extra_roots;
    int         extra_root_count;
    int         extra_root_capacity;
};

// add error to list
static void sema_error_list_add(Sema *sema, Token *token, const char *message)
{
    if (sema->errors_count >= sema->errors_capacity)
    {
        sema->errors_capacity = sema->errors_capacity ? sema->errors_capacity * 2 : 8;
        sema->errors          = realloc(sema->errors, sizeof(SemaError) * sema->errors_capacity);
    }

    SemaError *err = &sema->errors[sema->errors_count++];

    // copy token
    if (token)
    {
        err->token = malloc(sizeof(Token));
        if (err->token)
        {
            token_copy(token, err->token);
        }
    }
    else
    {
        err->token = NULL;
    }

    err->message   = strdup(message);
    err->file_path = sema->current_file_path ? strdup(sema->current_file_path) : NULL;
    err->source    = sema->current_source ? strdup(sema->current_source) : NULL;
}

Sema *sema_create(const char *module_path)
{
    Sema *sema = malloc(sizeof(Sema));
    if (!sema)
    {
        return NULL;
    }

    sema->modules        = NULL;
    sema->main_module    = NULL;
    sema->current_module = NULL;

    sema->root_table                   = NULL;
    sema->current_table                = NULL;
    sema->error_count                  = 0;
    sema->module_path                  = NULL;
    sema->errors                       = NULL;
    sema->errors_count                 = 0;
    sema->errors_capacity              = 0;
    sema->current_file_path            = NULL;
    sema->current_source               = NULL;
    sema->project_id                   = NULL;
    sema->src_root                     = NULL;
    sema->dep_root                     = NULL;
    sema->deps                         = NULL;
    sema->dep_count                    = 0;
    sema->current_function_return_type = NULL;
    sema->current_function_is_variadic = false;
    sema->extra_roots                  = NULL;
    sema->extra_root_count             = 0;
    sema->extra_root_capacity          = 0;

    // create entry module
    SemaModule *main_mod = calloc(1, sizeof(SemaModule));
    if (!main_mod)
    {
        free(sema);
        return NULL;
    }

    main_mod->module_path = module_path ? strdup(module_path) : strdup("main");
    main_mod->source      = NULL;
    main_mod->file_path   = NULL;
    main_mod->ast         = NULL;
    main_mod->table       = symbol_table_create(NULL);
    main_mod->imports     = NULL;
    main_mod->aliases     = NULL;
    main_mod->next        = NULL;

    if (!main_mod->module_path || !main_mod->table)
    {
        if (main_mod->module_path)
        {
            free(main_mod->module_path);
        }
        if (main_mod->table)
        {
            symbol_table_destroy(main_mod->table);
        }
        free(main_mod);
        free(sema);
        return NULL;
    }

    sema->modules        = main_mod;
    sema->main_module    = main_mod;
    sema->current_module = main_mod;
    sema->module_path    = main_mod->module_path;
    sema->current_table  = main_mod->table;

    return sema;
}

void sema_destroy(Sema *sema)
{
    if (!sema)
    {
        return;
    }

    if (sema->root_table)
    {
        symbol_table_destroy(sema->root_table);
    }
    if (sema->project_id)
    {
        free(sema->project_id);
    }
    if (sema->src_root)
    {
        free(sema->src_root);
    }
    if (sema->dep_root)
    {
        free(sema->dep_root);
    }

    // free deep-copied dependencies
    if (sema->deps)
    {
        for (int i = 0; i < sema->dep_count; i++)
        {
            if (sema->deps[i])
            {
                free(sema->deps[i]->name);
                free(sema->deps[i]->path);
                free(sema->deps[i]);
            }
        }
        free(sema->deps);
    }

    // free error list
    for (int i = 0; i < sema->errors_count; i++)
    {
        if (sema->errors[i].token)
        {
            token_dnit(sema->errors[i].token);
            free(sema->errors[i].token);
        }
        free(sema->errors[i].message);
        free(sema->errors[i].file_path);
        free(sema->errors[i].source);
    }
    free(sema->errors);

    // current file context is borrowed from the current module; do not free here.

    // free loaded modules (including entry module)
    SemaModule *mod = sema->modules;
    while (mod)
    {
        SemaModule *next = mod->next;

        free(mod->module_path);
        free(mod->file_path);
        free(mod->source);
        if (mod->table)
        {
            symbol_table_destroy(mod->table);
        }

        ModuleImport *imp = mod->imports;
        while (imp)
        {
            ModuleImport *imp_next = imp->next;
            free(imp);
            imp = imp_next;
        }

        ModuleAlias *al = mod->aliases;
        while (al)
        {
            ModuleAlias *al_next = al->next;
            free(al->alias);
            free(al);
            al = al_next;
        }

        // note: ast is not freed here as it may still be in use
        free(mod);
        mod = next;
    }

    // free extra module roots
    for (int i = 0; i < sema->extra_root_count; i++)
    {
        free(sema->extra_roots[i].prefix);
        free(sema->extra_roots[i].src_root);
    }
    free(sema->extra_roots);

    free(sema);
}

void sema_add_module_root(Sema *sema, const char *module_prefix, const char *src_root)
{
    if (!sema || !module_prefix || !src_root)
    {
        return;
    }

    if (sema->extra_root_count >= sema->extra_root_capacity)
    {
        int         new_cap   = sema->extra_root_capacity ? sema->extra_root_capacity * 2 : 4;
        ModuleRoot *new_roots = realloc(sema->extra_roots, sizeof(ModuleRoot) * new_cap);
        if (!new_roots)
        {
            return;
        }
        sema->extra_roots         = new_roots;
        sema->extra_root_capacity = new_cap;
    }

    ModuleRoot *mr = &sema->extra_roots[sema->extra_root_count++];
    mr->prefix     = strdup(module_prefix);
    mr->src_root   = strdup(src_root);
}

void sema_set_module_roots(Sema *sema, const char *project_id, const char *src_root, const char *dep_root, ConfigDep **deps, int dep_count)
{
    if (!sema)
    {
        return;
    }

    if (sema->project_id)
    {
        free(sema->project_id);
    }
    if (sema->src_root)
    {
        free(sema->src_root);
    }
    if (sema->dep_root)
    {
        free(sema->dep_root);
    }
    if (sema->deps)
    {
        free(sema->deps);
    }

    sema->project_id = project_id ? strdup(project_id) : NULL;
    sema->src_root   = src_root ? strdup(src_root) : NULL;
    sema->dep_root   = dep_root ? strdup(dep_root) : NULL;

    // make deep copies of dependencies (they will be freed by Config before sema is destroyed)
    if (deps && dep_count > 0)
    {
        sema->deps = malloc(sizeof(ConfigDep *) * dep_count);
        if (sema->deps)
        {
            for (int i = 0; i < dep_count; i++)
            {
                ConfigDep *dep_copy = malloc(sizeof(ConfigDep));
                if (dep_copy && deps[i])
                {
                    dep_copy->name    = deps[i]->name ? strdup(deps[i]->name) : NULL;
                    dep_copy->type    = deps[i]->type;
                    dep_copy->path    = deps[i]->path ? strdup(deps[i]->path) : NULL;
                    dep_copy->version = NULL;            // we don't need version info for module resolution
                    dep_copy->config  = deps[i]->config; // share the config pointer (not owned by sema)
                }
                else
                {
                    dep_copy = NULL;
                }
                sema->deps[i] = dep_copy;
            }
            sema->dep_count = dep_count;
        }
        else
        {
            sema->dep_count = 0;
        }
    }
    else
    {
        sema->deps      = NULL;
        sema->dep_count = 0;
    }
}

static bool sema_has_module_resolution(Sema *sema)
{
    if (!sema)
    {
        return false;
    }

    if (sema->project_id && sema->src_root)
    {
        return true;
    }
    if (sema->extra_root_count > 0)
    {
        return true;
    }
    if (sema->deps && sema->dep_count > 0 && sema->dep_root)
    {
        return true;
    }
    return false;
}

void sema_set_file_context(Sema *sema, const char *file_path, const char *source)
{
    if (!sema)
    {
        return;
    }

    // associate the provided context with the entry module
    if (sema->main_module)
    {
        free(sema->main_module->file_path);
        free(sema->main_module->source);

        sema->main_module->file_path = file_path ? strdup(file_path) : NULL;
        sema->main_module->source    = source ? strdup(source) : NULL;
    }

    // update current reporting context to match current module
    sema->current_file_path = sema->main_module ? sema->main_module->file_path : NULL;
    sema->current_source    = sema->main_module ? sema->main_module->source : NULL;
}

SymbolTable *sema_get_main_module_table(Sema *sema)
{
    return (sema && sema->main_module) ? sema->main_module->table : NULL;
}

int sema_get_loaded_modules(Sema *sema, SemaLoadedModule *modules, int max_count)
{
    if (!sema || !modules || max_count <= 0)
    {
        return 0;
    }

    int         count = 0;
    SemaModule *mod   = sema->modules;
    while (mod && count < max_count)
    {
        if (mod != sema->main_module)
        {
            modules[count].module_path = mod->module_path;
            modules[count].ast         = mod->ast;
            modules[count].table       = mod->table;
            count++;
        }
        mod = mod->next;
    }

    return count;
}

int sema_get_error_count(Sema *sema)
{
    return sema ? sema->error_count : 0;
}

// helper: get line number from position in source
static int sema_get_line(const char *source, int pos)
{
    int line = 0;
    for (int i = 0; i < pos && source[i] != '\0'; i++)
    {
        if (source[i] == '\n')
        {
            line++;
        }
    }
    return line;
}

// helper: get column offset from position in source
static int sema_get_column(const char *source, int pos)
{
    int col = 0;
    for (int i = 0; i < pos && source[i] != '\0'; i++)
    {
        if (source[i] == '\n')
        {
            col = 0;
        }
        else
        {
            col++;
        }
    }
    return col;
}

// helper: get line text from source
static char *sema_get_line_text(const char *source, int line)
{
    int line_start   = 0;
    int current_line = 0;

    while (current_line < line && source[line_start] != '\0')
    {
        if (source[line_start] == '\n')
        {
            current_line++;
        }
        line_start++;
    }

    if (source[line_start] == '\0')
    {
        return strdup("");
    }

    int line_end = line_start;
    while (source[line_end] != '\n' && source[line_end] != '\0')
    {
        line_end++;
    }

    int   line_len  = line_end - line_start;
    char *line_text = malloc(line_len + 1);
    if (line_text)
    {
        strncpy(line_text, source + line_start, line_len);
        line_text[line_len] = '\0';
    }
    return line_text;
}

void sema_print_errors(Sema *sema)
{
    if (!sema || sema->errors_count == 0)
    {
        return;
    }

    for (int i = 0; i < sema->errors_count; i++)
    {
        SemaError *err = &sema->errors[i];

        if (err->token && err->source)
        {
            int   line      = sema_get_line(err->source, err->token->pos);
            int   col       = sema_get_column(err->source, err->token->pos);
            char *line_text = sema_get_line_text(err->source, line);

            fprintf(stderr, "error: %s\n", err->message);
            fprintf(stderr, "%s:%d:%d\n", err->file_path ? err->file_path : "<unknown>", line + 1, col + 1);
            fprintf(stderr, "%5d | %s\n", line + 1, line_text ? line_text : "");

            // print caret
            fprintf(stderr, "      | ");
            for (int j = 0; j < col; j++)
            {
                fprintf(stderr, " ");
            }
            // underline the token
            int underline_len = err->token->len > 0 ? err->token->len : 1;
            fprintf(stderr, "^");
            for (int j = 1; j < underline_len; j++)
            {
                fprintf(stderr, "~");
            }
            fprintf(stderr, "\n");

            free(line_text);
        }
        else if (err->token)
        {
            fprintf(stderr, "error at position %d: %s\n", err->token->pos, err->message);
        }
        else
        {
            fprintf(stderr, "error: %s\n", err->message);
        }
    }

    fprintf(stderr, "%d semantic error(s) found\n", sema->error_count);
}

static void sema_error(Sema *sema, Token *token, const char *message)
{
    if (!sema)
    {
        return;
    }

    if (sema_trace_enabled())
    {
        fprintf(stderr,
                "[sema] error: module=%s file=%s pos=%d len=%d msg=%s\n",
                sema->module_path ? sema->module_path : "(null)",
                sema->current_file_path ? sema->current_file_path : "(null)",
                token ? token->pos : -1,
                token ? token->len : -1,
                message ? message : "(null)");
    }

    sema->error_count++;
    sema_error_list_add(sema, token, message);
}

// forward declarations for mutual recursion
static int sema_analyze_stmt(Sema *sema, AstNode *node);
static int sema_analyze_expr(Sema *sema, AstNode *node);

// helpers for numeric literal inference
static bool sema_try_infer_numeric_literal(AstNode *node, Type *target);
static bool sema_is_untyped_numeric_literal(AstNode *node)
{
    if (!node || node->kind != AST_EXPR_LIT || !node->token)
    {
        return false;
    }

    if (node->type != NULL)
    {
        return false; // already has a type
    }

    if (node->token->type_suffix != NULL)
    {
        return false; // explicitly typed literal
    }

    return node->token->kind == TOKEN_LIT_INT || node->token->kind == TOKEN_LIT_FLOAT;
}

static void sema_infer_numeric_expr(AstNode *node, Type *target)
{
    if (!node || !target || !type_is_numeric(target))
    {
        return;
    }

    switch (node->kind)
    {
    case AST_EXPR_LIT:
        sema_try_infer_numeric_literal(node, target);
        break;

    case AST_EXPR_UNARY:
        sema_infer_numeric_expr(node->unary_expr.expr, target);
        if (!node->type && node->unary_expr.expr && node->unary_expr.expr->type && type_is_numeric(node->unary_expr.expr->type))
        {
            node->type = node->unary_expr.expr->type;
        }
        break;

    case AST_EXPR_BINARY:
        sema_infer_numeric_expr(node->binary_expr.left, target);
        sema_infer_numeric_expr(node->binary_expr.right, target);
        if (!node->type)
        {
            if (node->binary_expr.left && node->binary_expr.left->type == target)
            {
                node->type = target;
            }
            else if (node->binary_expr.right && node->binary_expr.right->type == target)
            {
                node->type = target;
            }
        }
        break;

    default:
        break;
    }
}

static bool sema_check_untyped_numeric(Sema *sema, AstNode *node, const char *message)
{
    if (!node)
    {
        return true;
    }

    if (sema_is_untyped_numeric_literal(node))
    {
        sema_error(sema, node->token, message);
        return false;
    }

    switch (node->kind)
    {
    case AST_EXPR_UNARY:
        return sema_check_untyped_numeric(sema, node->unary_expr.expr, message);

    case AST_EXPR_BINARY:
        return sema_check_untyped_numeric(sema, node->binary_expr.left, message) && sema_check_untyped_numeric(sema, node->binary_expr.right, message);

    case AST_EXPR_CALL:
        if (node->call_expr.args)
        {
            for (int i = 0; i < node->call_expr.args->count; i++)
            {
                if (!sema_check_untyped_numeric(sema, node->call_expr.args->items[i], message))
                {
                    return false;
                }
            }
        }
        return true;

    case AST_EXPR_ARRAY:
        if (node->array_expr.elems)
        {
            for (int i = 0; i < node->array_expr.elems->count; i++)
            {
                if (!sema_check_untyped_numeric(sema, node->array_expr.elems->items[i], message))
                {
                    return false;
                }
            }
        }
        return true;

    case AST_EXPR_STRUCT:
        if (node->struct_expr.fields)
        {
            for (int i = 0; i < node->struct_expr.fields->count; i++)
            {
                AstNode *field_init = node->struct_expr.fields->items[i];
                if (field_init && !sema_check_untyped_numeric(sema, field_init->field_expr.object, message))
                {
                    return false;
                }
            }
        }
        return true;

    case AST_EXPR_FIELD:
        return sema_check_untyped_numeric(sema, node->field_expr.object, message);

    case AST_EXPR_CAST:
        return sema_check_untyped_numeric(sema, node->cast_expr.expr, message);

    case AST_EXPR_INDEX:
        return sema_check_untyped_numeric(sema, node->index_expr.index, message);

    default:
        return true;
    }
}

// default any remaining untyped numeric literals in a condition expression to i64/f64
static void sema_default_untyped_literals_in_condition(AstNode *node)
{
    if (!node)
    {
        return;
    }

    if (sema_is_untyped_numeric_literal(node))
    {
        Token *tok = node->token;
        if (tok && tok->kind == TOKEN_LIT_FLOAT)
        {
            node->type = type_get_primitive(TYPE_F64);
        }
        else
        {
            node->type = type_get_primitive(TYPE_I64);
        }
        return;
    }

    switch (node->kind)
    {
    case AST_EXPR_UNARY:
        sema_default_untyped_literals_in_condition(node->unary_expr.expr);
        break;

    case AST_EXPR_BINARY:
        sema_default_untyped_literals_in_condition(node->binary_expr.left);
        sema_default_untyped_literals_in_condition(node->binary_expr.right);
        break;

    default:
        break;
    }
}

static bool sema_try_infer_numeric_literal(AstNode *node, Type *target)
{
    if (!target || !type_is_numeric(target))
    {
        return false;
    }

    if (!sema_is_untyped_numeric_literal(node))
    {
        return false;
    }

    node->type = target;
    return true;
}
static Type *sema_resolve_type(Sema *sema, AstNode *type_node);

static bool sema_expr_is_lvalue(Sema *sema, AstNode *node)
{
    (void)sema;
    if (!node)
    {
        return false;
    }

    switch (node->kind)
    {
    case AST_EXPR_IDENT:
        // only variables (var/val) and parameters denote assignable storage
        return node->symbol && (node->symbol->kind == SYMBOL_VARIABLE || node->symbol->kind == SYMBOL_PARAMETER);

    case AST_EXPR_UNARY:
        // dereference yields an lvalue
        if (node->unary_expr.op == TOKEN_AT)
        {
            Type *t = node->unary_expr.expr ? node->unary_expr.expr->type : NULL;
            return t && t->kind == TYPE_POINTER;
        }
        return false;

    case AST_EXPR_FIELD:
    {
        // method references are not lvalues
        if (node->field_expr.is_method)
        {
            return false;
        }

        Type *obj_type = node->field_expr.object ? node->field_expr.object->type : NULL;
        if (!obj_type)
        {
            return false;
        }

        // field through a pointer denotes storage even if the pointer expression is an rvalue
        if (obj_type->kind == TYPE_POINTER)
        {
            return true;
        }

        // field of a value is an lvalue only if the object is an lvalue
        return sema_expr_is_lvalue(sema, node->field_expr.object);
    }

    case AST_EXPR_INDEX:
    {
        Type *arr_type = node->index_expr.array ? node->index_expr.array->type : NULL;
        if (!arr_type)
        {
            return false;
        }

        // indexing through a pointer denotes storage even if the pointer expression is an rvalue
        if (arr_type->kind == TYPE_POINTER)
        {
            return true;
        }

        // indexing into an array value is an lvalue only if the array expression is an lvalue
        if (arr_type->kind == TYPE_ARRAY)
        {
            return sema_expr_is_lvalue(sema, node->index_expr.array);
        }

        return false;
    }

    default:
        return false;
    }
}

static bool sema_expr_is_mutable_lvalue(Sema *sema, AstNode *node)
{
    if (!sema_expr_is_lvalue(sema, node))
    {
        return false;
    }

    switch (node->kind)
    {
    case AST_EXPR_IDENT:
        // only 'var' bindings are mutable; parameters are treated as immutable
        return node->symbol && node->symbol->kind == SYMBOL_VARIABLE && node->symbol->is_mutable;

    case AST_EXPR_UNARY:
        if (node->unary_expr.op == TOKEN_AT)
        {
            Type *t = node->unary_expr.expr ? node->unary_expr.expr->type : NULL;
            return t && t->kind == TYPE_POINTER && !t->pointer.is_const;
        }
        return false;

    case AST_EXPR_FIELD:
    {
        if (node->field_expr.is_method)
        {
            return false;
        }

        Type *obj_type = node->field_expr.object ? node->field_expr.object->type : NULL;
        if (!obj_type)
        {
            return false;
        }

        if (obj_type->kind == TYPE_POINTER)
        {
            return !obj_type->pointer.is_const;
        }

        return sema_expr_is_mutable_lvalue(sema, node->field_expr.object);
    }

    case AST_EXPR_INDEX:
    {
        Type *arr_type = node->index_expr.array ? node->index_expr.array->type : NULL;
        if (!arr_type)
        {
            return false;
        }

        if (arr_type->kind == TYPE_POINTER)
        {
            return !arr_type->pointer.is_const;
        }

        if (arr_type->kind == TYPE_ARRAY)
        {
            return sema_expr_is_mutable_lvalue(sema, node->index_expr.array);
        }

        return false;
    }

    default:
        return false;
    }
}
static int  sema_collect_symbols(Sema *sema, AstNode *node);
static int  sema_analyze_use(Sema *sema, AstNode *node);
static void sema_maybe_analyze_symbol_decl_in_module(Sema *sema, SemaModule *mod, Symbol *sym);

// helper: extract base type name from a type AST node (unwraps pointers)
static const char *sema_get_receiver_type_name(AstNode *receiver_type)
{
    if (!receiver_type)
    {
        return NULL;
    }

    // unwrap pointer types to get base type
    AstNode *base = receiver_type;
    while (base && base->kind == AST_TYPE_PTR)
    {
        base = base->type_ptr.base;
    }

    if (base && base->kind == AST_TYPE_NAME)
    {
        return base->type_name.name;
    }

    return NULL;
}

// helper: build qualified method name "ReceiverType.methodName"
static char *sema_build_method_symbol_name(const char *receiver_type_name, const char *method_name)
{
    if (!receiver_type_name || !method_name)
    {
        return NULL;
    }

    size_t len = strlen(receiver_type_name) + 1 + strlen(method_name) + 1;
    char  *buf = malloc(len);
    if (!buf)
    {
        return NULL;
    }

    snprintf(buf, len, "%s.%s", receiver_type_name, method_name);
    return buf;
}

// collect symbols from a statement (first pass - no body analysis)
static int sema_collect_fun_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }

    // for methods, use qualified name "ReceiverType.methodName" to allow
    // same-named methods on different types
    const char *symbol_key  = node->fun_stmt.name;
    char       *method_key  = NULL;
    bool        is_method   = node->fun_stmt.is_method && node->fun_stmt.method_receiver;

    if (is_method)
    {
        const char *receiver_name = sema_get_receiver_type_name(node->fun_stmt.method_receiver);
        if (receiver_name)
        {
            method_key = sema_build_method_symbol_name(receiver_name, node->fun_stmt.name);
            if (method_key)
            {
                symbol_key = method_key;
            }
        }
    }

    // check if symbol already exists (from forward declaration or previous pass)
    Symbol *existing = symbol_table_lookup_local(sema->current_table, symbol_key);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            free(method_key);
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate function declaration");
        free(method_key);
        return -1;
    }

    // create symbol for function - use qualified name for methods as internal key
    // but keep original name for display/export
    Symbol *sym = symbol_create(symbol_key, SYMBOL_FUNCTION, sema->module_path);
    free(method_key); // symbol_create copies the name
    if (!sym)
    {
        return -1;
    }

    sym->is_public = node->fun_stmt.is_public;
    sym->decl      = node;

    // check for generics - defer full resolution
    if (node->fun_stmt.generics && node->fun_stmt.generics->count > 0)
    {
        sym->is_generic = true;
    }

    // add function to symbol table (type will be resolved in analysis pass)
    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate function declaration");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

static int sema_collect_rec_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_REC)
    {
        return -1;
    }

    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->rec_stmt.name);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate type definition");
        return -1;
    }

    Symbol *sym = symbol_create(node->rec_stmt.name, SYMBOL_TYPE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    sym->is_public = node->rec_stmt.is_public;
    sym->decl      = node;

    if (node->rec_stmt.generics && node->rec_stmt.generics->count > 0)
    {
        sym->is_generic = true;
    }

    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate type definition");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

static int sema_collect_uni_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_UNI)
    {
        return -1;
    }

    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->uni_stmt.name);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate type definition");
        return -1;
    }

    Symbol *sym = symbol_create(node->uni_stmt.name, SYMBOL_TYPE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    sym->is_public = node->uni_stmt.is_public;
    sym->decl      = node;

    if (node->uni_stmt.generics && node->uni_stmt.generics->count > 0)
    {
        sym->is_generic = true;
    }

    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate type definition");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

static int sema_collect_def_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_DEF)
    {
        return -1;
    }

    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->def_stmt.name);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate type definition");
        return -1;
    }

    Symbol *sym = symbol_create(node->def_stmt.name, SYMBOL_TYPE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    sym->is_public = node->def_stmt.is_public;
    sym->decl      = node;

    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate type definition");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

static int sema_collect_var_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_VAL && node->kind != AST_STMT_VAR)
    {
        return -1;
    }

    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->var_stmt.name);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate variable declaration");
        return -1;
    }

    Symbol *sym = symbol_create(node->var_stmt.name, SYMBOL_VARIABLE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    sym->is_public  = node->var_stmt.is_public;
    sym->is_mutable = (node->kind == AST_STMT_VAR);
    sym->decl       = node;

    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate variable declaration");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

static int sema_collect_ext_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_EXT)
    {
        return -1;
    }

    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->ext_stmt.name);
    if (existing)
    {
        // if same node, already collected (multi-pass) - just link it
        if (existing->decl == node)
        {
            node->symbol = existing;
            return 0;
        }
        // different declaration with same name - duplicate error
        sema_error(sema, node->token, "duplicate external declaration");
        return -1;
    }

    Symbol *sym = symbol_create(node->ext_stmt.name, SYMBOL_FUNCTION, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    sym->is_public = node->ext_stmt.is_public;
    sym->decl      = node;

    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate external declaration");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    return 0;
}

// collect all top-level symbols from a list of statements
static int sema_collect_symbols(Sema *sema, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

    switch (node->kind)
    {
    case AST_STMT_FUN:
        return sema_collect_fun_symbol(sema, node);

    case AST_STMT_REC:
        return sema_collect_rec_symbol(sema, node);

    case AST_STMT_UNI:
        return sema_collect_uni_symbol(sema, node);

    case AST_STMT_DEF:
        return sema_collect_def_symbol(sema, node);

    case AST_STMT_EXT:
        return sema_collect_ext_symbol(sema, node);

    case AST_STMT_VAL:
    case AST_STMT_VAR:
        return sema_collect_var_symbol(sema, node);

    case AST_STMT_USE:
        // process use statements during collection to make imported symbols available
        return sema_analyze_use(sema, node);

    case AST_COMPTIME:
    case AST_STMT_COMPTIME_IF:
    case AST_STMT_COMPTIME_OR:
        // comptime statements are processed in the analysis pass
        return 0;

    default:
        return 0;
    }
}

static int sema_analyze_ext(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_EXT)
    {
        return -1;
    }

    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->ext_stmt.name);
        if (!sym)
        {
            sym = symbol_create(node->ext_stmt.name, SYMBOL_FUNCTION, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public = node->ext_stmt.is_public;
            sym->decl      = node;
            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate external declaration");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // resolve external function type
    Type *t = sema_resolve_type(sema, node->ext_stmt.type);
    if (!t || t->kind != TYPE_FUNCTION)
    {
        sema_error(sema, node->token, "external declaration requires a function type");
        return -1;
    }

    sym->type  = t;
    node->type = t;

    // set linkage/export name to the underlying symbol name
    if (node->ext_stmt.symbol)
    {
        if (sym->export_name)
        {
            free(sym->export_name);
        }
        sym->export_name = strdup(node->ext_stmt.symbol);
    }

    return 0;
}

// analyze function declaration (second pass - full analysis)
static int sema_analyze_fun(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }

    // get symbol from collection pass (or create if not collected)
    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->fun_stmt.name);
        if (!sym)
        {
            // symbol wasn't collected, create it now
            sym = symbol_create(node->fun_stmt.name, SYMBOL_FUNCTION, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public = node->fun_stmt.is_public;
            sym->decl      = node;

            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate function declaration");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // skip if already analyzed (e.g., via on-demand analysis)
    if (sym->type)
    {
        return 0;
    }

    // check for explicit generics - defer full analysis
    if (node->fun_stmt.generics && node->fun_stmt.generics->count > 0)
    {
        sym->is_generic = true;
        return 0;
    }

    // for methods on generic types, extract type parameters from receiver
    // e.g., fun (this: Option[T]) unwrap() T - the T comes from Option's generic param
    SymbolTable *method_scope = NULL;
    if (node->fun_stmt.is_method && node->fun_stmt.method_receiver)
    {
        AstNode *receiver_type = node->fun_stmt.method_receiver;

        // handle pointer receivers: unwrap *T or &T to get base type
        while (receiver_type->kind == AST_TYPE_PTR)
        {
            receiver_type = receiver_type->type_ptr.base;
        }

        if (receiver_type->kind == AST_TYPE_NAME && receiver_type->type_name.generic_args && receiver_type->type_name.generic_args->count > 0)
        {
            // look up the generic type
            Symbol *type_sym = symbol_table_lookup(sema->current_table, receiver_type->type_name.name);
            if (type_sym && type_sym->is_generic && type_sym->decl)
            {
                AstList *formal_params = NULL;
                if (type_sym->decl->kind == AST_STMT_REC)
                {
                    formal_params = type_sym->decl->rec_stmt.generics;
                }
                else if (type_sym->decl->kind == AST_STMT_UNI)
                {
                    formal_params = type_sym->decl->uni_stmt.generics;
                }

                if (formal_params && formal_params->count == receiver_type->type_name.generic_args->count)
                {
                    // create scope with type parameter bindings
                    method_scope        = symbol_table_create(sema->current_table);
                    sema->current_table = method_scope;

                    // mark method as implicitly generic
                    sym->is_generic = true;

                    for (int i = 0; i < formal_params->count; i++)
                    {
                        AstNode *formal = formal_params->items[i];
                        AstNode *actual = receiver_type->type_name.generic_args->items[i];

                        if (formal->kind == AST_TYPE_PARAM && actual->kind == AST_TYPE_NAME)
                        {
                            // bind the actual type name (e.g., "T") as a type parameter
                            // in this scope, so when we resolve "T" in return types/params,
                            // it resolves to this type parameter symbol
                            Symbol *param_sym           = symbol_create(actual->type_name.name, SYMBOL_TYPE, sema->module_path);
                            param_sym->is_generic_param = true;
                            // store the formal parameter name for instantiation lookup
                            param_sym->generic_param_name = strdup(formal->type_param.name);
                            symbol_table_insert(method_scope, param_sym);
                        }
                    }
                }
            }
        }
    }

    // if method is now marked generic, defer full analysis
    if (sym->is_generic)
    {
        if (method_scope)
        {
            sema->current_table = method_scope->parent;
        }
        return 0;
    }

    // resolve return type
    Type *ret_type = NULL;
    if (node->fun_stmt.return_type)
    {
        ret_type = sema_resolve_type(sema, node->fun_stmt.return_type);
        if (!ret_type)
        {
            sema_error(sema, node->fun_stmt.return_type->token, "failed to resolve return type");
            return -1;
        }
    }

    Type **param_types = NULL;
    int    param_count = 0;
    int    fixed_count = node->fun_stmt.params ? node->fun_stmt.params->count : 0;

    if (fixed_count > 0 || node->fun_stmt.is_variadic)
    {
        int alloc_count = fixed_count + (node->fun_stmt.is_variadic ? 1 : 0);
        param_types     = malloc(sizeof(Type *) * alloc_count);
        if (!param_types)
        {
            return -1;
        }

        for (int i = 0; i < fixed_count; i++)
        {
            AstNode *param = node->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Type *pt = NULL;
                if (param->param_stmt.type)
                {
                    pt = sema_resolve_type(sema, param->param_stmt.type);
                    if (!pt)
                    {
                        sema_error(sema, param->token, "failed to resolve parameter type");
                        free(param_types);
                        return -1;
                    }
                }
                else
                {
                    sema_error(sema, param->token, "parameter must have type");
                    free(param_types);
                    return -1;
                }
                param_types[i] = pt;
                param->type    = pt;
            }
        }

        if (node->fun_stmt.is_variadic)
        {
            param_types[fixed_count] = NULL; // sentinel for variadic
            param_count              = alloc_count;
        }
        else
        {
            param_count = fixed_count;
        }
    }

    sym->type = type_create_function(ret_type, param_types, param_count);

    // analyze body
    if (node->fun_stmt.body)
    {
        // create new scope for function body
        SymbolTable *prev_table = sema->current_table;
        sema->current_table     = symbol_table_create(prev_table);

        Type *prev_return_type             = sema->current_function_return_type;
        bool  prev_is_variadic             = sema->current_function_is_variadic;
        sema->current_function_return_type = sym->type ? sym->type->function.return_type : NULL;
        sema->current_function_is_variadic = node->fun_stmt.is_variadic;

        // add parameters to scope
        if (node->fun_stmt.params)
        {
            for (int i = 0; i < node->fun_stmt.params->count; i++)
            {
                AstNode *param = node->fun_stmt.params->items[i];
                if (param->kind == AST_STMT_PARAM)
                {
                    Symbol *param_sym = symbol_create(param->param_stmt.name, SYMBOL_VARIABLE, sema->module_path);
                    param_sym->type   = param->type;

                    symbol_table_insert(sema->current_table, param_sym);
                    param->symbol   = param_sym;
                    param_sym->decl = param;
                }
            }
        }

        sema_analyze_stmt(sema, node->fun_stmt.body);

        sema->current_table                = prev_table;
        sema->current_function_return_type = prev_return_type;
        sema->current_function_is_variadic = prev_is_variadic;
    }

    return 0;
}

// analyze variable/value declaration
static int sema_analyze_var(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_VAR && node->kind != AST_STMT_VAL)
    {
        return -1;
    }

    // get symbol from collection pass (or create if not collected)
    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->var_stmt.name);
        if (!sym)
        {
            sym = symbol_create(node->var_stmt.name, SYMBOL_VARIABLE, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public  = node->var_stmt.is_public;
            sym->is_mutable = (node->kind == AST_STMT_VAR);
            sym->decl       = node;

            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate variable declaration");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // resolve explicit type if provided
    if (node->var_stmt.type)
    {
        Type *var_type = sema_resolve_type(sema, node->var_stmt.type);
        if (!var_type)
        {
            sema_error(sema, node->token, "failed to resolve variable type");
            return -1;
        }
        sym->type = var_type;
    }

    // analyze initializer
    if (node->var_stmt.init)
    {
        if (sema_analyze_expr(sema, node->var_stmt.init) < 0)
        {
            return -1;
        }

        // infer type from initializer if not explicit
        if (!sym->type && node->var_stmt.init->type)
        {
            sym->type = node->var_stmt.init->type;
        }
        else if (!sym->type && sema_is_untyped_numeric_literal(node->var_stmt.init))
        {
            sema_error(sema, node->var_stmt.init->token, "could not infer type of numeric literal for variable");
            return -1;
        }
        else if (!sym->type)
        {
            sema_error(sema, node->token, "could not infer variable type");
            return -1;
        }

        // explicit variable type exists — align untyped numeric initializer with it
        if (sym->type)
        {
            sema_infer_numeric_expr(node->var_stmt.init, sym->type);

            if (!node->var_stmt.init->type && sema_is_untyped_numeric_literal(node->var_stmt.init) && type_is_numeric(sym->type))
            {
                node->var_stmt.init->type = sym->type;
            }
        }

        // check type compatibility if both sides are now typed
        if (sym->type && node->var_stmt.init->type)
        {
            if (!type_can_assign_to(node->var_stmt.init->type, sym->type))
            {
                sema_error(sema, node->token, "type mismatch: cannot assign type to variable");
                return -1;
            }
        }

        if (!sema_check_untyped_numeric(sema, node->var_stmt.init, "could not infer type of numeric literal for variable"))
        {
            return -1;
        }
    }

    node->type = sym->type;
    return 0;
}

// forward declaration for generic type instantiation
static Type *sema_instantiate_generic_type(Sema *sema, Symbol *generic_sym, AstList *type_args);

static AstNode *sema_type_node_from_type(Type *t)
{
    if (!t)
    {
        return NULL;
    }

    AstNode *node = calloc(1, sizeof(AstNode));
    if (!node)
    {
        return NULL;
    }

    switch (t->kind)
    {
    case TYPE_I8:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("i8");
        break;
    case TYPE_I16:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("i16");
        break;
    case TYPE_I32:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("i32");
        break;
    case TYPE_I64:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("i64");
        break;
    case TYPE_U8:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("u8");
        break;
    case TYPE_U16:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("u16");
        break;
    case TYPE_U32:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("u32");
        break;
    case TYPE_U64:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("u64");
        break;
    case TYPE_F32:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("f32");
        break;
    case TYPE_F64:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("f64");
        break;
    case TYPE_PTR:
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup("ptr");
        break;

    case TYPE_POINTER:
    {
        AstNode *base = sema_type_node_from_type(t->pointer.base);
        if (!base)
        {
            free(node);
            return NULL;
        }
        node->kind                  = AST_TYPE_PTR;
        node->type_ptr.base         = base;
        node->type_ptr.is_read_only = t->pointer.is_const;
        break;
    }

    case TYPE_STRUCT:
    {
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup(t->structure.name ? t->structure.name : "");
        break;
    }
    case TYPE_UNION:
    {
        node->kind           = AST_TYPE_NAME;
        node->type_name.name = strdup(t->union_type.name ? t->union_type.name : "");
        break;
    }

    default:
        free(node);
        return NULL;
    }

    node->type = t;
    return node;
}

// analyze record definition
static int sema_analyze_rec(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_REC)
    {
        return -1;
    }

    // get symbol from collection pass (or create if not collected)
    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->rec_stmt.name);
        if (!sym)
        {
            sym = symbol_create(node->rec_stmt.name, SYMBOL_TYPE, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public = node->rec_stmt.is_public;
            sym->decl      = node;

            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate type definition");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // if type already resolved, skip (handles re-entry during recursive type resolution)
    if (sym->type)
    {
        node->type = sym->type;
        return 0;
    }

    // check for generics - if present, defer field resolution until instantiation
    if (node->rec_stmt.generics && node->rec_stmt.generics->count > 0)
    {
        sym->is_generic = true;
        return 0;
    }

    // process fields for non-generic records
    int        field_count = node->rec_stmt.fields ? node->rec_stmt.fields->count : 0;
    TypeField *fields      = NULL;

    if (field_count > 0)
    {
        fields = malloc(sizeof(TypeField) * field_count);
        if (!fields)
        {
            return -1;
        }

        // initialize field names first
        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->rec_stmt.fields->items[i];
            if (field_node->kind != AST_STMT_FIELD)
            {
                free(fields);
                return -1;
            }
            fields[i].name   = strdup(field_node->field_stmt.name);
            fields[i].type   = NULL;
            fields[i].offset = 0;
        }
    }

    // create record type FIRST (with NULL field types) so recursive references can find it
    Type *rec_type = type_create_struct(node->rec_stmt.name, fields, field_count);
    if (!rec_type)
    {
        if (fields)
        {
            for (int i = 0; i < field_count; i++)
            {
                free(fields[i].name);
            }
            free(fields);
        }
        return -1;
    }

    // assign type to symbol BEFORE resolving field types (enables recursive types)
    sym->type  = rec_type;
    node->type = rec_type;

    // NOW resolve field types (recursive references will find sym->type)
    if (field_count > 0)
    {
        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->rec_stmt.fields->items[i];
            Type    *field_type = sema_resolve_type(sema, field_node->field_stmt.type);

            if (!field_type)
            {
                sema_error(sema, field_node->token, "failed to resolve field type");
                // don't free - type is already assigned and may be in use
                return -1;
            }

            rec_type->structure.fields[i].type = field_type;
        }

        // recalculate size and alignment now that field types are resolved
        size_t offset    = 0;
        size_t max_align = 1;
        for (int i = 0; i < field_count; i++)
        {
            Type  *ft    = rec_type->structure.fields[i].type;
            size_t align = ft ? ft->alignment : 1;
            size_t size  = ft ? ft->size : 0;

            if (align > max_align)
            {
                max_align = align;
            }

            // align offset
            if (align > 0)
            {
                offset = (offset + align - 1) & ~(align - 1);
            }

            rec_type->structure.fields[i].offset = offset;
            offset += size;
        }

        // final padding
        if (max_align > 0)
        {
            offset = (offset + max_align - 1) & ~(max_align - 1);
        }

        rec_type->size  = offset;
        rec_type->alignment = max_align;
    }

    return 0;
}

static int sema_analyze_uni(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_UNI)
    {
        return -1;
    }

    // get symbol from collection pass (or create if not collected)
    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->uni_stmt.name);
        if (!sym)
        {
            sym = symbol_create(node->uni_stmt.name, SYMBOL_TYPE, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public = node->uni_stmt.is_public;
            sym->decl      = node;

            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate type definition");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // if type already resolved, skip (handles re-entry during recursive type resolution)
    if (sym->type)
    {
        node->type = sym->type;
        return 0;
    }

    // check for generics - if present, defer field resolution until instantiation
    if (node->uni_stmt.generics && node->uni_stmt.generics->count > 0)
    {
        sym->is_generic = true;
        return 0;
    }

    // process fields for non-generic unions
    int        field_count = node->uni_stmt.fields ? node->uni_stmt.fields->count : 0;
    TypeField *fields      = NULL;

    if (field_count > 0)
    {
        fields = malloc(sizeof(TypeField) * field_count);
        if (!fields)
        {
            return -1;
        }

        // initialize field names first
        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->uni_stmt.fields->items[i];
            if (field_node->kind != AST_STMT_FIELD)
            {
                free(fields);
                return -1;
            }
            fields[i].name   = strdup(field_node->field_stmt.name);
            fields[i].type   = NULL;
            fields[i].offset = 0;
        }
    }

    // create union type FIRST (with NULL field types) so recursive references can find it
    Type *uni_type = type_create_union(node->uni_stmt.name, fields, field_count);
    if (!uni_type)
    {
        if (fields)
        {
            for (int i = 0; i < field_count; i++)
            {
                free(fields[i].name);
            }
            free(fields);
        }
        return -1;
    }

    // assign type to symbol BEFORE resolving field types (enables recursive types)
    sym->type  = uni_type;
    node->type = uni_type;

    // NOW resolve field types (recursive references will find sym->type)
    if (field_count > 0)
    {
        size_t max_size  = 0;
        size_t max_align = 1;

        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->uni_stmt.fields->items[i];
            Type    *field_type = sema_resolve_type(sema, field_node->field_stmt.type);

            if (!field_type)
            {
                sema_error(sema, field_node->token, "failed to resolve field type");
                return -1;
            }

            uni_type->union_type.fields[i].type = field_type;

            if (field_type->size > max_size)
            {
                max_size = field_type->size;
            }
            if (field_type->alignment > max_align)
            {
                max_align = field_type->alignment;
            }
        }

        // union size is size of largest field, aligned
        if (max_align > 0)
        {
            max_size = (max_size + max_align - 1) & ~(max_align - 1);
        }

        uni_type->size  = max_size + 8; // +8 for tag
        uni_type->alignment = max_align > 8 ? max_align : 8;
    }

    return 0;
}

// analyze type alias definition (def)
static int sema_analyze_def(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_DEF)
    {
        return -1;
    }

    // get symbol from collection pass (or create if not collected)
    Symbol *sym = node->symbol;
    if (!sym)
    {
        sym = symbol_table_lookup_local(sema->current_table, node->def_stmt.name);
        if (!sym)
        {
            sym = symbol_create(node->def_stmt.name, SYMBOL_TYPE, sema->module_path);
            if (!sym)
            {
                return -1;
            }
            sym->is_public = node->def_stmt.is_public;
            sym->decl      = node;

            if (symbol_table_insert(sema->current_table, sym) < 0)
            {
                sema_error(sema, node->token, "duplicate type definition");
                symbol_destroy(sym);
                return -1;
            }
        }
        node->symbol = sym;
    }

    // resolve the aliased type
    Type *aliased_type = sema_resolve_type(sema, node->def_stmt.type);
    if (!aliased_type)
    {
        sema_error(sema, node->token, "failed to resolve type in type alias");
        return -1;
    }

    // type aliases are transparent - the alias has the same type as the underlying type
    sym->type  = aliased_type;
    node->type = aliased_type;

    return 0;
}

static bool sema_eval_comptime_int(Sema *sema, AstNode *node, int64_t *out_val)
{
    if (!node)
    {
        return false;
    }

    if (node->kind == AST_COMPTIME)
    {
        if (node->comptime.value_kind == COMPTIME_INT)
        {
            *out_val = node->comptime.int_value;
            return true;
        }
        return false;
    }

    if (node->kind == AST_EXPR_LIT)
    {
        if (node->lit_expr.kind == TOKEN_LIT_INT)
        {
            *out_val = (int64_t)node->lit_expr.int_val;
            return true;
        }
        return false;
    }

    if (node->kind == AST_EXPR_BINARY)
    {
        int64_t left, right;
        if (!sema_eval_comptime_int(sema, node->binary_expr.left, &left))
        {
            return false;
        }
        if (!sema_eval_comptime_int(sema, node->binary_expr.right, &right))
        {
            return false;
        }

        switch (node->binary_expr.op)
        {
        case TOKEN_PLUS:
            *out_val = left + right;
            return true;
        case TOKEN_MINUS:
            *out_val = left - right;
            return true;
        case TOKEN_STAR:
            *out_val = left * right;
            return true;
        case TOKEN_SLASH:
            *out_val = (right != 0) ? (left / right) : 0;
            return true;
        case TOKEN_AMPERSAND:
            *out_val = left & right;
            return true;
        case TOKEN_PIPE:
            *out_val = left | right;
            return true;
        case TOKEN_CARET:
            *out_val = left ^ right;
            return true;
        case TOKEN_LESS_LESS:
            *out_val = left << right;
            return true;
        case TOKEN_GREATER_GREATER:
            *out_val = left >> right;
            return true;
        case TOKEN_EQUAL_EQUAL:
            *out_val = (left == right);
            return true;
        case TOKEN_BANG_EQUAL:
            *out_val = (left != right);
            return true;
        case TOKEN_LESS:
            *out_val = (left < right);
            return true;
        case TOKEN_GREATER:
            *out_val = (left > right);
            return true;
        case TOKEN_LESS_EQUAL:
            *out_val = (left <= right);
            return true;
        case TOKEN_GREATER_EQUAL:
            *out_val = (left >= right);
            return true;
        case TOKEN_AMPERSAND_AMPERSAND:
            *out_val = ((left != 0) && (right != 0));
            return true;
        case TOKEN_PIPE_PIPE:
            *out_val = ((left != 0) || (right != 0));
            return true;
        default:
            return false;
        }
    }

    if (node->kind == AST_EXPR_UNARY)
    {
        int64_t inner = 0;
        if (!sema_eval_comptime_int(sema, node->unary_expr.expr, &inner))
        {
            return false;
        }

        switch (node->unary_expr.op)
        {
        case TOKEN_BANG:
            *out_val = (inner == 0);
            return true;
        case TOKEN_MINUS:
            *out_val = -inner;
            return true;
        case TOKEN_TILDE:
            *out_val = ~inner;
            return true;
        default:
            return false;
        }
    }

    return false;
}

static int sema_analyze_comptime_stmt(Sema *sema, AstNode *node)
{
    AstNode *inner = node->comptime.inner;

    // handle assignment: $symbol.attr = value
    if (inner->kind == AST_EXPR_BINARY && inner->binary_expr.op == TOKEN_EQUAL)
    {
        AstNode *lhs = inner->binary_expr.left;
        AstNode *rhs = inner->binary_expr.right;

        if (sema_analyze_expr(sema, rhs) < 0)
        {
            return -1;
        }

        // rhs must be a string literal
        if (rhs->kind != AST_EXPR_LIT || rhs->lit_expr.kind != TOKEN_LIT_STRING)
        {
            sema_error(sema, rhs->token, "expected string literal for attribute value");
            return -1;
        }

        // lhs must be $symbol.attribute
        if (lhs->kind != AST_EXPR_FIELD)
        {
            sema_error(sema, lhs->token, "expected attribute access (e.g. $foo.name)");
            return -1;
        }

        AstNode *object = lhs->field_expr.object;
        char    *field  = lhs->field_expr.field;

        if (object->kind != AST_EXPR_IDENT)
        {
            sema_error(sema, object->token, "expected symbol identifier");
            return -1;
        }

        Symbol *sym = symbol_table_lookup(sema->current_table, object->ident_expr.name);
        if (!sym)
        {
            sema_error(sema, object->token, "undefined symbol");
            return -1;
        }
        if (strcmp(field, "name") == 0 || strcmp(field, "symbol") == 0)
        {
            if (sym->export_name)
            {
                free(sym->export_name);
            }
            sym->export_name = strdup(rhs->lit_expr.string_val);
        }
        else
        {
            sema_error(sema, lhs->token, "unknown attribute");
            return -1;
        }

        return 0;
    }
    else if (inner->kind == AST_EXPR_CALL)
    {
        AstNode *func = inner->call_expr.func;
        if (func->kind == AST_EXPR_IDENT)
        {
            char *name = func->ident_expr.name;
            if (strcmp(name, "error") == 0)
            {
                // $error("message")
                if (!inner->call_expr.args || inner->call_expr.args->count != 1)
                {
                    sema_error(sema, inner->token, "expected 1 argument for $error");
                    return -1;
                }

                AstNode *arg = inner->call_expr.args->items[0];
                if (sema_analyze_expr(sema, arg) < 0)
                {
                    return -1;
                }

                if (arg->kind != AST_EXPR_LIT || arg->lit_expr.kind != TOKEN_LIT_STRING)
                {
                    sema_error(sema, arg->token, "expected string literal for error message");
                    return -1;
                }

                sema_error(sema, inner->token, arg->lit_expr.string_val);
                return -1;
            }
        }
    }

    // treat bare comptime reads (e.g. $mach.os.id) as no-ops but still analyze them for validity
    return sema_analyze_expr(sema, inner);
}

// check if a module has already been loaded
static SemaModule *sema_find_module(Sema *sema, const char *module_path)
{
    if (!sema || !module_path)
    {
        return NULL;
    }

    for (SemaModule *mod = sema->modules; mod; mod = mod->next)
    {
        if (mod->module_path && strcmp(mod->module_path, module_path) == 0)
        {
            return mod;
        }
    }

    return NULL;
}

// resolve a module path to a filesystem path
// returns NULL if the module cannot be found
static char *sema_build_file_from_root(const char *base_dir, const char *rest)
{
    if (!base_dir || !rest)
    {
        return NULL;
    }

    size_t base_len = strlen(base_dir);
    size_t rest_len = strlen(rest);
    size_t path_len = base_len + 1 + rest_len + 6; // base + '/' + rest + ".mach\0"

    char *file_path = malloc(path_len);
    if (!file_path)
    {
        return NULL;
    }

    strcpy(file_path, base_dir);
    strcat(file_path, "/");

    char *dst = file_path + base_len + 1;
    for (const char *src = rest; *src; src++)
    {
        *dst++ = (*src == '.') ? '/' : *src;
    }
    *dst = '\0';

    strcat(file_path, ".mach");
    return file_path;
}

static char *sema_resolve_module_path(Sema *sema, const char *module_path)
{
    if (!sema || !module_path)
    {
        return NULL;
    }

    // 1) current project mapping: project_id -> src_root
    if (sema->project_id && sema->src_root)
    {
        size_t id_len = strlen(sema->project_id);
        if (strncmp(module_path, sema->project_id, id_len) == 0 && (module_path[id_len] == '.' || module_path[id_len] == '\0'))
        {
            const char *rest = module_path + id_len;
            if (*rest == '.')
            {
                rest++;
            }
            return sema_build_file_from_root(sema->src_root, rest);
        }
    }

    // 2) extra roots: prefix -> src_root (single-file mode via -I)
    for (int i = 0; i < sema->extra_root_count; i++)
    {
        ModuleRoot *mr = &sema->extra_roots[i];
        if (!mr->prefix || !mr->src_root)
        {
            continue;
        }

        size_t id_len = strlen(mr->prefix);
        if (strncmp(module_path, mr->prefix, id_len) == 0 && (module_path[id_len] == '.' || module_path[id_len] == '\0'))
        {
            const char *rest = module_path + id_len;
            if (*rest == '.')
            {
                rest++;
            }
            return sema_build_file_from_root(mr->src_root, rest);
        }
    }

    // 3) dependencies (mach.toml deps): dep_project_id -> dep_root/dep_name/dep_src_dir
    if (sema->deps && sema->dep_count > 0 && sema->dep_root)
    {
        for (int i = 0; i < sema->dep_count; i++)
        {
            ConfigDep *dep = sema->deps[i];
            if (!dep || !dep->name || !dep->config || !dep->config->id)
            {
                continue;
            }

            const char *dep_project_id = dep->config->id;
            size_t      dep_id_len     = strlen(dep_project_id);

            if (strncmp(module_path, dep_project_id, dep_id_len) == 0 && (module_path[dep_id_len] == '.' || module_path[dep_id_len] == '\0'))
            {
                const char *rest = module_path + dep_id_len;
                if (*rest == '.')
                {
                    rest++;
                }

                const char *dep_src_dir = dep->config->dir_src ? dep->config->dir_src : "src";

                // build base dir: dep_root/dep_name/dep_src_dir
                size_t dep_root_len = strlen(sema->dep_root);
                size_t dep_name_len = strlen(dep->name);
                size_t dep_src_len  = strlen(dep_src_dir);
                size_t base_len     = dep_root_len + 1 + dep_name_len + 1 + dep_src_len + 1;

                char *base_dir = malloc(base_len);
                if (!base_dir)
                {
                    return NULL;
                }

                strcpy(base_dir, sema->dep_root);
                strcat(base_dir, "/");
                strcat(base_dir, dep->name);
                strcat(base_dir, "/");
                strcat(base_dir, dep_src_dir);

                char *out = sema_build_file_from_root(base_dir, rest);
                free(base_dir);
                return out;
            }
        }
    }

    return NULL;
}

// load and analyze a module
static int sema_load_module(Sema *sema, const char *module_path, SemaModule **out_mod)
{
    if (!sema || !module_path || !out_mod)
    {
        return -1;
    }

    *out_mod = NULL;

    // check if already loaded
    SemaModule *cached = sema_find_module(sema, module_path);
    if (cached)
    {
        *out_mod = cached;
        return 0;
    }

    // resolve module path to file path
    char *file_path = sema_resolve_module_path(sema, module_path);
    if (!file_path)
    {
        return -1;
    }

    // read file
    FILE *f = fopen(file_path, "r");
    if (!f)
    {
        free(file_path);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = malloc(size + 1);
    if (!source)
    {
        fclose(f);
        free(file_path);
        return -1;
    }

    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    // lex and parse
    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    AstNode *ast = parser_parse_program(&parser);
    if (!ast || parser.had_error)
    {
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        free(file_path);
        return -1;
    }

    // create and cache module (keep source for error reporting)
    SemaModule *mod = calloc(1, sizeof(SemaModule));
    if (!mod)
    {
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        free(file_path);
        return -1;
    }

    mod->module_path = strdup(module_path);
    mod->file_path   = strdup(file_path);
    mod->source      = source; // take ownership
    mod->ast         = ast;
    mod->table       = symbol_table_create(NULL);
    mod->imports     = NULL;
    mod->aliases     = NULL;

    if (!mod->module_path || !mod->file_path || !mod->table)
    {
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(file_path);
        free(source);
        if (mod->module_path)
        {
            free(mod->module_path);
        }
        if (mod->file_path)
        {
            free(mod->file_path);
        }
        if (mod->table)
        {
            symbol_table_destroy(mod->table);
        }
        free(mod);
        return -1;
    }

    mod->next     = sema->modules;
    sema->modules = mod;

    // analyze the imported module
    // save current context and restore after
    SemaModule  *saved_module      = sema->current_module;
    SymbolTable *saved_table       = sema->current_table;
    const char  *saved_module_path = sema->module_path;
    char        *saved_file_path   = sema->current_file_path;
    char        *saved_source      = sema->current_source;

    sema->current_module    = mod;
    sema->current_table     = mod->table;
    sema->module_path       = mod->module_path;
    sema->current_file_path = mod->file_path;
    sema->current_source    = mod->source;

    int result = 0;
    if (ast->kind == AST_PROGRAM && ast->program.stmts)
    {
        // first pass: collect all symbols
        for (int i = 0; i < ast->program.stmts->count; i++)
        {
            sema_collect_symbols(sema, ast->program.stmts->items[i]);
        }

        // second pass: full analysis
        for (int i = 0; i < ast->program.stmts->count; i++)
        {
            if (sema_analyze_stmt(sema, ast->program.stmts->items[i]) < 0)
            {
                result = -1;
                // continue to find more errors
            }
        }
    }

    sema->current_module    = saved_module;
    sema->current_table     = saved_table;
    sema->module_path       = saved_module_path;
    sema->current_file_path = saved_file_path;
    sema->current_source    = saved_source;

    // clean up parser/lexer
    parser_dnit(&parser);
    lexer_dnit(&lexer);
    free(file_path);

    *out_mod = mod;
    return result;
}

// analyze use statement
static int sema_analyze_use(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_USE)
    {
        return -1;
    }

    const char *module_path = node->use_stmt.module_path;
    const char *alias       = node->use_stmt.alias;

    // check for NULL module_path
    if (!module_path)
    {
        sema_error(sema, node->token, "use statement has null module path");
        return -1;
    }

    // if no module resolution roots are configured, we can't resolve modules.
    // keep single-file mode permissive unless the user supplies -I/-m roots.
    if (!sema_has_module_resolution(sema))
    {
        return 0;
    }

    // load and analyze the module
    SemaModule *module = NULL;
    if (sema_load_module(sema, module_path, &module) < 0)
    {
        char errmsg[512];
        snprintf(errmsg, sizeof(errmsg), "failed to load module '%s'", module_path);
        sema_error(sema, node->token, errmsg);
        return -1;
    }

    if (!module || !sema->current_module)
    {
        return 0;
    }

    if (alias)
    {
        ModuleAlias *al = calloc(1, sizeof(ModuleAlias));
        if (!al)
        {
            return 0;
        }
        al->alias                     = strdup(alias);
        al->module                    = module;
        al->next                      = sema->current_module->aliases;
        sema->current_module->aliases = al;
        return 0;
    }

    ModuleImport *imp = calloc(1, sizeof(ModuleImport));
    if (!imp)
    {
        return 0;
    }
    imp->module                   = module;
    imp->next                     = sema->current_module->imports;
    sema->current_module->imports = imp;

    return 0;
}

static int sema_analyze_stmt(Sema *sema, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

    switch (node->kind)
    {
    case AST_COMPTIME:
        return sema_analyze_comptime_stmt(sema, node);

    case AST_STMT_COMPTIME_IF:
    case AST_STMT_COMPTIME_OR:
    {
        bool cond_val = true;
        if (node->comptime_if_stmt.cond)
        {
            if (sema_analyze_expr(sema, node->comptime_if_stmt.cond) < 0)
            {
                return -1;
            }

            int64_t val = 0;
            if (!sema_eval_comptime_int(sema, node->comptime_if_stmt.cond, &val))
            {
                sema_error(sema, node->token, "expression is not a compile-time constant");
                return -1;
            }
            cond_val = (val != 0);
        }

        if (cond_val)
        {
            node->comptime_if_stmt.taken_branch = node->comptime_if_stmt.body;
            return sema_analyze_stmt(sema, node->comptime_if_stmt.body);
        }
        else
        {
            node->comptime_if_stmt.taken_branch = node->comptime_if_stmt.stmt_or;
            if (node->comptime_if_stmt.stmt_or)
            {
                return sema_analyze_stmt(sema, node->comptime_if_stmt.stmt_or);
            }
        }
        return 0;
    }

    case AST_STMT_DEF:
        return sema_analyze_def(sema, node);

    case AST_STMT_EXT:
        return sema_analyze_ext(sema, node);

    case AST_STMT_USE:
        return sema_analyze_use(sema, node);

    case AST_STMT_REC:
        return sema_analyze_rec(sema, node);

    case AST_STMT_UNI:
        return sema_analyze_uni(sema, node);

    case AST_STMT_FUN:
        return sema_analyze_fun(sema, node);

    case AST_STMT_VAL:
    case AST_STMT_VAR:
        return sema_analyze_var(sema, node);

    case AST_STMT_BLOCK:
        if (node->block_stmt.stmts)
        {
            for (int i = 0; i < node->block_stmt.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, node->block_stmt.stmts->items[i]) < 0)
                {
                    return -1;
                }
            }
        }
        // analyze deferred statements (fin)
        if (node->block_stmt.deferred_stmts)
        {
            for (int i = 0; i < node->block_stmt.deferred_stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, node->block_stmt.deferred_stmts->items[i]) < 0)
                {
                    return -1;
                }
            }
        }
        return 0;

    case AST_STMT_RET:
        if (node->ret_stmt.expr)
        {
            if (sema_analyze_expr(sema, node->ret_stmt.expr) < 0)
            {
                return -1;
            }

            if (sema->current_function_return_type)
            {
                sema_infer_numeric_expr(node->ret_stmt.expr, sema->current_function_return_type);

                if (!sema_check_untyped_numeric(sema, node->ret_stmt.expr, "could not infer type of numeric literal in return"))
                {
                    return -1;
                }

                if (!type_can_assign_to(node->ret_stmt.expr->type, sema->current_function_return_type))
                {
                    sema_error(sema, node->ret_stmt.expr->token, "return type mismatch");
                    return -1;
                }
            }
            else
            {
                if (!sema_check_untyped_numeric(sema, node->ret_stmt.expr, "could not infer type of numeric literal in return"))
                {
                    return -1;
                }
            }

            return 0;
        }
        return 0;

    case AST_STMT_EXPR:
    {
        if (sema_analyze_expr(sema, node->expr_stmt.expr) < 0)
        {
            return -1;
        }

        if (!sema_check_untyped_numeric(sema, node->expr_stmt.expr, "could not infer type of numeric literal"))
        {
            return -1;
        }

        return 0;
    }

    case AST_STMT_IF:
        if (sema_analyze_expr(sema, node->cond_stmt.cond) < 0)
        {
            return -1;
        }
        // default any remaining untyped literals in condition to i64/f64
        sema_default_untyped_literals_in_condition(node->cond_stmt.cond);
        if (!sema_check_untyped_numeric(sema, node->cond_stmt.cond, "could not infer type of numeric literal in condition"))
        {
            return -1;
        }
        if (sema_analyze_stmt(sema, node->cond_stmt.body) < 0)
        {
            return -1;
        }
        if (node->cond_stmt.stmt_or)
        {
            return sema_analyze_stmt(sema, node->cond_stmt.stmt_or);
        }
        return 0;

    case AST_STMT_OR:
        if (node->cond_stmt.cond && sema_analyze_expr(sema, node->cond_stmt.cond) < 0)
        {
            return -1;
        }
        // default any remaining untyped literals in condition to i64/f64
        if (node->cond_stmt.cond)
        {
            sema_default_untyped_literals_in_condition(node->cond_stmt.cond);
        }
        if (node->cond_stmt.cond && !sema_check_untyped_numeric(sema, node->cond_stmt.cond, "could not infer type of numeric literal in condition"))
        {
            return -1;
        }
        if (sema_analyze_stmt(sema, node->cond_stmt.body) < 0)
        {
            return -1;
        }
        if (node->cond_stmt.stmt_or)
        {
            return sema_analyze_stmt(sema, node->cond_stmt.stmt_or);
        }
        return 0;

    case AST_STMT_FOR:
        if (node->for_stmt.cond && sema_analyze_expr(sema, node->for_stmt.cond) < 0)
        {
            return -1;
        }
        // default any remaining untyped literals in condition to i64/f64
        if (node->for_stmt.cond)
        {
            sema_default_untyped_literals_in_condition(node->for_stmt.cond);
        }
        if (node->for_stmt.cond && !sema_check_untyped_numeric(sema, node->for_stmt.cond, "could not infer type of numeric literal in loop condition"))
        {
            return -1;
        }
        return sema_analyze_stmt(sema, node->for_stmt.body);

    case AST_STMT_MASM:
        return 0;

    default:
        return 0;
    }
}

static Type *sema_resolve_type(Sema *sema, AstNode *type_node);

int sema_analyze_expr(Sema *sema, AstNode *node)
{
    if (!sema || !node)
    {
        return -1;
    }

    switch (node->kind)
    {
    case AST_EXPR_LIT:
        // literals have types based on suffix if provided; otherwise they must
        // be inferred from context. If inference fails by the time the
        // expression is fully processed, a semantic error is emitted by
        // callers.
        if (node->token)
        {
            switch (node->token->kind)
            {
            case TOKEN_LIT_INT:
            case TOKEN_LIT_FLOAT:
            {
                if (node->token->type_suffix)
                {
                    if (strcmp(node->token->type_suffix, "u8") == 0)
                    {
                        node->type = type_get_primitive(TYPE_U8);
                    }
                    else if (strcmp(node->token->type_suffix, "u16") == 0)
                    {
                        node->type = type_get_primitive(TYPE_U16);
                    }
                    else if (strcmp(node->token->type_suffix, "u32") == 0)
                    {
                        node->type = type_get_primitive(TYPE_U32);
                    }
                    else if (strcmp(node->token->type_suffix, "u64") == 0)
                    {
                        node->type = type_get_primitive(TYPE_U64);
                    }
                    else if (strcmp(node->token->type_suffix, "i8") == 0)
                    {
                        node->type = type_get_primitive(TYPE_I8);
                    }
                    else if (strcmp(node->token->type_suffix, "i16") == 0)
                    {
                        node->type = type_get_primitive(TYPE_I16);
                    }
                    else if (strcmp(node->token->type_suffix, "i32") == 0)
                    {
                        node->type = type_get_primitive(TYPE_I32);
                    }
                    else if (strcmp(node->token->type_suffix, "i64") == 0)
                    {
                        node->type = type_get_primitive(TYPE_I64);
                    }
                    else if (strcmp(node->token->type_suffix, "f32") == 0)
                    {
                        node->type = type_get_primitive(TYPE_F32);
                    }
                    else if (strcmp(node->token->type_suffix, "f64") == 0)
                    {
                        node->type = type_get_primitive(TYPE_F64);
                    }
                    else
                    {
                        sema_error(sema, node->token, "invalid type suffix for numeric literal");
                        return -1;
                    }
                }
                break;
            }
            case TOKEN_LIT_CHAR:
                // chars are u8 per language docs
                node->type = type_get_primitive(TYPE_U8);
                break;
            case TOKEN_LIT_STRING:
                // simplified: string is pointer to u8
                node->type = type_create_pointer(type_get_primitive(TYPE_U8), false);
                break;
            default:
                break;
            }
        }
        return 0;

    case AST_EXPR_IDENT:
    {
        // look up identifier in local scopes
        Symbol     *sym    = symbol_table_lookup(sema->current_table, node->ident_expr.name);
        SemaModule *origin = NULL;

        // if not found locally, check unaliased imports of the current module
        if (!sym && sema->current_module)
        {
            for (ModuleImport *imp = sema->current_module->imports; imp; imp = imp->next)
            {
                if (!imp->module || !imp->module->table)
                {
                    continue;
                }

                Symbol *cand = symbol_table_lookup_local(imp->module->table, node->ident_expr.name);
                if (cand && cand->is_public)
                {
                    sym    = cand;
                    origin = imp->module;
                    break;
                }
            }
        }

        if (!sym)
        {
            sema_error(sema, node->token, "undefined identifier");
            return -1;
        }

        if (!origin)
        {
            origin = sema_find_module(sema, sym->module_path);
        }

        // if symbol was collected but not yet analyzed, analyze its declaration now
        if (!sym->type && sym->decl)
        {
            SemaModule  *saved_module      = sema->current_module;
            SymbolTable *saved_table       = sema->current_table;
            const char  *saved_module_path = sema->module_path;
            char        *saved_file_path   = sema->current_file_path;
            char        *saved_source      = sema->current_source;
            Type        *saved_return_type = sema->current_function_return_type;

            if (origin)
            {
                sema->current_module    = origin;
                sema->current_table     = origin->table;
                sema->module_path       = origin->module_path;
                sema->current_file_path = origin->file_path;
                sema->current_source    = origin->source;
            }

            if (sema_analyze_stmt(sema, sym->decl) < 0)
            {
                sema->current_module               = saved_module;
                sema->current_table                = saved_table;
                sema->module_path                  = saved_module_path;
                sema->current_file_path            = saved_file_path;
                sema->current_source               = saved_source;
                sema->current_function_return_type = saved_return_type;
                return -1;
            }

            sema->current_module               = saved_module;
            sema->current_table                = saved_table;
            sema->module_path                  = saved_module_path;
            sema->current_file_path            = saved_file_path;
            sema->current_source               = saved_source;
            sema->current_function_return_type = saved_return_type;
        }

        node->symbol = sym;
        node->type   = sym->type;
        return 0;
    }

    case AST_EXPR_BINARY:
        if (sema_analyze_expr(sema, node->binary_expr.left) < 0)
        {
            return -1;
        }
        if (sema_analyze_expr(sema, node->binary_expr.right) < 0)
        {
            return -1;
        }
        // attempt to align untyped numeric literals with their counterpart
        if (type_is_numeric(node->binary_expr.left->type))
        {
            sema_infer_numeric_expr(node->binary_expr.right, node->binary_expr.left->type);
        }
        if (type_is_numeric(node->binary_expr.right->type))
        {
            sema_infer_numeric_expr(node->binary_expr.left, node->binary_expr.right->type);
        }

        // result type depends on operator
        switch (node->binary_expr.op)
        {
        case TOKEN_EQUAL: // assignment
        {
            AstNode *lhs = node->binary_expr.left;
            AstNode *rhs = node->binary_expr.right;

            if (!sema_expr_is_lvalue(sema, lhs))
            {
                sema_error(sema, lhs->token ? lhs->token : node->token, "left-hand side of assignment must be an lvalue");
                return -1;
            }

            if (!sema_expr_is_mutable_lvalue(sema, lhs))
            {
                sema_error(sema, lhs->token ? lhs->token : node->token, "cannot assign to immutable location");
                return -1;
            }

            // infer numeric literals on rhs from lhs type
            sema_infer_numeric_expr(rhs, lhs->type);
            if (!sema_check_untyped_numeric(sema, rhs, "could not infer type of numeric literal in assignment"))
            {
                return -1;
            }

            if (!type_can_assign_to(rhs->type, lhs->type))
            {
                sema_error(sema, node->token, "type mismatch in assignment");
                return -1;
            }

            node->type = lhs->type;
            break;
        }

        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
        case TOKEN_LESS:
        case TOKEN_GREATER:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_AMPERSAND_AMPERSAND:
        case TOKEN_PIPE_PIPE:
            // comparisons/logical operators produce boolean. bool is defined in std as u8.
            node->type = type_get_primitive(TYPE_U8);
            break;

        default:
            // arithmetic/bitwise/etc: simplified rule for now.
            node->type = node->binary_expr.left->type ? node->binary_expr.left->type : node->binary_expr.right->type;
            break;
        }
        return 0;

    case AST_EXPR_UNARY:
        if (sema_analyze_expr(sema, node->unary_expr.expr) < 0)
        {
            return -1;
        }

        if (node->unary_expr.op == TOKEN_QUESTION)
        {
            // address-of: ?expr
            // operand must be an lvalue. result is *T for mutable lvalues, &T for immutable lvalues.
            AstNode *operand = node->unary_expr.expr;
            if (!sema_expr_is_lvalue(sema, operand))
            {
                sema_error(sema, node->token, "cannot take address of temporary or r-value");
                return -1;
            }

            bool is_const = !sema_expr_is_mutable_lvalue(sema, operand);
            node->type    = type_create_pointer(operand->type, is_const);
        }
        else if (node->unary_expr.op == TOKEN_AT)
        {
            // dereference: @T -> T
            Type *operand_type = node->unary_expr.expr->type;
            if (operand_type->kind != TYPE_POINTER)
            {
                sema_error(sema, node->token, "cannot dereference non-pointer type");
                return -1;
            }
            node->type = operand_type->pointer.base;
        }
        else if (node->unary_expr.op == TOKEN_BANG)
        {
            // logical not produces boolean
            node->type = type_get_primitive(TYPE_U8);
        }
        else
        {
            // other unary ops (-, ~, etc.) preserve type
            node->type = node->unary_expr.expr->type;
        }
        return 0;

    case AST_EXPR_CALL:
    {
        // runtime varargs builtins (no `$` prefix)
        //
        // va_start(*va_list)
        // va_arg[T](*va_list) -> T
        // va_end(*va_list)
        if (node->call_expr.func && node->call_expr.func->kind == AST_EXPR_IDENT)
        {
            const char *bname              = node->call_expr.func->ident_expr.name;
            bool        is_varargs_builtin = bname && (!strcmp(bname, "va_start") || !strcmp(bname, "va_end") || !strcmp(bname, "va_arg"));
            if (is_varargs_builtin)
            {
                // analyze arguments (callee need not resolve to a symbol)
                if (node->call_expr.args)
                {
                    for (int i = 0; i < node->call_expr.args->count; i++)
                    {
                        if (sema_analyze_expr(sema, node->call_expr.args->items[i]) < 0)
                        {
                            return -1;
                        }
                    }
                }

                if (!sema->current_function_is_variadic)
                {
                    sema_error(sema, node->token, "va_* builtins are only valid in variadic functions");
                    return -1;
                }

                int arg_count = node->call_expr.args ? node->call_expr.args->count : 0;
                if (arg_count != 1)
                {
                    sema_error(sema, node->token, "va_* builtins expect exactly 1 argument");
                    return -1;
                }

                AstNode *ap_expr = node->call_expr.args->items[0];
                Type    *ap_type = ap_expr ? ap_expr->type : NULL;
                Type    *va_list = type_get_builtin_va_list();
                if (!va_list)
                {
                    sema_error(sema, node->token, "failed to resolve builtin va_list type");
                    return -1;
                }

                if (!ap_type || ap_type->kind != TYPE_POINTER || ap_type->pointer.is_const || !type_equals(ap_type->pointer.base, va_list))
                {
                    sema_error(sema, node->token, "va_* builtins require a mutable pointer to va_list");
                    return -1;
                }

                if (!strcmp(bname, "va_arg"))
                {
                    if (!node->call_expr.type_args || node->call_expr.type_args->count != 1)
                    {
                        sema_error(sema, node->token, "va_arg requires exactly 1 type argument: va_arg[T](ap)");
                        return -1;
                    }

                    Type *t = sema_resolve_type(sema, node->call_expr.type_args->items[0]);
                    if (!t)
                    {
                        sema_error(sema, node->token, "failed to resolve va_arg type argument");
                        return -1;
                    }

                    node->type = t;
                    return 0;
                }

                // va_start/va_end return void
                node->type = NULL;
                return 0;
            }
        }

        // analyze function
        if (sema_analyze_expr(sema, node->call_expr.func) < 0)
        {
            return -1;
        }

        // analyze arguments
        if (node->call_expr.args)
        {
            for (int i = 0; i < node->call_expr.args->count; i++)
            {
                if (sema_analyze_expr(sema, node->call_expr.args->items[i]) < 0)
                {
                    return -1;
                }
            }
        }

        Type   *func_type = node->call_expr.func->type;
        Symbol *func_sym  = node->call_expr.func->symbol;

        if (node->call_expr.func->kind == AST_EXPR_FIELD && node->call_expr.func->field_expr.is_method)
        {
            node->call_expr.is_method_call = true;

            // get receiver and determine if we need auto-ref
            AstNode *receiver      = node->call_expr.func->field_expr.object;
            Type    *receiver_type = receiver->type;

            // check what the method expects
            Type *expected_receiver_type = NULL;
            if (func_sym && func_sym->type && func_sym->type->kind == TYPE_FUNCTION && func_sym->type->function.param_count > 0)
            {
                expected_receiver_type = func_sym->type->function.param_types[0];
            }
            else if (func_sym && func_sym->decl && func_sym->decl->fun_stmt.method_receiver)
            {
                // For cross-module methods, resolve the receiver type in the method's module context
                SemaModule *method_origin = sema_find_module(sema, func_sym->module_path);
                if (method_origin && method_origin != sema->current_module)
                {
                    SemaModule  *saved_module      = sema->current_module;
                    SymbolTable *saved_table       = sema->current_table;
                    const char  *saved_module_path = sema->module_path;
                    char        *saved_file_path   = sema->current_file_path;
                    char        *saved_source      = sema->current_source;

                    sema->current_module    = method_origin;
                    sema->current_table     = method_origin->table;
                    sema->module_path       = method_origin->module_path;
                    sema->current_file_path = method_origin->file_path;
                    sema->current_source    = method_origin->source;

                    expected_receiver_type = sema_resolve_type(sema, func_sym->decl->fun_stmt.method_receiver);

                    sema->current_module    = saved_module;
                    sema->current_table     = saved_table;
                    sema->module_path       = saved_module_path;
                    sema->current_file_path = saved_file_path;
                    sema->current_source    = saved_source;
                }
                else
                {
                    expected_receiver_type = sema_resolve_type(sema, func_sym->decl->fun_stmt.method_receiver);
                }
            }

            // auto-ref: if method expects pointer but receiver is value, wrap in address-of
            if (expected_receiver_type && expected_receiver_type->kind == TYPE_POINTER && receiver_type && receiver_type->kind != TYPE_POINTER)
            {
                if (!sema_expr_is_lvalue(sema, receiver))
                {
                    sema_error(sema, receiver->token ? receiver->token : node->token, "cannot take address of temporary or r-value");
                    return -1;
                }

                // create address-of expression: ?receiver
                AstNode *addr_of = malloc(sizeof(AstNode));
                memset(addr_of, 0, sizeof(AstNode));
                addr_of->kind            = AST_EXPR_UNARY;
                addr_of->unary_expr.op   = TOKEN_QUESTION;
                addr_of->unary_expr.expr = receiver;
                addr_of->token           = receiver->token;
                addr_of->type            = type_create_pointer(receiver_type, !sema_expr_is_mutable_lvalue(sema, receiver));
                receiver                 = addr_of;
            }
            // auto-deref: if method expects value but receiver is pointer, wrap in dereference
            else if (expected_receiver_type && expected_receiver_type->kind != TYPE_POINTER && receiver_type && receiver_type->kind == TYPE_POINTER)
            {
                // create dereference expression: @receiver
                AstNode *deref = malloc(sizeof(AstNode));
                memset(deref, 0, sizeof(AstNode));
                deref->kind            = AST_EXPR_UNARY;
                deref->unary_expr.op   = TOKEN_AT;
                deref->unary_expr.expr = receiver;
                deref->type            = receiver_type->pointer.base;
                receiver               = deref;
            }

            if (!node->call_expr.args)
            {
                node->call_expr.args           = malloc(sizeof(AstList));
                node->call_expr.args->items    = NULL;
                node->call_expr.args->count    = 0;
                node->call_expr.args->capacity = 0;
            }

            ast_list_prepend(node->call_expr.args, receiver);
        }

        if (func_sym && func_sym->is_generic)
        {

            if (!node->call_expr.type_args)
            {
                sema_error(sema, node->token, "generic function call requires type arguments");
                return -1;
            }

            Symbol *inst = sema_instantiate_generic(sema, func_sym, node->call_expr.type_args);
            if (!inst)
            {
                sema_error(sema, node->token, "failed to instantiate generic function");
                return -1;
            }

            node->call_expr.func->symbol = inst;
            node->call_expr.func->type   = inst->type;
            func_type                    = inst->type;
        }

        if (!func_type || func_type->kind != TYPE_FUNCTION)
        {
            sema_error(sema, node->token, "calling non-function type");
            return -1;
        }

        int arg_count   = node->call_expr.args ? node->call_expr.args->count : 0;
        int param_count = func_type->function.param_count;

        if (arg_count != param_count)
        {
            if (func_type->function.param_count > 0 && func_type->function.param_types[func_type->function.param_count - 1] == NULL) // variadic check
            {
                if (arg_count < param_count - 1)
                {
                    sema_error(sema, node->token, "too few arguments to function call");
                    return -1;
                }
            }
            else
            {
                sema_error(sema, node->token, "argument count mismatch");
                return -1;
            }
        }

        // check argument types
        for (int i = 0; i < arg_count; i++)
        {
            AstNode *arg = node->call_expr.args->items[i];

            // for variadic functions, stop checking fixed params
            if (i >= param_count)
            {
                break;
            }

            Type *param_type = func_type->function.param_types[i];

            // variadic sentinel check
            if (param_type == NULL)
            {
                break;
            }

            sema_infer_numeric_expr(arg, param_type);

            if (!sema_check_untyped_numeric(sema, arg, "could not infer type of numeric literal for argument"))
            {
                return -1;
            }

            if (!type_can_assign_to(arg->type, param_type))
            {
                sema_error(sema, arg->token, "argument type mismatch");
                return -1;
            }
        }

        node->type = func_type->function.return_type;
        return 0;
    }

    case AST_EXPR_FIELD:
    {
        // check if this is an aliased module access (alias.symbol) BEFORE analyzing the object
        // this prevents errors when the alias name is not in the symbol table
        if (node->field_expr.object->kind == AST_EXPR_IDENT)
        {
            const char *ident_name = node->field_expr.object->ident_expr.name;

            if (sema->current_module)
            {
                for (ModuleAlias *al = sema->current_module->aliases; al; al = al->next)
                {
                    if (!al->alias || strcmp(al->alias, ident_name) != 0)
                    {
                        continue;
                    }

                    const char *symbol_name = node->field_expr.field;
                    if (!al->module || !al->module->table)
                    {
                        sema_error(sema, node->token, "undefined symbol in aliased module");
                        return -1;
                    }

                    Symbol *sym = symbol_table_lookup_local(al->module->table, symbol_name);
                    if (!sym || !sym->is_public)
                    {
                        sema_error(sema, node->token, "undefined symbol in aliased module");
                        return -1;
                    }

                    // if symbol was collected but not yet analyzed, analyze it now under the module context
                    if (!sym->type && sym->decl)
                    {
                        SemaModule  *saved_module      = sema->current_module;
                        SymbolTable *saved_table       = sema->current_table;
                        const char  *saved_module_path = sema->module_path;
                        char        *saved_file_path   = sema->current_file_path;
                        char        *saved_source      = sema->current_source;
                        Type        *saved_return_type = sema->current_function_return_type;

                        sema->current_module    = al->module;
                        sema->current_table     = al->module->table;
                        sema->module_path       = al->module->module_path;
                        sema->current_file_path = al->module->file_path;
                        sema->current_source    = al->module->source;

                        if (sema_analyze_stmt(sema, sym->decl) < 0)
                        {
                            sema->current_module               = saved_module;
                            sema->current_table                = saved_table;
                            sema->module_path                  = saved_module_path;
                            sema->current_file_path            = saved_file_path;
                            sema->current_source               = saved_source;
                            sema->current_function_return_type = saved_return_type;
                            return -1;
                        }

                        sema->current_module               = saved_module;
                        sema->current_table                = saved_table;
                        sema->module_path                  = saved_module_path;
                        sema->current_file_path            = saved_file_path;
                        sema->current_source               = saved_source;
                        sema->current_function_return_type = saved_return_type;
                    }

                    // convert this field access into an identifier
                    node->kind            = AST_EXPR_IDENT;
                    node->ident_expr.name = strdup(symbol_name);
                    node->symbol          = sym;
                    node->type            = sym->type;
                    return 0;
                }
            }
        }

        // not an alias, analyze object normally
        if (sema_analyze_expr(sema, node->field_expr.object) < 0)
        {
            return -1;
        }

        Type *obj_type   = node->field_expr.object->type;
        Type *deref_type = obj_type;

        // auto-dereference pointer to record/union for field access
        if (obj_type->kind == TYPE_POINTER && (obj_type->pointer.base->kind == TYPE_STRUCT || obj_type->pointer.base->kind == TYPE_UNION))
        {
            deref_type = obj_type->pointer.base;
        }

        // try field access on records/unions
        if (deref_type->kind == TYPE_STRUCT || deref_type->kind == TYPE_UNION)
        {
            TypeField *fields      = (deref_type->kind == TYPE_STRUCT) ? deref_type->structure.fields : deref_type->union_type.fields;
            int        field_count = (deref_type->kind == TYPE_STRUCT) ? deref_type->structure.field_count : deref_type->union_type.field_count;

            for (int i = 0; i < field_count; i++)
            {
                if (strcmp(fields[i].name, node->field_expr.field) == 0)
                {
                    node->type = fields[i].type;
                    return 0;
                }
            }
        }

        // look for method - methods can be defined on any type including pointers.
        // methods are stored with qualified names "ReceiverType.methodName" to allow
        // same-named methods on different types.
        Symbol     *method        = NULL;
        SemaModule *method_origin = NULL;

        // get the base type name for the object to build qualified method name
        const char *obj_type_name = NULL;
        const char *generic_base_name = NULL; // for instantiated generic types like OptionIi64E -> Option
        Type       *base_type     = obj_type;
        if (base_type->kind == TYPE_POINTER)
        {
            base_type = base_type->pointer.base;
        }
        if (base_type->kind == TYPE_STRUCT && base_type->structure.name)
        {
            obj_type_name = base_type->structure.name;
            // check if this is an instantiated generic type (has generic_args)
            // if so, extract base name by finding 'I' separator in mangled name
            if (base_type->structure.generic_arg_count > 0)
            {
                const char *i_pos = strchr(base_type->structure.name, 'I');
                if (i_pos && i_pos != base_type->structure.name)
                {
                    // extract base name (e.g., "Option" from "OptionIi64E")
                    static char generic_base_buf[256];
                    size_t len = i_pos - base_type->structure.name;
                    if (len < sizeof(generic_base_buf))
                    {
                        strncpy(generic_base_buf, base_type->structure.name, len);
                        generic_base_buf[len] = '\0';
                        generic_base_name = generic_base_buf;
                    }
                }
            }
        }
        else if (base_type->kind == TYPE_UNION && base_type->union_type.name)
        {
            obj_type_name = base_type->union_type.name;
            // check if this is an instantiated generic type
            if (base_type->union_type.generic_arg_count > 0)
            {
                const char *i_pos = strchr(base_type->union_type.name, 'I');
                if (i_pos && i_pos != base_type->union_type.name)
                {
                    static char generic_base_buf[256];
                    size_t len = i_pos - base_type->union_type.name;
                    if (len < sizeof(generic_base_buf))
                    {
                        strncpy(generic_base_buf, base_type->union_type.name, len);
                        generic_base_buf[len] = '\0';
                        generic_base_name = generic_base_buf;
                    }
                }
            }
        }

        // try qualified lookup first (ReceiverType.methodName) - works for structs/unions
        char *qualified_name = NULL;
        char *generic_qualified_name = NULL;
        if (obj_type_name)
        {
            qualified_name = sema_build_method_symbol_name(obj_type_name, node->field_expr.field);
            if (qualified_name)
            {
                method = symbol_table_lookup(sema->current_table, qualified_name);
            }
        }
        // for instantiated generic types, also try with base generic name
        if (!method && generic_base_name)
        {
            generic_qualified_name = sema_build_method_symbol_name(generic_base_name, node->field_expr.field);
            if (generic_qualified_name)
            {
                method = symbol_table_lookup(sema->current_table, generic_qualified_name);
            }
        }

        // ignore non-method symbols that shadow method names
        if (method && (method->kind != SYMBOL_FUNCTION || !method->decl || method->decl->kind != AST_STMT_FUN || !method->decl->fun_stmt.is_method))
        {
            method = NULL;
        }

        // if qualified lookup failed (e.g., for type aliases like str), iterate through
        // all methods with matching name suffix and check receiver type compatibility
        // search through all scopes (current and parents)
        if (!method)
        {
            for (SymbolTable *scope = sema->current_table; scope && !method; scope = scope->parent)
            {
                Symbol *cand = NULL;
                while ((cand = symbol_table_lookup_method_next(scope, node->field_expr.field, cand)) != NULL)
                {
                    if (cand->kind != SYMBOL_FUNCTION || !cand->decl || cand->decl->kind != AST_STMT_FUN || !cand->decl->fun_stmt.is_method)
                    {
                        continue;
                    }

                    // check if receiver type matches - need to analyze the method first if not done
                    if (!cand->type && cand->decl)
                    {
                        SemaModule *cand_origin = sema_find_module(sema, cand->module_path);
                        sema_maybe_analyze_symbol_decl_in_module(sema, cand_origin, cand);
                    }

                    // get receiver type from method's function type
                    Type *cand_receiver_type = NULL;
                    if (cand->type && cand->type->kind == TYPE_FUNCTION && cand->type->function.param_count > 0)
                    {
                        cand_receiver_type = cand->type->function.param_types[0];
                    }
                    else
                    {
                        cand_receiver_type = sema_resolve_type(sema, cand->decl->fun_stmt.method_receiver);
                    }

                    if (cand_receiver_type)
                    {
                        // check if receiver type matches object type
                        if (type_can_assign_to(obj_type, cand_receiver_type))
                        {
                            method = cand;
                            break;
                        }

                        // also check base types for pointer receivers (auto-ref/deref)
                        Type *cand_base = cand_receiver_type;
                        if (cand_base->kind == TYPE_POINTER)
                        {
                            cand_base = cand_base->pointer.base;
                        }

                        Type *obj_base = obj_type;
                        if (obj_base->kind == TYPE_POINTER)
                        {
                            obj_base = obj_base->pointer.base;
                        }

                        if (type_equals(cand_base, obj_base))
                        {
                            method = cand;
                            break;
                        }
                    }
                }
            }
        }

        // fallback to unqualified lookup for backwards compatibility (non-method functions, etc.)
        if (!method)
        {
            method = symbol_table_lookup(sema->current_table, node->field_expr.field);
            if (method && (method->kind != SYMBOL_FUNCTION || !method->decl || method->decl->kind != AST_STMT_FUN || !method->decl->fun_stmt.is_method))
            {
                method = NULL;
            }
        }

        if (!method && sema->current_module)
        {
            for (ModuleImport *imp = sema->current_module->imports; imp; imp = imp->next)
            {
                if (!imp->module || !imp->module->table)
                {
                    continue;
                }

                // try qualified lookup in imported module
                Symbol *cand = NULL;
                if (qualified_name)
                {
                    cand = symbol_table_lookup_local(imp->module->table, qualified_name);
                }
                // for instantiated generic types, also try with base generic name
                if (!cand && generic_qualified_name)
                {
                    cand = symbol_table_lookup_local(imp->module->table, generic_qualified_name);
                }
                // if qualified lookup failed, iterate through method candidates
                if (!cand)
                {
                    Symbol *iter = NULL;
                    while ((iter = symbol_table_lookup_method_next(imp->module->table, node->field_expr.field, iter)) != NULL)
                    {
                        if (iter->kind != SYMBOL_FUNCTION || !iter->is_public || !iter->decl || iter->decl->kind != AST_STMT_FUN || !iter->decl->fun_stmt.is_method)
                        {
                            continue;
                        }

                        // analyze if needed
                        if (!iter->type && iter->decl)
                        {
                            sema_maybe_analyze_symbol_decl_in_module(sema, imp->module, iter);
                        }

                        Type *iter_receiver_type = NULL;
                        if (iter->type && iter->type->kind == TYPE_FUNCTION && iter->type->function.param_count > 0)
                        {
                            iter_receiver_type = iter->type->function.param_types[0];
                        }

                        if (iter_receiver_type)
                        {
                            if (type_can_assign_to(obj_type, iter_receiver_type))
                            {
                                cand = iter;
                                break;
                            }

                            Type *iter_base = iter_receiver_type;
                            if (iter_base->kind == TYPE_POINTER)
                            {
                                iter_base = iter_base->pointer.base;
                            }
                            Type *obj_base = obj_type;
                            if (obj_base->kind == TYPE_POINTER)
                            {
                                obj_base = obj_base->pointer.base;
                            }
                            if (type_equals(iter_base, obj_base))
                            {
                                cand = iter;
                                break;
                            }
                        }
                    }
                }
                if (!cand)
                {
                    cand = symbol_table_lookup_local(imp->module->table, node->field_expr.field);
                }
                if (!cand || cand->kind != SYMBOL_FUNCTION || !cand->is_public || !cand->decl || cand->decl->kind != AST_STMT_FUN || !cand->decl->fun_stmt.is_method)
                {
                    continue;
                }

                method        = cand;
                method_origin = imp->module;
                break;
            }
        }

        if (!method && sema->current_module)
        {
            for (ModuleAlias *al = sema->current_module->aliases; al; al = al->next)
            {
                if (!al->module || !al->module->table)
                {
                    continue;
                }

                // try qualified lookup in aliased module
                Symbol *cand = NULL;
                if (qualified_name)
                {
                    cand = symbol_table_lookup_local(al->module->table, qualified_name);
                }
                // for instantiated generic types, also try with base generic name
                if (!cand && generic_qualified_name)
                {
                    cand = symbol_table_lookup_local(al->module->table, generic_qualified_name);
                }
                // if qualified lookup failed, iterate through method candidates
                if (!cand)
                {
                    Symbol *iter = NULL;
                    while ((iter = symbol_table_lookup_method_next(al->module->table, node->field_expr.field, iter)) != NULL)
                    {
                        if (iter->kind != SYMBOL_FUNCTION || !iter->is_public || !iter->decl || iter->decl->kind != AST_STMT_FUN || !iter->decl->fun_stmt.is_method)
                        {
                            continue;
                        }

                        // analyze if needed
                        if (!iter->type && iter->decl)
                        {
                            sema_maybe_analyze_symbol_decl_in_module(sema, al->module, iter);
                        }

                        Type *iter_receiver_type = NULL;
                        if (iter->type && iter->type->kind == TYPE_FUNCTION && iter->type->function.param_count > 0)
                        {
                            iter_receiver_type = iter->type->function.param_types[0];
                        }

                        if (iter_receiver_type)
                        {
                            if (type_can_assign_to(obj_type, iter_receiver_type))
                            {
                                cand = iter;
                                break;
                            }

                            Type *iter_base = iter_receiver_type;
                            if (iter_base->kind == TYPE_POINTER)
                            {
                                iter_base = iter_base->pointer.base;
                            }
                            Type *obj_base = obj_type;
                            if (obj_base->kind == TYPE_POINTER)
                            {
                                obj_base = obj_base->pointer.base;
                            }
                            if (type_equals(iter_base, obj_base))
                            {
                                cand = iter;
                                break;
                            }
                        }
                    }
                }
                if (!cand)
                {
                    cand = symbol_table_lookup_local(al->module->table, node->field_expr.field);
                }
                if (!cand || cand->kind != SYMBOL_FUNCTION || !cand->is_public || !cand->decl || cand->decl->kind != AST_STMT_FUN || !cand->decl->fun_stmt.is_method)
                {
                    continue;
                }

                method        = cand;
                method_origin = al->module;
                break;
            }
        }

        free(qualified_name);
        free(generic_qualified_name);

        // if the method exists but hasn't been analyzed yet, analyze it under its module context
        if (method && !method->type && method->decl)
        {
            if (!method_origin)
            {
                method_origin = sema_find_module(sema, method->module_path);
            }
            sema_maybe_analyze_symbol_decl_in_module(sema, method_origin, method);
        }
        if (method && method->kind == SYMBOL_FUNCTION && method->decl && method->decl->kind == AST_STMT_FUN && method->decl->fun_stmt.is_method)
        {
            // if this is a generic method on an instantiated generic receiver type,
            // instantiate it using the receiver's concrete type args.
            if (method->is_generic)
            {
                Type *recv_type = obj_type;
                if (recv_type && recv_type->kind == TYPE_POINTER)
                {
                    recv_type = recv_type->pointer.base;
                }

                Type **args      = NULL;
                int    arg_count = 0;
                if (recv_type && recv_type->kind == TYPE_STRUCT)
                {
                    args      = recv_type->structure.generic_args;
                    arg_count = recv_type->structure.generic_arg_count;
                }
                else if (recv_type && recv_type->kind == TYPE_UNION)
                {
                    args      = recv_type->union_type.generic_args;
                    arg_count = recv_type->union_type.generic_arg_count;
                }

                if (args && arg_count > 0)
                {
                    AstList *type_args = malloc(sizeof(AstList));
                    if (type_args)
                    {
                        ast_list_init(type_args);
                        for (int i = 0; i < arg_count; i++)
                        {
                            AstNode *tn = sema_type_node_from_type(args[i]);
                            if (tn)
                            {
                                ast_list_append(type_args, tn);
                            }
                        }

                        if (type_args->count == arg_count)
                        {
                            Symbol *inst_method = sema_instantiate_generic(sema, method, type_args);
                            if (inst_method)
                            {
                                node->field_expr.is_method = true;
                                node->symbol               = inst_method;
                                node->type                 = inst_method->type;

                                // keep type_args nodes alive for now (owned by AST list)
                                free(type_args->items);
                                free(type_args);
                                return 0;
                            }
                        }

                        // cleanup on failure
                        for (int i = 0; i < type_args->count; i++)
                        {
                            AstNode *tn = type_args->items[i];
                            if (tn)
                            {
                                if (tn->kind == AST_TYPE_NAME)
                                {
                                    free(tn->type_name.name);
                                }
                                else if (tn->kind == AST_TYPE_PTR)
                                {
                                    // free base node (only simple forms are built here)
                                    AstNode *base = tn->type_ptr.base;
                                    if (base && base->kind == AST_TYPE_NAME)
                                    {
                                        free(base->type_name.name);
                                    }
                                    free(base);
                                }
                                free(tn);
                            }
                        }
                        free(type_args->items);
                        free(type_args);
                    }
                }
            }

            AstNode *receiver_ast = method->decl->fun_stmt.method_receiver;

            // handle pointer receivers: unwrap *T or &T to get base type
            while (receiver_ast && receiver_ast->kind == AST_TYPE_PTR)
            {
                receiver_ast = receiver_ast->type_ptr.base;
            }

            // check if this is a method on a generic type
            if (receiver_ast && receiver_ast->kind == AST_TYPE_NAME && receiver_ast->type_name.generic_args && receiver_ast->type_name.generic_args->count > 0)
            {
                // get the base type name (e.g., "Option" from "Option[T]")
                const char *generic_type_name = receiver_ast->type_name.name;

                // get the object's type name (e.g., "OptionIi64E" for Option[i64])
                Type *check_type = obj_type;
                if (check_type->kind == TYPE_POINTER)
                {
                    check_type = check_type->pointer.base;
                }

                // check if object type is an instantiation of this generic type
                if (check_type->kind == TYPE_STRUCT || check_type->kind == TYPE_UNION)
                {
                    const char *inst_name = (check_type->kind == TYPE_STRUCT) ? check_type->structure.name : check_type->union_type.name;

                    // check if inst_name starts with generic_type_name followed by 'I' (mangling convention)
                    size_t base_len = strlen(generic_type_name);
                    if (inst_name && strncmp(inst_name, generic_type_name, base_len) == 0 && inst_name[base_len] == 'I')
                    {
                        // this is an instantiated generic type - need to instantiate the method
                        // extract type arguments from the object's instantiated type

                        // look up the original generic type symbol
                        Symbol *generic_type_sym = symbol_table_lookup(sema->current_table, generic_type_name);
                        if (generic_type_sym && generic_type_sym->is_generic && generic_type_sym->decl)
                        {
                            AstList *formal_params = NULL;
                            if (generic_type_sym->decl->kind == AST_STMT_REC)
                            {
                                formal_params = generic_type_sym->decl->rec_stmt.generics;
                            }
                            else if (generic_type_sym->decl->kind == AST_STMT_UNI)
                            {
                                formal_params = generic_type_sym->decl->uni_stmt.generics;
                            }

                            if (formal_params)
                            {
                                // use the stored generic_args from the instantiated type
                                Type **stored_args     = NULL;
                                int    stored_arg_count = 0;

                                if (check_type->kind == TYPE_STRUCT)
                                {
                                    stored_args      = check_type->structure.generic_args;
                                    stored_arg_count = check_type->structure.generic_arg_count;
                                }
                                else if (check_type->kind == TYPE_UNION)
                                {
                                    stored_args      = check_type->union_type.generic_args;
                                    stored_arg_count = check_type->union_type.generic_arg_count;
                                }

                                if (stored_args && stored_arg_count > 0 && stored_arg_count == formal_params->count)
                                {
                                    // create synthetic type_args list from stored generic args
                                    AstList *type_args = malloc(sizeof(AstList));
                                    if (type_args)
                                    {
                                        ast_list_init(type_args);

                                        for (int i = 0; i < stored_arg_count; i++)
                                        {
                                            Type *actual_type = stored_args[i];
                                            if (actual_type)
                                            {
                                                // create a type node from the stored type using sema_type_node_from_type
                                                AstNode *type_node = sema_type_node_from_type(actual_type);
                                                if (type_node)
                                                {
                                                    ast_list_append(type_args, type_node);
                                                }
                                            }
                                        }

                                        if (type_args->count == stored_arg_count)
                                        {
                                            // instantiate the method with these type args
                                            Symbol *inst_method = sema_instantiate_generic(sema, method, type_args);
                                            if (inst_method)
                                            {
                                                node->field_expr.is_method = true;
                                                node->symbol               = inst_method;
                                                node->type                 = inst_method->type;

                                                // cleanup type_args
                                                for (int i = 0; i < type_args->count; i++)
                                                {
                                                    AstNode *tn = type_args->items[i];
                                                    if (tn->kind == AST_TYPE_NAME && tn->type_name.name)
                                                    {
                                                        free(tn->type_name.name);
                                                    }
                                                    free(tn);
                                                }
                                                free(type_args->items);
                                                free(type_args);

                                                return 0;
                                            }
                                        }

                                        // cleanup on failure
                                        for (int i = 0; i < type_args->count; i++)
                                        {
                                            AstNode *tn = type_args->items[i];
                                            if (tn->kind == AST_TYPE_NAME && tn->type_name.name)
                                            {
                                                free(tn->type_name.name);
                                            }
                                            free(tn);
                                        }
                                        free(type_args->items);
                                        free(type_args);
                                    }
                                }
                            }
                        }
                    }
                }
            }



            // non-generic method resolution (also handles generic methods where instantiation is deferred)
            // use the already-resolved receiver type from the method's function type
            // (first parameter) instead of re-resolving in caller's context, which
            // would fail for cross-module methods where the type name is unqualified
            Type *method_receiver_type = NULL;
            if (method->type && method->type->kind == TYPE_FUNCTION && method->type->function.param_count > 0)
            {
                method_receiver_type = method->type->function.param_types[0];
            }
            else
            {
                // fallback to resolving from AST - must do this in the method's module context
                // to properly resolve type names like "Allocator" that are local to that module
                SemaModule  *saved_module      = sema->current_module;
                SymbolTable *saved_table       = sema->current_table;
                const char  *saved_module_path = sema->module_path;
                char        *saved_file_path   = sema->current_file_path;
                char        *saved_source      = sema->current_source;

                if (method_origin)
                {
                    sema->current_module    = method_origin;
                    sema->current_table     = method_origin->table;
                    sema->module_path       = method_origin->module_path;
                    sema->current_file_path = method_origin->file_path;
                    sema->current_source    = method_origin->source;
                }

                method_receiver_type = sema_resolve_type(sema, method->decl->fun_stmt.method_receiver);

                sema->current_module    = saved_module;
                sema->current_table     = saved_table;
                sema->module_path       = saved_module_path;
                sema->current_file_path = saved_file_path;
                sema->current_source    = saved_source;
            }
            if (method_receiver_type)
            {
                // check if receiver type matches object type (including pointer coercion)
                if (type_can_assign_to(obj_type, method_receiver_type))
                {
                    node->field_expr.is_method = true;
                    node->symbol               = method;
                    node->type                 = method->type;
                    return 0;
                }

                // also check base types for pointer receivers (auto-ref/deref)
                Type *method_base_type = method_receiver_type;
                if (method_base_type->kind == TYPE_POINTER)
                {
                    method_base_type = method_base_type->pointer.base;
                }

                Type *obj_base_type = obj_type;
                if (obj_base_type->kind == TYPE_POINTER)
                {
                    obj_base_type = obj_base_type->pointer.base;
                }

                if (type_equals(method_base_type, obj_base_type))
                {
                    node->field_expr.is_method = true;
                    node->symbol               = method;
                    node->type                 = method->type;
                    return 0;
                }
            }
        }

        sema_error(sema, node->token, "undefined field or method");
        return -1;
    }

    case AST_EXPR_STRUCT:
    {
        Type *type = sema_resolve_type(sema, node->struct_expr.type);
        if (!type || (type->kind != TYPE_STRUCT && type->kind != TYPE_UNION))
        {
            sema_error(sema, node->token, "invalid record or union type");
            return -1;
        }

        bool is_union = (type->kind == TYPE_UNION);

        // track which fields have been initialized
        int   field_count    = type->structure.field_count;
        bool *field_provided = calloc(field_count, sizeof(bool));
        if (!field_provided && field_count > 0)
        {
            return -1;
        }

        // verify fields
        if (node->struct_expr.fields)
        {
            for (int i = 0; i < node->struct_expr.fields->count; i++)
            {
                AstNode *field_init = node->struct_expr.fields->items[i];
                // field_init is AST_EXPR_FIELD (field: name, object: init_expr)

                // find field in record/union type
                TypeField *field     = NULL;
                int        field_idx = -1;
                for (int j = 0; j < field_count; j++)
                {
                    if (strcmp(type->structure.fields[j].name, field_init->field_expr.field) == 0)
                    {
                        field     = &type->structure.fields[j];
                        field_idx = j;
                        break;
                    }
                }

                if (!field)
                {
                    sema_error(sema, field_init->token, is_union ? "undefined field in union literal" : "undefined field in record literal");
                    free(field_provided);
                    return -1;
                }

                // check for duplicate field initialization
                if (field_provided[field_idx])
                {
                    sema_error(sema, field_init->token, "duplicate field initialization");
                    free(field_provided);
                    return -1;
                }
                field_provided[field_idx] = true;

                // analyze init expression
                if (sema_analyze_expr(sema, field_init->field_expr.object) < 0)
                {
                    free(field_provided);
                    return -1;
                }

                sema_infer_numeric_expr(field_init->field_expr.object, field->type);

                if (!type_can_assign_to(field_init->field_expr.object->type, field->type))
                {
                    sema_error(sema, field_init->token, "field type mismatch");
                    free(field_provided);
                    return -1;
                }
                if (!sema_check_untyped_numeric(sema, field_init->field_expr.object, "could not infer type of numeric literal for field"))
                {
                    free(field_provided);
                    return -1;
                }
            }
        }

        // validation: records need all fields, unions need exactly one field
        int initialized_count = 0;
        for (int i = 0; i < field_count; i++)
        {
            if (field_provided[i])
            {
                initialized_count++;
            }
        }

        if (is_union)
        {
            // unions must have exactly one field initialized
            if (initialized_count != 1)
            {
                sema_error(sema, node->token, "union literal must initialize exactly one field");
                free(field_provided);
                return -1;
            }
        }
        else
        {
            // records must have all fields initialized
            for (int i = 0; i < field_count; i++)
            {
                if (!field_provided[i])
                {
                    char errmsg[256];
                    snprintf(errmsg, sizeof(errmsg), "missing required field '%s' in record literal", type->structure.fields[i].name);
                    sema_error(sema, node->token, errmsg);
                    free(field_provided);
                    return -1;
                }
            }
        }

        free(field_provided);
        node->type = type;
        return 0;
    }

    case AST_EXPR_CAST:
    {
        if (sema_analyze_expr(sema, node->cast_expr.expr) < 0)
        {
            return -1;
        }

        Type *target_type = sema_resolve_type(sema, node->cast_expr.type);
        if (!target_type)
        {
            sema_error(sema, node->token, "invalid cast type");
            return -1;
        }

        sema_infer_numeric_expr(node->cast_expr.expr, target_type);

        if (!sema_check_untyped_numeric(sema, node->cast_expr.expr, "could not infer type of numeric literal in cast"))
        {
            return -1;
        }

        // casts are pure bit reinterpretation (or zero-extension/truncation).
        // require both types to be sized.
        Type *from_type = node->cast_expr.expr->type;
        if (!from_type)
        {
            sema_error(sema, node->token, "cannot cast value with unknown type");
            return -1;
        }

        if (from_type->size == 0 || target_type->size == 0)
        {
            sema_error(sema, node->token, "cast requires sized types");
            return -1;
        }

        // forbid dropping constness: &T -> *U (cast-away-const)
        if (from_type->kind == TYPE_POINTER && target_type->kind == TYPE_POINTER)
        {
            if (from_type->pointer.is_const && !target_type->pointer.is_const)
            {
                sema_error(sema, node->token, "cannot cast readonly pointer to mutable pointer");
                return -1;
            }
        }

        node->type = target_type;
        return 0;
    }

    case AST_EXPR_INDEX:
    {
        if (sema_analyze_expr(sema, node->index_expr.array) < 0)
        {
            return -1;
        }
        Symbol *sym = node->index_expr.array->symbol;

        if (sym && sym->is_generic)
        {
            AstList *type_args = malloc(sizeof(AstList));
            if (type_args)
            {
                ast_list_init(type_args);
                ast_list_append(type_args, node->index_expr.index);

                Symbol *inst = sema_instantiate_generic(sema, sym, type_args);

                // cleanup list shell (items are owned by ast)
                free(type_args);

                if (!inst)
                {
                    sema_error(sema, node->token, "failed to instantiate generic");
                    return -1;
                }

                node->kind            = AST_EXPR_IDENT;
                node->ident_expr.name = strdup(inst->name);
                node->symbol          = inst;
                node->type            = inst->type;

                return 0;
            }
        }

        if (sema_analyze_expr(sema, node->index_expr.index) < 0)
        {
            return -1;
        }

        // index context requires an integer; infer to i64 if untyped numeric
        if (sema_is_untyped_numeric_literal(node->index_expr.index))
        {
            sema_infer_numeric_expr(node->index_expr.index, type_get_primitive(TYPE_I64));
        }

        Type *obj_type   = node->index_expr.array->type;
        Type *index_type = node->index_expr.index->type;

        if (sema_is_untyped_numeric_literal(node->index_expr.index))
        {
            sema_error(sema, node->index_expr.index->token, "could not infer type of numeric literal for index");
            return -1;
        }

        if (!index_type)
        {
            sema_error(sema, node->index_expr.index->token, "array index type unknown");
            return -1;
        }

        if (index_type->kind != TYPE_I64 && index_type->kind != TYPE_U64 && index_type->kind != TYPE_I32 && index_type->kind != TYPE_U32 && index_type->kind != TYPE_I16 && index_type->kind != TYPE_U16 && index_type->kind != TYPE_I8 &&
            index_type->kind != TYPE_U8)
        {
            sema_error(sema, node->index_expr.index->token, "array index must be an integer");
            return -1;
        }

        if (!obj_type)
        {
            sema_error(sema, node->token, "indexing on unknown type");
            return -1;
        }

        if (obj_type->kind == TYPE_ARRAY)
        {
            node->type = obj_type->array.elem_type;
        }
        else if (obj_type->kind == TYPE_POINTER)
        {
            node->type = obj_type->pointer.base;
        }
        else
        {
            sema_error(sema, node->token, "indexing on non-array/pointer type");
            return -1;
        }

        return 0;
    }

    case AST_COMPTIME:
    {
        AstNode *inner = node->comptime.inner;

        // check for $mach constants
        if (comptime_lookup(sema, node) == 0)
        {
            return 0;
        }

        // handle $symbol.attribute pattern
        if (inner && inner->kind == AST_EXPR_FIELD)
        {
            AstNode *obj = inner->field_expr.object;
            if (obj->kind != AST_EXPR_IDENT)
            {
                sema_error(sema, node->token, "compiletime attribute access requires symbol name");
                return -1;
            }

            Symbol *sym = symbol_table_lookup(sema->current_table, obj->ident_expr.name);
            if (!sym)
            {
                sema_error(sema, obj->token, "undefined symbol in compiletime expression");
                return -1;
            }

            const char *attr_name = inner->field_expr.field;
            if (strcmp(attr_name, "name") == 0)
            {
                node->comptime.value_kind   = COMPTIME_STRING;
                node->comptime.string_value = strdup(obj->ident_expr.name);
                node->type                  = type_get_primitive(TYPE_PTR); // &u8
                return 0;
            }
            else if (strcmp(attr_name, "size") == 0)
            {
                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value  = sym->type ? sym->type->size : 0;
                node->type                = type_get_primitive(TYPE_U64);
                return 0;
            }
            else if (strcmp(attr_name, "align") == 0)
            {
                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value  = sym->type ? sym->type->alignment : 0;
                node->type                = type_get_primitive(TYPE_U64);
                return 0;
            }
            else if (strcmp(attr_name, "field_count") == 0)
            {
                if (sym->type && sym->type->kind == TYPE_STRUCT)
                {
                    node->comptime.value_kind = COMPTIME_INT;
                    node->comptime.int_value  = sym->type->structure.field_count;
                    node->type                = type_get_primitive(TYPE_U64);
                    return 0;
                }
                sema_error(sema, node->token, "field_count only valid for record types");
                return -1;
            }
            else
            {
                sema_error(sema, node->token, "unknown compiletime attribute");
                return -1;
            }
        }

        // handle comptime intrinsics: $size_of(T), $align_of(T), $offset_of(T, field)
        if (inner && inner->kind == AST_EXPR_CALL && inner->call_expr.func && inner->call_expr.func->kind == AST_EXPR_IDENT)
        {
            const char *name = inner->call_expr.func->ident_expr.name;

            if (name && (strcmp(name, "size_of") == 0 || strcmp(name, "align_of") == 0))
            {
                if (!inner->call_expr.args || inner->call_expr.args->count != 1)
                {
                    sema_error(sema, node->token, "expected 1 type argument");
                    return -1;
                }

                Type *t = sema_resolve_type(sema, inner->call_expr.args->items[0]);
                if (!t)
                {
                    sema_error(sema, node->token, "failed to resolve type in comptime intrinsic");
                    return -1;
                }

                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value  = (strcmp(name, "size_of") == 0) ? (int64_t)t->size : (int64_t)t->alignment;
                node->type                = type_get_primitive(TYPE_U64);
                return 0;
            }

            if (name && strcmp(name, "offset_of") == 0)
            {
                if (!inner->call_expr.args || inner->call_expr.args->count != 2)
                {
                    sema_error(sema, node->token, "expected 2 arguments for offset_of(Type, field)");
                    return -1;
                }

                Type *t = sema_resolve_type(sema, inner->call_expr.args->items[0]);
                if (!t || (t->kind != TYPE_STRUCT && t->kind != TYPE_UNION))
                {
                    sema_error(sema, node->token, "offset_of requires a record or union type");
                    return -1;
                }

                AstNode *field_expr = inner->call_expr.args->items[1];
                if (!field_expr || field_expr->kind != AST_EXPR_FIELD)
                {
                    sema_error(sema, node->token, "offset_of second argument must be a field expression");
                    return -1;
                }

                const char *field_name = field_expr->field_expr.field;
                TypeField  *fields     = (t->kind == TYPE_STRUCT) ? t->structure.fields : t->union_type.fields;
                int         count      = (t->kind == TYPE_STRUCT) ? t->structure.field_count : t->union_type.field_count;

                for (int i = 0; i < count; i++)
                {
                    if (strcmp(fields[i].name, field_name) == 0)
                    {
                        node->comptime.value_kind = COMPTIME_INT;
                        node->comptime.int_value  = (int64_t)fields[i].offset;
                        node->type                = type_get_primitive(TYPE_U64);
                        return 0;
                    }
                }

                sema_error(sema, node->token, "unknown field in offset_of");
                return -1;
            }
        }

        sema_error(sema, node->token, "unsupported compiletime expression");
        return -1;
    }

    case AST_EXPR_NULL:
    {
        // null literal has untyped pointer type
        node->type = type_get_primitive(TYPE_PTR);
        return 0;
    }

    case AST_EXPR_ARRAY:
    {
        // array literal: [elem_type]{elem1, elem2, ...}
        if (!node->array_expr.type)
        {
            sema_error(sema, node->token, "array literal requires element type");
            return -1;
        }

        Type *array_type = sema_resolve_type(sema, node->array_expr.type);
        if (!array_type || array_type->kind != TYPE_ARRAY)
        {
            sema_error(sema, node->token, "failed to resolve array type");
            return -1;
        }
        Type *elem_type = array_type->array.elem_type;

        int elem_count = node->array_expr.elems ? node->array_expr.elems->count : 0;

        // analyze and type-check each element
        if (node->array_expr.elems)
        {
            for (int i = 0; i < elem_count; i++)
            {
                AstNode *elem = node->array_expr.elems->items[i];
                if (sema_analyze_expr(sema, elem) < 0)
                {
                    return -1;
                }

                sema_infer_numeric_expr(elem, elem_type);

                if (!type_can_assign_to(elem->type, elem_type))
                {
                    sema_error(sema, elem->token, "array element type mismatch");
                    return -1;
                }
                if (!sema_check_untyped_numeric(sema, elem, "could not infer type of numeric literal for array element"))
                {
                    return -1;
                }
            }
        }

        node->type = type_create_array(elem_type, elem_count);
        return 0;
    }

    case AST_EXPR_VARARGS:
    {
        // variadic argument pack - type is determined by context
        // for now, treat as untyped pointer
        node->type = type_get_primitive(TYPE_PTR);
        return 0;
    }

    default:
        return 0;
    }
}

// resolve a type symbol (SYMBOL_TYPE) considering module imports.
// - unqualified: search local scopes, then unaliased imports.
// - qualified: alias.Type resolves against the aliased module only.
//
// note: tokens do not carry file/module information, so any lazy analysis must
// run under the correct module context to keep diagnostics accurate.
static void sema_maybe_analyze_symbol_decl_in_module(Sema *sema, SemaModule *mod, Symbol *sym)
{
    if (!sema || !sym || sym->type || !sym->decl)
    {
        return;
    }

    // prevent infinite recursion for recursive types (e.g., rec Node { child: *Node; })
    if (sym->is_being_analyzed)
    {
        return;
    }
    sym->is_being_analyzed = true;

    SemaModule  *saved_module      = sema->current_module;
    SymbolTable *saved_table       = sema->current_table;
    const char  *saved_module_path = sema->module_path;
    char        *saved_file_path   = sema->current_file_path;
    char        *saved_source      = sema->current_source;
    Type        *saved_return_type = sema->current_function_return_type;

    if (mod)
    {
        sema->current_module    = mod;
        sema->current_table     = mod->table;
        sema->module_path       = mod->module_path;
        sema->current_file_path = mod->file_path;
        sema->current_source    = mod->source;
    }

    (void)sema_analyze_stmt(sema, sym->decl);

    sym->is_being_analyzed = false;

    sema->current_module               = saved_module;
    sema->current_table                = saved_table;
    sema->module_path                  = saved_module_path;
    sema->current_file_path            = saved_file_path;
    sema->current_source               = saved_source;
    sema->current_function_return_type = saved_return_type;
}

static Symbol *sema_lookup_type_symbol(Sema *sema, const char *module_alias, const char *name)
{
    if (!sema || !name)
    {
        return NULL;
    }

    // alias-qualified type: alias.Type
    if (module_alias)
    {
        if (!sema->current_module)
        {
            return NULL;
        }

        for (ModuleAlias *al = sema->current_module->aliases; al; al = al->next)
        {
            if (!al->alias || strcmp(al->alias, module_alias) != 0)
            {
                continue;
            }

            if (!al->module || !al->module->table)
            {
                return NULL;
            }

            Symbol *sym = symbol_table_lookup_local(al->module->table, name);
            if (!sym || sym->kind != SYMBOL_TYPE || !sym->is_public)
            {
                return NULL;
            }

            sema_maybe_analyze_symbol_decl_in_module(sema, al->module, sym);
            return sym;
        }

        return NULL;
    }

    // unqualified type: first check local scopes
    Symbol *sym = symbol_table_lookup(sema->current_table, name);
    if (sym && sym->kind == SYMBOL_TYPE)
    {
        SemaModule *origin = sema_find_module(sema, sym->module_path);
        sema_maybe_analyze_symbol_decl_in_module(sema, origin, sym);
        return sym;
    }

    // then check unaliased imports (`use foo.bar;` brings public symbols into scope)
    if (sema->current_module)
    {
        for (ModuleImport *imp = sema->current_module->imports; imp; imp = imp->next)
        {
            if (!imp->module || !imp->module->table)
            {
                continue;
            }

            Symbol *cand = symbol_table_lookup_local(imp->module->table, name);
            if (!cand || cand->kind != SYMBOL_TYPE || !cand->is_public)
            {
                continue;
            }

            sema_maybe_analyze_symbol_decl_in_module(sema, imp->module, cand);
            return cand;
        }
    }

    return NULL;
}

static Type *sema_resolve_type(Sema *sema, AstNode *type_node)
{
    if (!type_node)
    {
        return NULL;
    }

    // if the node already carries a resolved type, reuse it
    if (type_node->type)
    {
        return type_node->type;
    }

    switch (type_node->kind)
    {
    case AST_TYPE_NAME:
    case AST_EXPR_IDENT:
    {
        const char *name         = (type_node->kind == AST_TYPE_NAME) ? type_node->type_name.name : type_node->ident_expr.name;
        const char *module_alias = (type_node->kind == AST_TYPE_NAME) ? type_node->type_name.module_alias : NULL;
        AstList    *generic_args = (type_node->kind == AST_TYPE_NAME) ? type_node->type_name.generic_args : NULL;

        // check primitive types (no generics allowed)
        if (strcmp(name, "i8") == 0)
        {
            return type_get_primitive(TYPE_I8);
        }
        if (strcmp(name, "i16") == 0)
        {
            return type_get_primitive(TYPE_I16);
        }
        if (strcmp(name, "i32") == 0)
        {
            return type_get_primitive(TYPE_I32);
        }
        if (strcmp(name, "i64") == 0)
        {
            return type_get_primitive(TYPE_I64);
        }
        if (strcmp(name, "u8") == 0)
        {
            return type_get_primitive(TYPE_U8);
        }
        if (strcmp(name, "u16") == 0)
        {
            return type_get_primitive(TYPE_U16);
        }
        if (strcmp(name, "u32") == 0)
        {
            return type_get_primitive(TYPE_U32);
        }
        if (strcmp(name, "u64") == 0)
        {
            return type_get_primitive(TYPE_U64);
        }
        if (strcmp(name, "f32") == 0)
        {
            return type_get_primitive(TYPE_F32);
        }
        if (strcmp(name, "f64") == 0)
        {
            return type_get_primitive(TYPE_F64);
        }
        if (strcmp(name, "ptr") == 0)
        {
            return type_get_primitive(TYPE_PTR);
        }

        // builtin va_list
        if (strcmp(name, "va_list") == 0)
        {
            return type_get_builtin_va_list();
        }

        // look up user-defined types (local scope, then imports; or alias-qualified)
        Symbol *sym = sema_lookup_type_symbol(sema, module_alias, name);
        if (!sym)
        {
            return NULL;
        }

        if (sym->kind != SYMBOL_TYPE)
        {
            return NULL;
        }

        // handle generic type instantiation
        if (sym->is_generic && generic_args && generic_args->count > 0)
        {
            return sema_instantiate_generic_type(sema, sym, generic_args);
        }

        // non-generic type or generic type used without args (error case, but return type for now)
        if (sym->type)
        {
            return sym->type;
        }

        return NULL;
    }

    case AST_TYPE_PTR:
    {
        Type *base = sema_resolve_type(sema, type_node->type_ptr.base);
        if (!base)
        {
            return NULL;
        }
        return type_create_pointer(base, type_node->type_ptr.is_read_only);
    }

    case AST_TYPE_ARRAY:
    {
        Type *elem = sema_resolve_type(sema, type_node->type_array.elem_type);
        if (!elem)
        {
            return NULL;
        }

        // evaluate size
        AstNode *size_expr = type_node->type_array.size;
        if (size_expr->kind == AST_EXPR_LIT && size_expr->token->kind == TOKEN_LIT_INT)
        {
            size_t count = (size_t)size_expr->lit_expr.int_val;
            return type_create_array(elem, count);
        }
        else
        {
            sema_error(sema, size_expr->token, "array size must be a constant integer");
            return NULL;
        }
    }

    case AST_TYPE_PARAM:
    {
        // generic parameter type (e.g. T). these are typically bound to concrete
        // types in a temporary scope during instantiation.
        Symbol *sym = symbol_table_lookup(sema->current_table, type_node->type_param.name);
        if (sym && sym->type)
        {
            return sym->type;
        }

        // if unbound, treat it as an opaque generic param.
        return type_create_generic_param(type_node->type_param.name);
    }

    case AST_TYPE_FUN:
    {
        Type  *ret_type    = NULL;
        Type **param_types = NULL;
        int    param_count = 0;

        if (type_node->type_fun.return_type)
        {
            ret_type = sema_resolve_type(sema, type_node->type_fun.return_type);
            if (!ret_type)
            {
                return NULL;
            }
        }

        int fixed_count = type_node->type_fun.params ? type_node->type_fun.params->count : 0;
        if (fixed_count > 0 || type_node->type_fun.is_variadic)
        {
            int alloc_count = fixed_count + (type_node->type_fun.is_variadic ? 1 : 0);
            param_types     = malloc(sizeof(Type *) * alloc_count);
            if (!param_types)
            {
                return NULL;
            }

            for (int i = 0; i < fixed_count; i++)
            {
                AstNode *pt_node = type_node->type_fun.params->items[i];
                Type    *pt      = sema_resolve_type(sema, pt_node);
                if (!pt)
                {
                    free(param_types);
                    return NULL;
                }
                param_types[i] = pt;
            }

            if (type_node->type_fun.is_variadic)
            {
                param_types[fixed_count] = NULL; // sentinel
                param_count              = alloc_count;
            }
            else
            {
                param_count = fixed_count;
            }
        }

        return type_create_function(ret_type, param_types, param_count);
    }

    case AST_TYPE_REC:
    case AST_TYPE_UNI:
    {
        AstList *fields_ast  = (type_node->kind == AST_TYPE_REC) ? type_node->type_rec.fields : type_node->type_uni.fields;
        int      field_count = fields_ast ? fields_ast->count : 0;

        TypeField *fields = NULL;
        if (field_count > 0)
        {
            fields = malloc(sizeof(TypeField) * field_count);
            if (!fields)
            {
                return NULL;
            }

            for (int i = 0; i < field_count; i++)
            {
                AstNode *field_node = fields_ast->items[i];
                if (!field_node || field_node->kind != AST_STMT_FIELD)
                {
                    free(fields);
                    return NULL;
                }

                fields[i].name   = strdup(field_node->field_stmt.name);
                fields[i].type   = sema_resolve_type(sema, field_node->field_stmt.type);
                fields[i].offset = 0;

                if (!fields[i].type)
                {
                    for (int j = 0; j <= i; j++)
                    {
                        free(fields[j].name);
                    }
                    free(fields);
                    return NULL;
                }
            }
        }

        const char *name = (type_node->kind == AST_TYPE_REC) ? type_node->type_rec.name : type_node->type_uni.name;
        return (type_node->kind == AST_TYPE_REC) ? type_create_struct(name, fields, field_count) : type_create_union(name, fields, field_count);
    }

    default:
        return NULL;
    }
}

// analyze program root
int sema_analyze(Sema *sema, AstNode *ast)
{
    if (!sema || !ast)
    {
        return -1;
    }

    if (ast->kind == AST_PROGRAM)
    {
        if (ast->program.stmts)
        {
            // first pass: collect all top-level symbols
            for (int i = 0; i < ast->program.stmts->count; i++)
            {
                sema_collect_symbols(sema, ast->program.stmts->items[i]);
            }

            // second pass: full analysis
            for (int i = 0; i < ast->program.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, ast->program.stmts->items[i]) < 0)
                {
                    // continue analyzing to find more errors
                }
            }
        }
    }
    else if (ast->kind == AST_MODULE)
    {
        if (ast->module.stmts)
        {
            // first pass: collect all top-level symbols
            for (int i = 0; i < ast->module.stmts->count; i++)
            {
                sema_collect_symbols(sema, ast->module.stmts->items[i]);
            }

            // second pass: full analysis
            for (int i = 0; i < ast->module.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, ast->module.stmts->items[i]) < 0)
                {
                    // continue analyzing to find more errors
                }
            }
        }
    }

    return sema->error_count > 0 ? -1 : 0;
}

static Type *sema_instantiate_generic_type(Sema *sema, Symbol *generic_sym, AstList *type_args)
{
    if (!sema || !generic_sym || !type_args)
    {
        return NULL;
    }

    if (!generic_sym->is_generic || !generic_sym->decl)
    {
        return NULL;
    }

    AstNode *decl           = generic_sym->decl;
    AstList *generic_params = NULL;
    bool     is_struct      = false;

    if (decl->kind == AST_STMT_REC)
    {
        generic_params = decl->rec_stmt.generics;
        is_struct      = true;
    }
    else if (decl->kind == AST_STMT_UNI)
    {
        generic_params = decl->uni_stmt.generics;
        is_struct      = false;
    }
    else
    {
        return NULL; // not a type declaration
    }

    if (!generic_params || generic_params->count != type_args->count)
    {
        return NULL;
    }

    // resolve and retain concrete type args
    Type **resolved_args = malloc(sizeof(Type *) * type_args->count);
    if (!resolved_args)
    {
        return NULL;
    }

    for (int i = 0; i < type_args->count; i++)
    {
        AstNode *arg_node = type_args->items[i];
        Type    *arg_type = sema_resolve_type(sema, arg_node);
        if (!arg_type)
        {
            free(resolved_args);
            return NULL;
        }
        resolved_args[i] = arg_type;
    }

    // generate mangled name: <name>I<type_args>E
    char mangled_name[512];
    int  pos = snprintf(mangled_name, sizeof(mangled_name), "%s", generic_sym->name);

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "I");

    for (int i = 0; i < type_args->count; i++)
    {
        pos += type_mangle(resolved_args[i], mangled_name + pos, sizeof(mangled_name) - pos);
    }

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "E");

    // instantiate within the originating module scope
    SemaModule  *saved_module      = sema->current_module;
    SymbolTable *saved_table       = sema->current_table;
    const char  *saved_module_path = sema->module_path;
    char        *saved_file_path   = sema->current_file_path;
    char        *saved_source      = sema->current_source;
    Type        *saved_return_type = sema->current_function_return_type;

    SemaModule *origin = sema_find_module(sema, generic_sym->module_path);
    if (origin)
    {
        sema->current_module    = origin;
        sema->current_table     = origin->table;
        sema->module_path       = origin->module_path;
        sema->current_file_path = origin->file_path;
        sema->current_source    = origin->source;
    }

    // check if already instantiated
    Symbol *inst_sym = (origin && origin->table) ? symbol_table_lookup_local(origin->table, mangled_name) : NULL;
    if (inst_sym && inst_sym->type)
    {
        sema->current_module               = saved_module;
        sema->current_table                = saved_table;
        sema->module_path                  = saved_module_path;
        sema->current_file_path            = saved_file_path;
        sema->current_source               = saved_source;
        sema->current_function_return_type = saved_return_type;
        free(resolved_args);
        return inst_sym->type;
    }

    // create scope with generic parameter bindings
    SymbolTable *scope      = symbol_table_create(sema->current_table);
    SymbolTable *prev_table = sema->current_table;

    for (int i = 0; i < generic_params->count; i++)
    {
        AstNode *param_node = generic_params->items[i];
        if (param_node->kind == AST_TYPE_PARAM)
        {
            Type   *arg_type = resolved_args[i];
            Symbol *type_sym = symbol_create(param_node->type_param.name, SYMBOL_TYPE, sema->module_path);
            type_sym->type   = arg_type;
            symbol_table_insert(scope, type_sym);
        }
    }

    sema->current_table = scope;

    // resolve fields with substituted types
    AstList   *fields_ast  = is_struct ? decl->rec_stmt.fields : decl->uni_stmt.fields;
    int        field_count = fields_ast ? fields_ast->count : 0;
    TypeField *fields      = NULL;

    if (field_count > 0)
    {
        fields = malloc(sizeof(TypeField) * field_count);
        if (!fields)
        {
            sema->current_table = prev_table;
            return NULL;
        }

        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = fields_ast->items[i];
            if (field_node->kind != AST_STMT_FIELD)
            {
                free(fields);
                sema->current_table = prev_table;
                return NULL;
            }

            fields[i].name   = strdup(field_node->field_stmt.name);
            fields[i].type   = sema_resolve_type(sema, field_node->field_stmt.type);
            fields[i].offset = 0;

            if (!fields[i].type)
            {
                for (int j = 0; j <= i; j++)
                {
                    free(fields[j].name);
                }
                free(fields);
                sema->current_table = prev_table;
                return NULL;
            }
        }
    }

    sema->current_table = prev_table;

    // create instantiated type
    Type *inst_type = is_struct ? type_create_struct(mangled_name, fields, field_count) : type_create_union(mangled_name, fields, field_count);

    if (!inst_type)
    {
        if (fields)
        {
            for (int i = 0; i < field_count; i++)
            {
                free(fields[i].name);
            }
            free(fields);
        }
        free(resolved_args);
        return NULL;
    }

    // attach concrete generic args for later method instantiation
    if (is_struct)
    {
        inst_type->structure.generic_args      = resolved_args;
        inst_type->structure.generic_arg_count = type_args->count;
    }
    else
    {
        inst_type->union_type.generic_args      = resolved_args;
        inst_type->union_type.generic_arg_count = type_args->count;
    }

    inst_sym       = symbol_create(mangled_name, SYMBOL_TYPE, generic_sym->module_path);
    inst_sym->type = inst_type;
    inst_sym->decl = decl; // reference original declaration

    // cache within the originating module
    if (origin && origin->table)
    {
        symbol_table_insert(origin->table, inst_sym);
    }

    // restore caller context
    sema->current_module               = saved_module;
    sema->current_table                = saved_table;
    sema->module_path                  = saved_module_path;
    sema->current_file_path            = saved_file_path;
    sema->current_source               = saved_source;
    sema->current_function_return_type = saved_return_type;

    return inst_type;
}

Symbol *sema_instantiate_generic(Sema *sema, Symbol *generic_sym, AstList *type_args)
{
    if (!sema || !generic_sym || !type_args)
    {
        return NULL;
    }

    if (!generic_sym->is_generic || !generic_sym->decl)
    {
        return NULL;
    }

    AstNode *decl           = generic_sym->decl;
    AstList *generic_params = NULL;

    if (decl->kind == AST_STMT_FUN)
    {
        generic_params = decl->fun_stmt.generics;

        // for methods on generic types, get params from receiver type
        if (!generic_params && decl->fun_stmt.is_method && decl->fun_stmt.method_receiver)
        {
            AstNode *receiver = decl->fun_stmt.method_receiver;
            while (receiver && receiver->kind == AST_TYPE_PTR)
            {
                receiver = receiver->type_ptr.base;
            }

            if (receiver && receiver->kind == AST_TYPE_NAME && receiver->type_name.generic_args)
            {
                // look up the generic type to get its formal parameters.
                // prefer the method's originating module table (receiver type is declared there).
                SemaModule  *origin_mod   = sema_find_module(sema, generic_sym->module_path);
                SymbolTable *origin_table = (origin_mod && origin_mod->table) ? origin_mod->table : sema->current_table;
                Symbol      *type_sym     = origin_table ? symbol_table_lookup(origin_table, receiver->type_name.name) : NULL;
                if (type_sym && type_sym->is_generic && type_sym->decl)
                {
                    if (type_sym->decl->kind == AST_STMT_REC)
                    {
                        generic_params = type_sym->decl->rec_stmt.generics;
                    }
                    else if (type_sym->decl->kind == AST_STMT_UNI)
                    {
                        generic_params = type_sym->decl->uni_stmt.generics;
                    }
                }
            }
        }
    }

    if (!generic_params)
    {
        return NULL;
    }

    if (generic_params->count != type_args->count)
    {
        return NULL;
    }

    // resolve type arguments in the caller's current scope BEFORE switching modules.
    // this is required for dependent type args like `*T` where `T` is bound in the
    // caller's instantiation scope.
    Type **resolved_args = malloc(sizeof(Type *) * type_args->count);
    if (!resolved_args)
    {
        return NULL;
    }
    for (int i = 0; i < type_args->count; i++)
    {
        AstNode *arg_node = type_args->items[i];
        resolved_args[i]  = sema_resolve_type(sema, arg_node);
        if (!resolved_args[i])
        {
            free(resolved_args);
            return NULL;
        }
    }

    // generate mangled name: <name>I<type_args>E
    char mangled_name[512];
    int  pos = snprintf(mangled_name, sizeof(mangled_name), "%s", generic_sym->name);

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "I");

    for (int i = 0; i < type_args->count; i++)
    {
        pos += type_mangle(resolved_args[i], mangled_name + pos, sizeof(mangled_name) - pos);
    }

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "E");

    // instantiate within the originating module scope to avoid cross-module collisions
    SemaModule  *saved_module      = sema->current_module;
    SymbolTable *saved_table       = sema->current_table;
    const char  *saved_module_path = sema->module_path;
    char        *saved_file_path   = sema->current_file_path;
    char        *saved_source      = sema->current_source;
    Type        *saved_return_type = sema->current_function_return_type;

    SemaModule *origin = sema_find_module(sema, generic_sym->module_path);
    if (origin)
    {
        sema->current_module    = origin;
        sema->current_table     = origin->table;
        sema->module_path       = origin->module_path;
        sema->current_file_path = origin->file_path;
        sema->current_source    = origin->source;
    }

    Symbol *inst = NULL;
    if (origin && origin->table)
    {
        inst = symbol_table_lookup_local(origin->table, mangled_name);
    }
    if (inst)
    {
        sema->current_module               = saved_module;
        sema->current_table                = saved_table;
        sema->module_path                  = saved_module_path;
        sema->current_file_path            = saved_file_path;
        sema->current_source               = saved_source;
        sema->current_function_return_type = saved_return_type;
        free(resolved_args);
        return inst;
    }

    AstNode *cloned_decl = ast_clone(decl);
    if (!cloned_decl)
    {
        sema->current_module               = saved_module;
        sema->current_table                = saved_table;
        sema->module_path                  = saved_module_path;
        sema->current_file_path            = saved_file_path;
        sema->current_source               = saved_source;
        sema->current_function_return_type = saved_return_type;
        free(resolved_args);
        return NULL;
    }

    if (cloned_decl->kind == AST_STMT_FUN)
    {
        free(cloned_decl->fun_stmt.name);
        cloned_decl->fun_stmt.name     = strdup(mangled_name);
        cloned_decl->fun_stmt.generics = NULL;
    }

    Symbol *inst_sym = symbol_create(mangled_name, SYMBOL_FUNCTION, generic_sym->module_path);
    inst_sym->decl   = cloned_decl; // link symbol to cloned ast for MIR lowering
    if (origin && origin->table)
    {
        symbol_table_insert(origin->table, inst_sym);
    }
    cloned_decl->symbol = inst_sym;

    SymbolTable *scope      = symbol_table_create(sema->current_table);
    SymbolTable *prev_table = sema->current_table;

    for (int i = 0; i < generic_params->count; i++)
    {
        AstNode *param_node = generic_params->items[i];
        if (param_node->kind == AST_TYPE_PARAM)
        {
            Symbol *type_sym = symbol_create(param_node->type_param.name, SYMBOL_TYPE, generic_sym->module_path);
            type_sym->type   = resolved_args[i];
            symbol_table_insert(scope, type_sym);
        }
    }

    sema->current_table = scope;

    Type *ret_type = NULL;
    if (cloned_decl->fun_stmt.return_type)
    {
        ret_type = sema_resolve_type(sema, cloned_decl->fun_stmt.return_type);
    }

    Type **param_types = NULL;
    int    param_count = 0;
    int    fixed_count = cloned_decl->fun_stmt.params ? cloned_decl->fun_stmt.params->count : 0;

    if (fixed_count > 0 || cloned_decl->fun_stmt.is_variadic)
    {
        int alloc_count = fixed_count + (cloned_decl->fun_stmt.is_variadic ? 1 : 0);
        param_types     = malloc(sizeof(Type *) * alloc_count);
        if (!param_types)
        {
            sema->current_table = prev_table;
            return NULL;
        }

        for (int i = 0; i < fixed_count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Type *pt = NULL;
                if (param->param_stmt.type)
                {
                    pt = sema_resolve_type(sema, param->param_stmt.type);
                }
                param_types[i] = pt;
                param->type    = pt;
            }
        }

        if (cloned_decl->fun_stmt.is_variadic)
        {
            param_types[fixed_count] = NULL; // sentinel
            param_count              = alloc_count;
        }
        else
        {
            param_count = fixed_count;
        }
    }

    inst_sym->type = type_create_function(ret_type, param_types, param_count);

    // make the instantiated decl self-contained for later lowering passes.
    cloned_decl->type = inst_sym->type;

    if (cloned_decl->fun_stmt.params)
    {
        for (int i = 0; i < cloned_decl->fun_stmt.params->count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Symbol *param_sym = symbol_create(param->param_stmt.name, SYMBOL_VARIABLE, generic_sym->module_path);
                param_sym->type   = param->type;
                symbol_table_insert(sema->current_table, param_sym);
                param->symbol   = param_sym;
                param_sym->decl = param;
            }
        }
    }

    if (cloned_decl->fun_stmt.body)
    {
        // ensure return statements inside the instantiated body are checked
        // against this instantiated function's return type.
        sema->current_function_return_type = ret_type;
        sema_analyze_stmt(sema, cloned_decl->fun_stmt.body);
        sema->current_function_return_type = saved_return_type;
    }

    sema->current_table = prev_table;

    // restore caller context
    sema->current_module               = saved_module;
    sema->current_table                = saved_table;
    sema->module_path                  = saved_module_path;
    sema->current_file_path            = saved_file_path;
    sema->current_source               = saved_source;
    sema->current_function_return_type = saved_return_type;

    free(resolved_args);
    return inst_sym;
}
