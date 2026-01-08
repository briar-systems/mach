#include "compiler/sema.h"
#include "compiler/comptime.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/type.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// track loaded modules to avoid circular imports and redundant work
typedef struct LoadedModule
{
    char                *module_path; // fully qualified module path
    char                *source;      // source text (kept for error reporting)
    char                *file_path;   // file path for error messages
    AstNode             *ast;         // parsed AST
    struct LoadedModule *next;
} LoadedModule;

// track import aliases (use alias: module.path;)
typedef struct ImportAlias
{
    char               *alias;       // the alias name
    char               *module_path; // fully qualified module path
    SymbolTable        *table;       // symbol table of the imported module
    struct ImportAlias *next;
} ImportAlias;

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
    SymbolTable *root_table;
    SymbolTable *current_table;
    int          error_count;
    char        *module_path;
    Type        *current_function_return_type;

    // error list
    SemaError *errors;
    int        errors_count;
    int        errors_capacity;

    // current file context for error reporting
    char *current_file_path;
    char *current_source;

    // module resolution
    char         *project_id;     // project id (module prefix)
    char         *src_root;       // source directory path
    char         *dep_root;       // dependencies directory path
    ConfigDep   **deps;           // dependencies for module resolution
    int           dep_count;      // number of dependencies
    LoadedModule *loaded_modules; // cache of loaded modules
    ImportAlias  *import_aliases; // linked list of import aliases

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

    sema->root_table                 = symbol_table_create(NULL);
    sema->current_table              = sema->root_table;
    sema->error_count                = 0;
    sema->module_path                = module_path ? strdup(module_path) : NULL;
    sema->errors                     = NULL;
    sema->errors_count               = 0;
    sema->errors_capacity            = 0;
    sema->current_file_path          = NULL;
    sema->current_source             = NULL;
    sema->project_id                 = NULL;
    sema->src_root                   = NULL;
    sema->dep_root                   = NULL;
    sema->deps                       = NULL;
    sema->dep_count                  = 0;
    sema->loaded_modules             = NULL;
    sema->import_aliases             = NULL;
    sema->current_function_return_type = NULL;
    sema->extra_roots                = NULL;
    sema->extra_root_count           = 0;
    sema->extra_root_capacity        = 0;

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
    if (sema->module_path)
    {
        free(sema->module_path);
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

    // free current file context
    free(sema->current_file_path);
    free(sema->current_source);

    // free loaded modules
    LoadedModule *mod = sema->loaded_modules;
    while (mod)
    {
        LoadedModule *next = mod->next;
        free(mod->module_path);
        free(mod->file_path);
        free(mod->source);
        // note: ast is not freed here as it may still be in use
        free(mod);
        mod = next;
    }

    // free import aliases
    ImportAlias *alias = sema->import_aliases;
    while (alias)
    {
        ImportAlias *next = alias->next;
        free(alias->alias);
        free(alias->module_path);
        // note: symbol table is not freed here as it's part of loaded modules
        free(alias);
        alias = next;
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
        int new_cap = sema->extra_root_capacity ? sema->extra_root_capacity * 2 : 4;
        ModuleRoot *new_roots = realloc(sema->extra_roots, sizeof(ModuleRoot) * new_cap);
        if (!new_roots)
        {
            return;
        }
        sema->extra_roots         = new_roots;
        sema->extra_root_capacity = new_cap;
    }

    ModuleRoot *mr = &sema->extra_roots[sema->extra_root_count++];
    mr->prefix      = strdup(module_prefix);
    mr->src_root    = strdup(src_root);
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
                    dep_copy->version = NULL;           // we don't need version info for module resolution
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
        sema->deps = NULL;
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

    free(sema->current_file_path);
    free(sema->current_source);

    sema->current_file_path = file_path ? strdup(file_path) : NULL;
    sema->current_source    = source ? strdup(source) : NULL;
}

SymbolTable *sema_get_root_table(Sema *sema)
{
    return sema ? sema->root_table : NULL;
}

int sema_get_loaded_modules(Sema *sema, AstNode **asts, int max_count)
{
    if (!sema || !asts || max_count <= 0)
    {
        return 0;
    }

    int           count = 0;
    LoadedModule *mod   = sema->loaded_modules;
    while (mod && count < max_count)
    {
        asts[count++] = mod->ast;
        mod           = mod->next;
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

    sema->error_count++;
    sema_error_list_add(sema, token, message);
}

// forward declarations for mutual recursion
static int   sema_analyze_stmt(Sema *sema, AstNode *node);
static int   sema_analyze_expr(Sema *sema, AstNode *node);

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
static int   sema_collect_symbols(Sema *sema, AstNode *node);
static int   sema_analyze_use(Sema *sema, AstNode *node);

// collect symbols from a statement (first pass - no body analysis)
static int sema_collect_fun_symbol(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }

    // check if symbol already exists (from forward declaration or previous pass)
    Symbol *existing = symbol_table_lookup_local(sema->current_table, node->fun_stmt.name);
    if (existing)
    {
        // symbol already collected, just link it
        node->symbol = existing;
        return 0;
    }

    // create symbol for function
    Symbol *sym = symbol_create(node->fun_stmt.name, SYMBOL_FUNCTION, sema->module_path);
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
        node->symbol = existing;
        return 0;
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
        node->symbol = existing;
        return 0;
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
        node->symbol = existing;
        return 0;
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
        node->symbol = existing;
        return 0;
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
        node->symbol = existing;
        return 0;
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

    if (node->fun_stmt.params)
    {
        param_count = node->fun_stmt.params->count;
        param_types = malloc(sizeof(Type *) * param_count);

        for (int i = 0; i < param_count; i++)
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
    }

    sym->type = type_create_function(ret_type, param_types, param_count);

    // analyze body
    if (node->fun_stmt.body)
    {
        // create new scope for function body
        SymbolTable *prev_table = sema->current_table;
        sema->current_table     = symbol_table_create(prev_table);

        Type *prev_return_type               = sema->current_function_return_type;
        sema->current_function_return_type = sym->type ? sym->type->function.return_type : NULL;

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

        sema->current_table = prev_table;
        sema->current_function_return_type = prev_return_type;
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
    case TYPE_I8:  node->kind = AST_TYPE_NAME; node->type_name.name = strdup("i8");  break;
    case TYPE_I16: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("i16"); break;
    case TYPE_I32: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("i32"); break;
    case TYPE_I64: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("i64"); break;
    case TYPE_U8:  node->kind = AST_TYPE_NAME; node->type_name.name = strdup("u8");  break;
    case TYPE_U16: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("u16"); break;
    case TYPE_U32: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("u32"); break;
    case TYPE_U64: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("u64"); break;
    case TYPE_F32: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("f32"); break;
    case TYPE_F64: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("f64"); break;
    case TYPE_PTR: node->kind = AST_TYPE_NAME; node->type_name.name = strdup("ptr"); break;

    case TYPE_POINTER:
    {
        AstNode *base = sema_type_node_from_type(t->pointer.base);
        if (!base)
        {
            free(node);
            return NULL;
        }
        node->kind = AST_TYPE_PTR;
        node->type_ptr.base = base;
        node->type_ptr.is_read_only = t->pointer.is_const;
        break;
    }

    case TYPE_STRUCT:
    {
        node->kind = AST_TYPE_NAME;
        node->type_name.name = strdup(t->structure.name ? t->structure.name : "");
        break;
    }
    case TYPE_UNION:
    {
        node->kind = AST_TYPE_NAME;
        node->type_name.name = strdup(t->union_type.name ? t->union_type.name : "");
        break;
    }

    default:
        free(node);
        return NULL;
    }

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

        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->rec_stmt.fields->items[i];
            if (field_node->kind != AST_STMT_FIELD)
            {
                free(fields);
                return -1;
            }

            fields[i].name   = strdup(field_node->field_stmt.name);
            fields[i].type   = sema_resolve_type(sema, field_node->field_stmt.type);
            fields[i].offset = 0; // calculated in type_create_struct

            if (!fields[i].type)
            {
                sema_error(sema, field_node->token, "failed to resolve field type");
                for (int j = 0; j <= i; j++)
                {
                    free(fields[j].name);
                }
                free(fields);
                return -1;
            }
        }
    }

    // create record type
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

    sym->type  = rec_type;
    node->type = rec_type;

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
            else if (strcmp(name, "assert") == 0)
            {
                // $assert(cond, "message")
                if (!inner->call_expr.args || inner->call_expr.args->count != 2)
                {
                    sema_error(sema, inner->token, "expected 2 arguments for $assert");
                    return -1;
                }

                AstNode *cond = inner->call_expr.args->items[0];
                AstNode *msg  = inner->call_expr.args->items[1];

                if (sema_analyze_expr(sema, cond) < 0)
                {
                    return -1;
                }
                if (sema_analyze_expr(sema, msg) < 0)
                {
                    return -1;
                }

                int64_t val = 0;
                if (!sema_eval_comptime_int(sema, cond, &val))
                {
                    sema_error(sema, cond->token, "condition is not a compile-time constant");
                    return -1;
                }

                if (msg->kind != AST_EXPR_LIT || msg->lit_expr.kind != TOKEN_LIT_STRING)
                {
                    sema_error(sema, msg->token, "expected string literal for assertion message");
                    return -1;
                }

                if (val == 0)
                {
                    sema_error(sema, inner->token, msg->lit_expr.string_val);
                    return -1;
                }

                return 0;
            }
        }
    }

    // treat bare comptime reads (e.g. $mach.os.id) as no-ops but still analyze them for validity
    return sema_analyze_expr(sema, inner);
}

// check if a module has already been loaded
static LoadedModule *sema_find_loaded_module(Sema *sema, const char *module_path)
{
    LoadedModule *mod = sema->loaded_modules;
    while (mod)
    {
        if (strcmp(mod->module_path, module_path) == 0)
        {
            return mod;
        }
        mod = mod->next;
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
static int sema_load_module(Sema *sema, const char *module_path, AstNode **out_ast)
{
    if (!sema || !module_path || !out_ast)
    {
        return -1;
    }

    *out_ast = NULL;

    // check if already loaded
    LoadedModule *cached = sema_find_loaded_module(sema, module_path);
    if (cached)
    {
        *out_ast = cached->ast;
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

    // cache the module (keep source for error reporting)
    LoadedModule *mod = malloc(sizeof(LoadedModule));
    if (mod)
    {
        mod->module_path     = strdup(module_path);
        mod->file_path       = strdup(file_path);
        mod->source          = source; // take ownership
        mod->ast             = ast;
        mod->next            = sema->loaded_modules;
        sema->loaded_modules = mod;
    }

    // analyze the imported module
    // save current context and restore after
    char *saved_module_path = sema->module_path;
    char *saved_file_path   = sema->current_file_path;
    char *saved_source      = sema->current_source;

    sema->module_path       = strdup(module_path);
    sema->current_file_path = strdup(file_path);
    sema->current_source    = source; // borrowed from mod

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

    free(sema->module_path);
    free(sema->current_file_path);
    sema->module_path       = saved_module_path;
    sema->current_file_path = saved_file_path;
    sema->current_source    = saved_source;

    // clean up parser/lexer
    parser_dnit(&parser);
    lexer_dnit(&lexer);
    free(file_path);

    *out_ast = ast;
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
    AstNode *module_ast = NULL;
    if (sema_load_module(sema, module_path, &module_ast) < 0)
    {
        char errmsg[512];
        snprintf(errmsg, sizeof(errmsg), "failed to load module '%s'", module_path);
        sema_error(sema, node->token, errmsg);
        return -1;
    }

    // if aliased, store the alias mapping
    if (alias)
    {
        ImportAlias *new_alias = malloc(sizeof(ImportAlias));
        if (new_alias)
        {
            new_alias->alias       = strdup(alias);
            new_alias->module_path = strdup(module_path);
            new_alias->table       = sema->root_table; // symbols are in root table
            new_alias->next        = sema->import_aliases;
            sema->import_aliases   = new_alias;
        }
    }

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
        // look up identifier in symbol table
        Symbol *sym = symbol_table_lookup(sema->current_table, node->ident_expr.name);

        if (!sym)
        {
            sema_error(sema, node->token, "undefined identifier");
            return -1;
        }

        // if symbol was collected but not yet analyzed, analyze its declaration now
        // temporarily switch to root scope since top-level declarations belong there
        if (!sym->type && sym->decl)
        {
            SymbolTable *saved_table = sema->current_table;
            sema->current_table      = sema->root_table;

            if (sema_analyze_stmt(sema, sym->decl) < 0)
            {
                sema->current_table = saved_table;
                return -1;
            }

            sema->current_table = saved_table;
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
            sema_try_infer_numeric_literal(node->binary_expr.right, node->binary_expr.left->type);
        }
        if (type_is_numeric(node->binary_expr.right->type))
        {
            sema_try_infer_numeric_literal(node->binary_expr.left, node->binary_expr.right->type);
        }

        // result type depends on operator
        switch (node->binary_expr.op)
        {
        case TOKEN_EQUAL: // assignment
            node->type = node->binary_expr.left->type;
            break;

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
            // address-of: ?T -> *T
            // check if operand is an l-value
            AstNode *operand   = node->unary_expr.expr;
            bool     is_lvalue = operand->kind == AST_EXPR_IDENT ||                                     // variable
                             (operand->kind == AST_EXPR_UNARY && operand->unary_expr.op == TOKEN_AT) || // dereference
                             operand->kind == AST_EXPR_FIELD ||                                         // field access
                             operand->kind == AST_EXPR_INDEX;                                           // array indexing

            if (!is_lvalue)
            {
                sema_error(sema, node->token, "cannot take address of temporary or r-value");
                return -1;
            }
            node->type = type_create_pointer(node->unary_expr.expr->type, false);
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
            if (func_sym && func_sym->decl && func_sym->decl->fun_stmt.method_receiver)
            {
                expected_receiver_type = sema_resolve_type(sema, func_sym->decl->fun_stmt.method_receiver);
            }

            // auto-ref: if method expects pointer but receiver is value, wrap in address-of
            if (expected_receiver_type && expected_receiver_type->kind == TYPE_POINTER && receiver_type && receiver_type->kind != TYPE_POINTER)
            {
                // create address-of expression: ?receiver
                AstNode *addr_of = malloc(sizeof(AstNode));
                memset(addr_of, 0, sizeof(AstNode));
                addr_of->kind            = AST_EXPR_UNARY;
                addr_of->unary_expr.op   = TOKEN_QUESTION;
                addr_of->unary_expr.expr = receiver;
                addr_of->type            = type_create_pointer(receiver_type, false);
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

            // check if ident_name matches an import alias
            ImportAlias *alias = sema->import_aliases;
            while (alias)
            {
                if (strcmp(alias->alias, ident_name) == 0)
                {
                    // this is an aliased module access
                    // look up the symbol in the module's namespace
                    const char *symbol_name = node->field_expr.field;

                    // find symbols from the imported module by checking module_path
                    Symbol *sym = sema->root_table->symbols;
                    while (sym)
                    {
                        if (sym->module_path && strcmp(sym->module_path, alias->module_path) == 0 && strcmp(sym->name, symbol_name) == 0 && sym->is_public)
                        {
                            // found matching symbol from the aliased module
                            // if symbol was collected but not yet analyzed, analyze it now
                            if (!sym->type && sym->decl)
                            {
                                SymbolTable *saved_table = sema->current_table;
                                sema->current_table      = sema->root_table;

                                if (sema_analyze_stmt(sema, sym->decl) < 0)
                                {
                                    sema->current_table = saved_table;
                                    return -1;
                                }

                                sema->current_table = saved_table;
                            }

                            // convert this field access into an identifier
                            node->kind            = AST_EXPR_IDENT;
                            node->ident_expr.name = strdup(symbol_name);
                            node->symbol          = sym;
                            node->type            = sym->type;
                            return 0;
                        }
                        sym = sym->next;
                    }

                    // symbol not found in aliased module
                    sema_error(sema, node->token, "undefined symbol in aliased module");
                    return -1;
                }
                alias = alias->next;
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

        // look for method - methods can be defined on any type including pointers
        Symbol *method = symbol_table_lookup(sema->current_table, node->field_expr.field);
        if (method)
        {
            if (method->decl)
            {
            }
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

                Type **args = NULL;
                int    arg_count = 0;
                if (recv_type && recv_type->kind == TYPE_STRUCT)
                {
                    args = recv_type->structure.generic_args;
                    arg_count = recv_type->structure.generic_arg_count;
                }
                else if (recv_type && recv_type->kind == TYPE_UNION)
                {
                    args = recv_type->union_type.generic_args;
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
                                // create type args from the instantiated type's fields
                                // for now, extract from the mangled name or use the object's type directly
                                // since we stored the instantiated type in check_type, we can match field types

                                // create synthetic type_args list from the object's actual type
                                AstList *type_args = malloc(sizeof(AstList));
                                if (type_args)
                                {
                                    ast_list_init(type_args);

                                    // for each formal param, find the corresponding actual type from the instantiated record
                                    for (int i = 0; i < formal_params->count && i < receiver_ast->type_name.generic_args->count; i++)
                                    {
                                        // get the field at the same index to infer type
                                        // (this is a simplification - proper impl would parse the mangled name or store type args)
                                        if (check_type->kind == TYPE_STRUCT && check_type->structure.field_count > 0)
                                        {
                                            // find a field that uses the type parameter and get its actual type
                                            Type *actual_type = NULL;

                                            // look for the 'value' field which should have type T
                                            for (int j = 0; j < check_type->structure.field_count; j++)
                                            {
                                                if (strcmp(check_type->structure.fields[j].name, "value") == 0)
                                                {
                                                    actual_type = check_type->structure.fields[j].type;
                                                    break;
                                                }
                                            }

                                            if (actual_type)
                                            {
                                                // create a type node for the actual type
                                                AstNode *type_node = malloc(sizeof(AstNode));
                                                if (type_node)
                                                {
                                                    memset(type_node, 0, sizeof(AstNode));
                                                    type_node->kind = AST_TYPE_NAME;
                                                    type_node->type = actual_type;

                                                    // set the name based on type kind
                                                    if (actual_type->kind == TYPE_I64)
                                                    {
                                                        type_node->type_name.name = strdup("i64");
                                                    }
                                                    else if (actual_type->kind == TYPE_I32)
                                                    {
                                                        type_node->type_name.name = strdup("i32");
                                                    }
                                                    else if (actual_type->kind == TYPE_U8)
                                                    {
                                                        type_node->type_name.name = strdup("u8");
                                                    }
                                                    else if (actual_type->kind == TYPE_U64)
                                                    {
                                                        type_node->type_name.name = strdup("u64");
                                                    }
                                                    else if (actual_type->kind == TYPE_PTR)
                                                    {
                                                        type_node->type_name.name = strdup("ptr");
                                                    }
                                                    else
                                                    {
                                                        type_node->type_name.name = strdup("i64"); // fallback
                                                    }

                                                    ast_list_append(type_args, type_node);
                                                }
                                            }
                                        }
                                    }

                                    if (type_args->count > 0)
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
                                                if (tn->type_name.name)
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
                                        if (tn->type_name.name)
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

            // non-generic method resolution
            Type *method_receiver_type = sema_resolve_type(sema, method->decl->fun_stmt.method_receiver);
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
                node->type                = type_get_primitive(TYPE_I64);
                return 0;
            }
            else if (strcmp(attr_name, "align") == 0)
            {
                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value  = sym->type ? sym->type->size : 0;
                node->type                = type_get_primitive(TYPE_I64);
                return 0;
            }
            else if (strcmp(attr_name, "field_count") == 0)
            {
                if (sym->type && sym->type->kind == TYPE_STRUCT)
                {
                    node->comptime.value_kind = COMPTIME_INT;
                    node->comptime.int_value  = sym->type->structure.field_count;
                    node->type                = type_get_primitive(TYPE_I64);
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
                node->type                = type_get_primitive(TYPE_I64);
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
                TypeField  *fields = (t->kind == TYPE_STRUCT) ? t->structure.fields : t->union_type.fields;
                int         count  = (t->kind == TYPE_STRUCT) ? t->structure.field_count : t->union_type.field_count;

                for (int i = 0; i < count; i++)
                {
                    if (strcmp(fields[i].name, field_name) == 0)
                    {
                        node->comptime.value_kind = COMPTIME_INT;
                        node->comptime.int_value  = (int64_t)fields[i].offset;
                        node->type                = type_get_primitive(TYPE_I64);
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

static Type *sema_resolve_type(Sema *sema, AstNode *type_node)
{
    if (!type_node)
    {
        return NULL;
    }

    switch (type_node->kind)
    {
    case AST_TYPE_NAME:
    case AST_EXPR_IDENT:
    {
        const char *name         = (type_node->kind == AST_TYPE_NAME) ? type_node->type_name.name : type_node->ident_expr.name;
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

        // look up user-defined types
        Symbol *sym = symbol_table_lookup(sema->current_table, name);
        if (!sym)
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
        Type  *ret_type   = NULL;
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

        if (type_node->type_fun.params)
        {
            param_count = type_node->type_fun.params->count;
            param_types = malloc(sizeof(Type *) * param_count);
            if (!param_types)
            {
                return NULL;
            }

            for (int i = 0; i < param_count; i++)
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
        }

        return type_create_function(ret_type, param_types, param_count);
    }

    case AST_TYPE_REC:
    case AST_TYPE_UNI:
    {
        AstList *fields_ast = (type_node->kind == AST_TYPE_REC) ? type_node->type_rec.fields : type_node->type_uni.fields;
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

    // check if already instantiated
    Symbol *inst_sym = symbol_table_lookup(sema->root_table, mangled_name);
    if (inst_sym && inst_sym->type)
    {
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
        inst_type->structure.generic_args = resolved_args;
        inst_type->structure.generic_arg_count = type_args->count;
    }
    else
    {
        inst_type->union_type.generic_args = resolved_args;
        inst_type->union_type.generic_arg_count = type_args->count;
    }

    inst_sym       = symbol_create(mangled_name, SYMBOL_TYPE, sema->module_path);
    inst_sym->type = inst_type;
    inst_sym->decl = decl; // reference original declaration
    symbol_table_insert(sema->root_table, inst_sym);

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
                // look up the generic type to get its formal parameters
                Symbol *type_sym = symbol_table_lookup(sema->current_table, receiver->type_name.name);
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

    // generate mangled name: <name>I<type_args>E
    char mangled_name[512];
    int  pos = snprintf(mangled_name, sizeof(mangled_name), "%s", generic_sym->name);

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "I");

    for (int i = 0; i < type_args->count; i++)
    {
        AstNode *arg_node = type_args->items[i];
        Type    *arg_type = sema_resolve_type(sema, arg_node);
        if (arg_type)
        {
            pos += type_mangle(arg_type, mangled_name + pos, sizeof(mangled_name) - pos);
        }
    }

    pos += snprintf(mangled_name + pos, sizeof(mangled_name) - pos, "E");

    Symbol *inst = symbol_table_lookup(sema->root_table, mangled_name);
    if (inst)
    {
        return inst;
    }

    AstNode *cloned_decl = ast_clone(decl);
    if (!cloned_decl)
    {
        return NULL;
    }

    if (cloned_decl->kind == AST_STMT_FUN)
    {
        free(cloned_decl->fun_stmt.name);
        cloned_decl->fun_stmt.name     = strdup(mangled_name);
        cloned_decl->fun_stmt.generics = NULL;
    }

    Symbol *inst_sym = symbol_create(mangled_name, SYMBOL_FUNCTION, sema->module_path);
    inst_sym->decl   = cloned_decl; // link symbol to cloned ast for MIR lowering
    symbol_table_insert(sema->root_table, inst_sym);
    cloned_decl->symbol = inst_sym;

    SymbolTable *scope      = symbol_table_create(sema->current_table);
    SymbolTable *prev_table = sema->current_table;

    for (int i = 0; i < generic_params->count; i++)
    {
        AstNode *param_node = generic_params->items[i];
        AstNode *arg_node   = type_args->items[i];
        if (param_node->kind == AST_TYPE_PARAM)
        {
            Type   *arg_type = sema_resolve_type(sema, arg_node);
            Symbol *type_sym = symbol_create(param_node->type_param.name, SYMBOL_TYPE, sema->module_path);
            type_sym->type   = arg_type;
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

    if (cloned_decl->fun_stmt.params)
    {
        param_count = cloned_decl->fun_stmt.params->count;
        param_types = malloc(sizeof(Type *) * param_count);

        for (int i = 0; i < param_count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Type *pt = NULL;
                if (param->param_stmt.type)
                {
                    pt = sema_resolve_type(sema, param->param_stmt.type);
                    if (pt && pt->kind == TYPE_STRUCT)
                    {
                        for (int j = 0; j < pt->structure.field_count; j++)
                        {
                        }
                    }
                }
                param_types[i] = pt;
                param->type    = pt;
            }
        }
    }

    inst_sym->type = type_create_function(ret_type, param_types, param_count);

    if (cloned_decl->fun_stmt.params)
    {
        for (int i = 0; i < cloned_decl->fun_stmt.params->count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
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

    if (cloned_decl->fun_stmt.body)
    {
        sema_analyze_stmt(sema, cloned_decl->fun_stmt.body);
    }

    sema->current_table = prev_table;

    return inst_sym;
}
