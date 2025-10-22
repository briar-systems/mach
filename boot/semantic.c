#include "semantic.h"
#include "ioutil.h"
#include "lexer.h"
#include "symbol.h"
#include "type.h"
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Type *analyze_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected);
static Type *analyze_lit_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected);
static Type *analyze_null_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected);
static Type *analyze_array_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr);
static bool  ensure_assignable(SemanticDriver *driver, const AnalysisContext *ctx, Type *expected, Type *actual, AstNode *site, const char *what);

void diagnostic_sink_init(DiagnosticSink *sink)
{
    sink->entries      = NULL;
    sink->count        = 0;
    sink->capacity     = 0;
    sink->has_errors   = false;
    sink->has_fatal    = false;
    sink->source_cache = calloc(16, sizeof(SourceCacheEntry *));
    sink->cache_size   = 16;
}

void diagnostic_sink_dnit(DiagnosticSink *sink)
{
    for (size_t i = 0; i < sink->count; i++)
    {
        free(sink->entries[i].message);
        free(sink->entries[i].file_path);
        if (sink->entries[i].token)
            free(sink->entries[i].token);
    }
    free(sink->entries);
    sink->entries  = NULL;
    sink->count    = 0;
    sink->capacity = 0;

    // clean up source cache
    if (sink->source_cache)
    {
        for (size_t i = 0; i < sink->cache_size; i++)
        {
            SourceCacheEntry *entry = sink->source_cache[i];
            while (entry)
            {
                SourceCacheEntry *next = entry->next;
                free(entry->file_path);
                free(entry->source);
                free(entry);
                entry = next;
            }
        }
        free(sink->source_cache);
        sink->source_cache = NULL;
        sink->cache_size   = 0;
    }
}

void diagnostic_emit(DiagnosticSink *sink, DiagnosticLevel level, AstNode *node, const char *file_path, const char *fmt, ...)
{
    if (sink->count >= sink->capacity)
    {
        size_t      new_capacity = sink->capacity ? sink->capacity * 2 : 16;
        Diagnostic *new_entries  = realloc(sink->entries, new_capacity * sizeof(Diagnostic));
        if (!new_entries)
            return;
        sink->entries  = new_entries;
        sink->capacity = new_capacity;
    }

    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    Diagnostic *diag = &sink->entries[sink->count++];
    diag->level      = level;
    diag->message    = strdup(buffer);
    diag->file_path  = file_path ? strdup(file_path) : NULL;

    // copy token info if available (position calculated at print time)
    if (node && node->token)
    {
        diag->token = malloc(sizeof(Token));
        if (diag->token)
        {
            diag->token->kind = node->token->kind;
            diag->token->pos  = node->token->pos;
            diag->token->len  = node->token->len;
        }
        diag->line   = 0; // calculated at print time
        diag->column = 0; // calculated at print time
    }
    else
    {
        diag->token  = NULL;
        diag->line   = 0;
        diag->column = 0;
    }

    if (level == DIAG_ERROR)
        sink->has_errors = true;
}

static unsigned int hash_file_path(const char *path)
{
    unsigned int hash = 5381;
    while (*path)
    {
        hash = ((hash << 5) + hash) + (unsigned char)*path;
        path++;
    }
    return hash;
}

static char *get_cached_source(DiagnosticSink *sink, ModuleManager *module_manager, const char *file_path)
{
    if (!sink->source_cache || !file_path)
        return NULL;

    unsigned int      hash  = hash_file_path(file_path) % sink->cache_size;
    SourceCacheEntry *entry = sink->source_cache[hash];

    while (entry)
    {
        if (strcmp(entry->file_path, file_path) == 0)
            return entry->source;
        entry = entry->next;
    }

    // try to get preprocessed source from module manager first
    char *source = NULL;
    if (module_manager)
    {
        Module *module = module_manager_find_by_file_path(module_manager, file_path);
        if (module && module->source)
        {
            source = module->source;
        }
    }

    // fallback: read original file from disk
    if (!source)
    {
        source = read_file((char *)file_path);
    }

    if (source)
    {
        entry                    = malloc(sizeof(SourceCacheEntry));
        entry->file_path         = strdup(file_path);
        entry->source            = strdup(source); // always strdup for consistent memory management
        entry->next              = sink->source_cache[hash];
        sink->source_cache[hash] = entry;
        return entry->source;
    }

    return NULL;
}

void diagnostic_print_all(DiagnosticSink *sink, ModuleManager *module_manager)
{
    for (size_t i = 0; i < sink->count; i++)
    {
        Diagnostic *diag      = &sink->entries[i];
        const char *level_str = (diag->level == DIAG_ERROR) ? "error" : (diag->level == DIAG_WARNING) ? "warning" : "note";

        // skip note for now
        if (diag->level == DIAG_NOTE)
            continue;

        fprintf(stderr, "%s: %s\n", level_str, diag->message);

        // if we have token and file info, calculate and display location
        if (diag->token && diag->file_path)
        {
            // use cached source (preprocessed if available)
            char *source = get_cached_source(sink, module_manager, diag->file_path);
            if (source)
            {
                // create a temporary lexer for this file to calculate position
                Lexer lexer;
                lexer_init(&lexer, source);

                int   line      = lexer_get_pos_line(&lexer, diag->token->pos);
                int   col       = lexer_get_pos_line_offset(&lexer, diag->token->pos);
                char *line_text = lexer_get_line_text(&lexer, line);

                if (line_text)
                {
                    fprintf(stderr, "%s:%d:%d\n", diag->file_path, line + 1, col);
                    fprintf(stderr, "%5d | %s\n", line + 1, line_text);
                    fprintf(stderr, "      | %*s^\n", col > 1 ? col - 1 : 0, "");
                    free(line_text);
                }
                else
                {
                    // fallback if we can't get line text
                    fprintf(stderr, "%s:%d:%d\n", diag->file_path, line + 1, col);
                }

                lexer_dnit(&lexer);
                // don't free source - it's cached
            }
            else
            {
                // fallback if we can't open the file
                fprintf(stderr, "%s:<unknown position>\n", diag->file_path);
            }
        }
        else if (diag->file_path)
        {
            // no token info, just show file
            fprintf(stderr, "%s\n", diag->file_path);
        }
    }
}

GenericBindingCtx generic_binding_ctx_create(void)
{
    GenericBindingCtx ctx;
    ctx.bindings = NULL;
    ctx.count    = 0;
    ctx.capacity = 0;
    return ctx;
}

void generic_binding_ctx_destroy(GenericBindingCtx *ctx)
{
    free(ctx->bindings);
    ctx->bindings = NULL;
    ctx->count    = 0;
    ctx->capacity = 0;
}

GenericBindingCtx generic_binding_ctx_push(GenericBindingCtx *parent, const char *param_name, Type *concrete_type)
{
    GenericBindingCtx new_ctx;
    new_ctx.count    = parent->count + 1;
    new_ctx.capacity = new_ctx.count;
    new_ctx.bindings = malloc(new_ctx.capacity * sizeof(GenericBinding));

    // copy parent bindings
    if (parent->count > 0)
        memcpy(new_ctx.bindings, parent->bindings, parent->count * sizeof(GenericBinding));

    // add new binding
    new_ctx.bindings[parent->count].param_name    = param_name;
    new_ctx.bindings[parent->count].concrete_type = concrete_type;

    return new_ctx;
}

Type *generic_binding_ctx_lookup(const GenericBindingCtx *ctx, const char *param_name)
{
    if (!param_name)
        return NULL;

    for (size_t i = ctx->count; i > 0; i--)
    {
        if (ctx->bindings[i - 1].param_name && strcmp(ctx->bindings[i - 1].param_name, param_name) == 0)
            return ctx->bindings[i - 1].concrete_type;
    }

    return NULL;
}

#define SPEC_CACHE_INITIAL_BUCKETS 64

static size_t hash_specialization_key(Symbol *generic_symbol, Type **type_args, size_t type_arg_count)
{
    size_t hash = (size_t)generic_symbol;
    for (size_t i = 0; i < type_arg_count; i++)
    {
        hash = hash * 31 + (size_t)type_args[i];
    }
    return hash;
}

// compare types for specialization cache - does NOT resolve aliases
static bool type_equals_strict(Type *a, Type *b)
{
    if (a == b)
        return true;
    if (!a || !b)
        return false;

    if (a->kind != b->kind)
        return false;

    switch (a->kind)
    {
    case TYPE_ALIAS:
    {
        if (b->kind != TYPE_ALIAS)
            return false;
        if (a->name && b->name)
            return strcmp(a->name, b->name) == 0;
        return a->alias.target == b->alias.target && type_equals_strict(a->alias.target, b->alias.target);
    }

    case TYPE_POINTER:
        return type_equals_strict(a->pointer.base, b->pointer.base);

    case TYPE_ARRAY:
        return type_equals_strict(a->array.elem_type, b->array.elem_type);

    case TYPE_FUNCTION:
        if (a->function.param_count != b->function.param_count)
            return false;
        if (a->function.is_variadic != b->function.is_variadic)
            return false;
        if (!type_equals_strict(a->function.return_type, b->function.return_type))
            return false;
        for (size_t i = 0; i < a->function.param_count; i++)
        {
            if (!type_equals_strict(a->function.param_types[i], b->function.param_types[i]))
                return false;
        }
        return true;

    case TYPE_STRUCT:
    case TYPE_UNION:
        if (a->name && b->name)
            return strcmp(a->name, b->name) == 0;
        return a == b;

    default:
        return a == b;
    }
}

static bool keys_equal(const SpecializationKey *a, const SpecializationKey *b)
{
    if (a->generic_symbol != b->generic_symbol)
        return false;
    if (a->type_arg_count != b->type_arg_count)
        return false;
    for (size_t i = 0; i < a->type_arg_count; i++)
    {
        if (!type_equals_strict(a->type_args[i], b->type_args[i]))
            return false;
    }
    return true;
}

void specialization_cache_init(SpecializationCache *cache)
{
    cache->bucket_count = SPEC_CACHE_INITIAL_BUCKETS;
    cache->buckets      = calloc(cache->bucket_count, sizeof(SpecializationEntry *));
    cache->entry_count  = 0;
}

void specialization_cache_dnit(SpecializationCache *cache)
{
    for (size_t i = 0; i < cache->bucket_count; i++)
    {
        SpecializationEntry *entry = cache->buckets[i];
        while (entry)
        {
            SpecializationEntry *next = entry->next;
            free(entry->key.type_args);
            free(entry);
            entry = next;
        }
    }
    free(cache->buckets); // Free the memory allocated for the buckets
    cache->buckets      = NULL;
    cache->bucket_count = 0;
    cache->entry_count  = 0;
}

Symbol *specialization_cache_find(SpecializationCache *cache, Symbol *generic_symbol, Type **type_args, size_t type_arg_count)
{
    size_t hash   = hash_specialization_key(generic_symbol, type_args, type_arg_count);
    size_t bucket = hash % cache->bucket_count;

    SpecializationKey search_key = {generic_symbol, type_args, type_arg_count};

    for (SpecializationEntry *entry = cache->buckets[bucket]; entry; entry = entry->next)
    {
        if (keys_equal(&entry->key, &search_key))
            return entry->specialized_symbol;
    }

    return NULL;
}

void specialization_cache_insert(SpecializationCache *cache, Symbol *generic_symbol, Type **type_args, size_t type_arg_count, Symbol *specialized)
{
    size_t hash   = hash_specialization_key(generic_symbol, type_args, type_arg_count);
    size_t bucket = hash % cache->bucket_count;

    SpecializationEntry *entry = malloc(sizeof(SpecializationEntry));
    entry->key.generic_symbol  = generic_symbol;
    entry->key.type_arg_count  = type_arg_count;
    entry->key.type_args       = malloc(type_arg_count * sizeof(Type *));
    memcpy(entry->key.type_args, type_args, type_arg_count * sizeof(Type *));
    entry->specialized_symbol = specialized;
    entry->next               = cache->buckets[bucket];
    cache->buckets[bucket]    = entry;
    cache->entry_count++;
}

void specialization_cache_foreach(SpecializationCache *cache, void (*callback)(Symbol *specialized, void *user_data), void *user_data)
{
    if (!cache || !callback)
        return;

    for (size_t i = 0; i < cache->bucket_count; i++)
    {
        SpecializationEntry *entry = cache->buckets[i];
        while (entry)
        {
            callback(entry->specialized_symbol, user_data);
            entry = entry->next;
        }
    }
}

void instantiation_queue_init(InstantiationQueue *queue)
{
    queue->head  = NULL;
    queue->tail  = NULL;
    queue->count = 0;
}

void instantiation_queue_dnit(InstantiationQueue *queue)
{
    InstantiationRequest *req = queue->head;
    while (req)
    {
        InstantiationRequest *next = req->next;
        free(req->type_args);
        free(req);
        req = next;
    }
    queue->head  = NULL; // Reset the head of the queue
    queue->tail  = NULL;
    queue->count = 0;
}

void instantiation_queue_push(InstantiationQueue *queue, InstantiationKind kind, Symbol *generic_symbol, Type **type_args, size_t type_arg_count, AstNode *call_site)
{
    InstantiationRequest *req = malloc(sizeof(InstantiationRequest));
    req->kind                 = kind;
    req->generic_symbol       = generic_symbol;
    req->type_arg_count       = type_arg_count;
    req->type_args            = malloc(type_arg_count * sizeof(Type *));
    memcpy(req->type_args, type_args, type_arg_count * sizeof(Type *));
    req->call_site = call_site;
    req->next      = NULL;

    if (queue->tail)
    {
        queue->tail->next = req;
        queue->tail       = req;
    }
    else
    {
        queue->head = queue->tail = req;
    }
    queue->count++;
}

InstantiationRequest *instantiation_queue_pop(InstantiationQueue *queue)
{
    if (!queue->head)
        return NULL;

    InstantiationRequest *req = queue->head;
    queue->head               = req->next;
    if (!queue->head)
        queue->tail = NULL;
    queue->count--;

    return req;
}

static char *sanitize_type_name(const char *name)
{
    if (!name)
        return strdup("anon");

    size_t len    = strlen(name);
    char  *result = malloc(len + 1);
    char  *dst    = result;

    for (size_t i = 0; i < len; i++)
    {
        char c = name[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
        {
            *dst++ = c;
        }
    }
    *dst = '\0';

    return result;
}

static char *type_to_mangled_string(Type *type)
{
    if (!type)
        return strdup("unknown");

    // for aliases with names, use the alias name directly for better readability
    if (type->kind == TYPE_ALIAS && type->name && strlen(type->name) > 0)
        return sanitize_type_name(type->name);

    // for named types (structs, unions), use the name
    if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION) && type->name && strlen(type->name) > 0)
        return sanitize_type_name(type->name);

    switch (type->kind)
    {
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64:
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_F16:
    case TYPE_F32:
    case TYPE_F64:
    case TYPE_PTR:
        return strdup(type->name ? type->name : "type");

    case TYPE_POINTER:
    {
        char *base_str = type_to_mangled_string(type->pointer.base);
        char *result   = malloc(strlen("ptr_") + strlen(base_str) + 1);
        sprintf(result, "ptr_%s", base_str);
        free(base_str);
        return result;
    }

    case TYPE_ARRAY:
    {
        char *elem_str = type_to_mangled_string(type->array.elem_type);
        char *result   = malloc(strlen("arr_") + strlen(elem_str) + 1);
        sprintf(result, "arr_%s", elem_str);
        free(elem_str);
        return result;
    }

    case TYPE_STRUCT:
    case TYPE_UNION:
        return sanitize_type_name(type->name);

    default:
        return strdup("unknown");
    }
}

char *mangle_generic_type(const char *module_name, const char *base_name, Type **type_args, size_t type_arg_count)
{
    if (!base_name)
        base_name = "type";

    char *sanitized_base = sanitize_type_name(base_name);

    // calculate total length with module prefix
    size_t total_len     = 0;
    char  *module_prefix = NULL;
    if (module_name && strlen(module_name) > 0)
    {
        // sanitize module path: std.types.result → std__types__result
        size_t mod_len = strlen(module_name);
        module_prefix  = malloc(mod_len + 1);
        char *dst      = module_prefix;
        for (size_t i = 0; i < mod_len; i++)
        {
            char c = module_name[i];
            if (c == '.')
                *dst++ = '_';
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
                *dst++ = c;
        }
        *dst      = '\0';
        total_len = strlen(module_prefix) + 2; // module + "__"
    }

    total_len += strlen(sanitized_base);

    char **arg_strs = malloc(type_arg_count * sizeof(char *));
    for (size_t i = 0; i < type_arg_count; i++)
    {
        arg_strs[i] = type_to_mangled_string(type_args[i]);
        total_len += 1 + strlen(arg_strs[i]); // '$' + arg
    }

    char  *result = malloc(total_len + 1);
    size_t offset = 0;

    // add module prefix
    if (module_prefix)
    {
        strcpy(result, module_prefix);
        offset           = strlen(module_prefix);
        result[offset++] = '_';
        result[offset++] = '_';
        free(module_prefix);
    }

    // add base name
    strcpy(result + offset, sanitized_base);
    offset += strlen(sanitized_base);

    // add type arguments
    for (size_t i = 0; i < type_arg_count; i++)
    {
        result[offset++] = '$';
        strcpy(result + offset, arg_strs[i]);
        offset += strlen(arg_strs[i]);
        free(arg_strs[i]);
    }
    result[offset] = '\0';

    free(arg_strs);
    free(sanitized_base);
    return result;
}

char *mangle_generic_function(const char *module_name, const char *base_name, Type **type_args, size_t type_arg_count)
{
    // functions use same mangling as types
    return mangle_generic_type(module_name, base_name, type_args, type_arg_count);
}

char *mangle_method(const char *module_name, const char *owner_name, const char *method_name, bool receiver_is_pointer)
{
    const char *mod        = module_name ? module_name : "mod";
    const char *own        = owner_name ? owner_name : "owner";
    const char *mth        = method_name ? method_name : "method";
    const char *ptr_suffix = receiver_is_pointer ? "_ptr" : "";

    // normalize module name: replace dots and slashes with underscores
    size_t mod_len        = strlen(mod);
    char  *normalized_mod = malloc(mod_len + 1);
    for (size_t i = 0; i < mod_len; i++)
    {
        char ch           = mod[i];
        normalized_mod[i] = (ch == '.' || ch == '/') ? '_' : ch;
    }
    normalized_mod[mod_len] = '\0';

    size_t len    = strlen(normalized_mod) + 2 + strlen(own) + 2 + strlen(mth) + strlen(ptr_suffix) + 1;
    char  *result = malloc(len);
    snprintf(result, len, "%s__%s__%s%s", normalized_mod, own, mth, ptr_suffix);

    free(normalized_mod);
    return result;
}

char *mangle_global_symbol(const char *module_name, const char *symbol_name)
{
    const char *mod = module_name ? module_name : "mod";
    const char *sym = symbol_name ? symbol_name : "symbol";

    // normalize module name: replace dots and slashes with underscores
    size_t mod_len        = strlen(mod);
    char  *normalized_mod = malloc(mod_len + 1);
    for (size_t i = 0; i < mod_len; i++)
    {
        char ch           = mod[i];
        normalized_mod[i] = (ch == '.' || ch == '/') ? '_' : ch;
    }
    normalized_mod[mod_len] = '\0';

    size_t len    = strlen(normalized_mod) + 2 + strlen(sym) + 1;
    char  *result = malloc(len);
    snprintf(result, len, "%s__%s", normalized_mod, sym);

    free(normalized_mod);
    return result;
}

AnalysisContext analysis_context_create(Scope *global_scope, Scope *module_scope, const char *module_name, const char *module_path)
{
    AnalysisContext ctx;
    ctx.current_scope    = module_scope ? module_scope : global_scope;
    ctx.module_scope     = module_scope;
    ctx.global_scope     = global_scope;
    ctx.bindings         = generic_binding_ctx_create();
    ctx.module_name      = module_name;
    ctx.file_path        = module_path;
    ctx.current_function = NULL;
    return ctx;
}

AnalysisContext analysis_context_with_scope(const AnalysisContext *parent, Scope *new_scope)
{
    AnalysisContext ctx = *parent;
    ctx.current_scope   = new_scope;
    return ctx;
}

AnalysisContext analysis_context_with_bindings(const AnalysisContext *parent, GenericBindingCtx new_bindings)
{
    AnalysisContext ctx = *parent;
    ctx.bindings        = new_bindings;
    return ctx;
}

AnalysisContext analysis_context_with_function(const AnalysisContext *parent, Symbol *function)
{
    AnalysisContext ctx  = *parent;
    ctx.current_function = function;
    return ctx;
}

SemanticDriver *semantic_driver_create(void)
{
    SemanticDriver *driver = malloc(sizeof(SemanticDriver));
    module_manager_init(&driver->module_manager);
    specialization_cache_init(&driver->spec_cache);
    instantiation_queue_init(&driver->inst_queue);
    diagnostic_sink_init(&driver->diagnostics);
    driver->program_root      = NULL;
    driver->entry_module_name = NULL;
    return driver;
}

void semantic_driver_destroy(SemanticDriver *driver)
{
    symbol_table_dnit(&driver->symbol_table);
    module_manager_dnit(&driver->module_manager);
    specialization_cache_dnit(&driver->spec_cache);
    instantiation_queue_dnit(&driver->inst_queue);
    diagnostic_sink_dnit(&driver->diagnostics);
    free(driver);
}

static bool    analyze_pass_a_declarations(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root);
static bool    analyze_pass_b_signatures(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root);
static bool    analyze_pass_c_bodies(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root);
static Symbol *request_generic_type_instantiation(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *type_node);
static bool    analyze_function_body(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt);
static bool    process_use_statement(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt);
static bool    enqueue_module(SemanticDriver *driver, const char *module_path);
static bool    import_public_symbols(SemanticDriver *driver, const AnalysisContext *ctx, Module *src_module, AstNode *stmt, Scope *alias_scope);
static Symbol *clone_imported_symbol(Symbol *source, const char *module_name);

// helper macro to iterate all modules in hash table
#define FOR_EACH_MODULE(manager, module_var)         \
    for (int _i = 0; _i < (manager)->capacity; _i++) \
        for (Module *module_var = (manager)->modules[_i]; module_var != NULL; module_var = module_var->next)

// load module (parse only, no analysis)
static Module *load_module_deferred(SemanticDriver *driver, const char *module_path)
{
    // check if already loaded
    Module *existing = module_manager_find_module(&driver->module_manager, module_path);
    if (existing)
        return existing;

    // use module manager to load and parse
    Module *module = module_manager_load_module(&driver->module_manager, module_path);
    if (!module)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "failed to load module '%s'", module_path);
        return NULL;
    }

    // initialize symbol table if not present
    if (!module->symbols)
    {
        module->symbols = malloc(sizeof(SymbolTable));
        symbol_table_init(module->symbols);
    }

    if (!module->symbols->global_scope)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "module '%s' has invalid symbol table", module_path);
        return NULL;
    }

    return module;
}

static Type *resolve_type_in_context(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *type_node)
{
    if (!type_node)
        return NULL;

    switch (type_node->kind)
    {
    case AST_TYPE_NAME:
    {
        const char *name = type_node->type_name.name;

        // check for generic binding first - this resolves type parameters like T, E, etc.
        Type *bound = generic_binding_ctx_lookup(&ctx->bindings, name);
        if (bound)
            return bound;

        // check for generic instantiation (must come before caching)
        if (type_node->type_name.generic_args && type_node->type_name.generic_args->count > 0)
        {
            Symbol *specialized = request_generic_type_instantiation(driver, ctx, type_node);
            if (specialized && specialized->type)
            {
                // don't cache specialized types on AST nodes to avoid cross-contamination
                return specialized->type;
            }
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "failed to instantiate generic type '%s'", name);
            return NULL;
        }

        // cache if already resolved (only for non-generic, non-parameterized references)
        if (type_node->type && !ctx->bindings.count)
            return type_node->type;

        // check builtin types
        Type *builtin = type_lookup_builtin(name);
        if (builtin)
        {
            // only cache builtins when not in generic context
            if (!ctx->bindings.count)
                type_node->type = builtin;
            return builtin;
        }

        // look up user-defined type in scope
        Symbol *type_sym = symbol_lookup_scope(ctx->current_scope, name);
        if (!type_sym)
            type_sym = symbol_lookup_scope(ctx->module_scope, name);
        if (!type_sym)
            type_sym = symbol_lookup_scope(ctx->global_scope, name);

        if (type_sym && type_sym->kind == SYMBOL_TYPE)
        {
            // only cache when not in generic context
            if (!ctx->bindings.count)
                type_node->type = type_sym->type;
            return type_sym->type;
        }

        // provide more context for debugging generic instantiation issues
        if (ctx->bindings.count > 0)
        {
            // in generic context - list what we DO have bound
            char   bindings_info[256] = {0};
            size_t offset             = 0;
            for (size_t i = 0; i < ctx->bindings.count && i < 3 && ctx->bindings.bindings; i++)
            {
                if (i > 0)
                    offset += snprintf(bindings_info + offset, sizeof(bindings_info) - offset, ", ");
                const char *bound_name = ctx->bindings.bindings[i].param_name ? ctx->bindings.bindings[i].param_name : "?";
                const char *bound_type = ctx->bindings.bindings[i].concrete_type && ctx->bindings.bindings[i].concrete_type->name ? ctx->bindings.bindings[i].concrete_type->name : "<?>";
                offset += snprintf(bindings_info + offset, sizeof(bindings_info) - offset, "%s=%s", bound_name, bound_type);
            }
            if (ctx->bindings.count > 3)
                offset += snprintf(bindings_info + offset, sizeof(bindings_info) - offset, ", ...");

            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "unknown type '%s' during generic instantiation (have bindings: %s)", name, bindings_info);
        }
        else
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "unknown type '%s'", name);
        }
        return NULL;
    }

    case AST_TYPE_PTR:
    {
        Type *base = resolve_type_in_context(driver, ctx, type_node->type_ptr.base);
        if (!base)
            return NULL;
        Type *ptr       = type_pointer_create(base);
        type_node->type = ptr;
        return ptr;
    }

    case AST_TYPE_ARRAY:
    {
        Type *elem = resolve_type_in_context(driver, ctx, type_node->type_array.elem_type);
        if (!elem)
            return NULL;

        Type *arr = NULL;
        if (type_node->type_array.size)
        {
            // fixed-size array [N]T - evaluate size
            // for now, only support integer literals
            if (type_node->type_array.size->kind != AST_EXPR_LIT || type_node->type_array.size->lit_expr.kind != TOKEN_LIT_INT)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "array size must be an integer literal");
                return NULL;
            }
            int64_t size_val = type_node->type_array.size->lit_expr.int_val;
            if (size_val <= 0)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "array size must be positive");
                return NULL;
            }
            arr = type_fixed_array_create(elem, (size_t)size_val);
        }
        else
        {
            // slice/fat pointer []T
            arr = type_array_create(elem);
        }

        type_node->type = arr;
        return arr;
    }

    case AST_TYPE_FUN:
    {
        Type *ret_type = NULL;
        if (type_node->type_fun.return_type)
        {
            ret_type = resolve_type_in_context(driver, ctx, type_node->type_fun.return_type);
            if (!ret_type)
                return NULL;
        }

        size_t param_count = type_node->type_fun.params ? type_node->type_fun.params->count : 0;
        Type **param_types = NULL;
        if (param_count > 0)
        {
            param_types = malloc(sizeof(Type *) * param_count);
            for (size_t i = 0; i < param_count; i++)
            {
                param_types[i] = resolve_type_in_context(driver, ctx, type_node->type_fun.params->items[i]);
                if (!param_types[i])
                {
                    free(param_types);
                    return NULL;
                }
            }
        }

        Type *func = type_function_create(ret_type, param_types, param_count, type_node->type_fun.is_variadic);
        free(param_types);
        type_node->type = func;
        return func;
    }

    case AST_TYPE_STR:
    {
        // named struct - look it up
        if (type_node->type_str.name)
        {
            Symbol *sym = symbol_lookup_scope(ctx->current_scope, type_node->type_str.name);
            if (!sym || sym->kind != SYMBOL_TYPE)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "unknown type '%s'", type_node->type_str.name);
                return NULL;
            }
            type_node->type = sym->type;
            return sym->type;
        }

        // anonymous struct - create inline
        Type   *struct_type = type_struct_create(NULL);
        Symbol *head        = NULL;
        Symbol *tail        = NULL;
        size_t  offset      = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        if (type_node->type_str.fields)
        {
            for (int i = 0; i < type_node->type_str.fields->count; i++)
            {
                AstNode *field_node = type_node->type_str.fields->items[i];
                Type    *field_type = resolve_type_in_context(driver, ctx, field_node->field_stmt.type);
                if (!field_type)
                {
                    // cleanup
                    Symbol *curr = head;
                    while (curr)
                    {
                        Symbol *next = curr->next;
                        symbol_destroy(curr);
                        curr = next;
                    }
                    free(struct_type);
                    return NULL;
                }

                // resolve alias to get actual type with proper size
                Type *resolved_type = type_resolve_alias(field_type);
                if (resolved_type)
                    field_type = resolved_type;

                Symbol *field_symbol = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);

                size_t field_align         = type_alignof(field_type);
                size_t align_val           = field_align ? field_align : 1;
                offset                     = (offset + align_val - 1) / align_val * align_val; // align offset
                field_symbol->field.offset = offset;

                if (!head)
                {
                    head = tail = field_symbol;
                }
                else
                {
                    tail->next = field_symbol;
                    tail       = field_symbol;
                }

                offset += type_sizeof(field_type);
                if (field_align > max_align)
                    max_align = field_align;
                field_count++;
            }
        }

        size_t final_align = max_align ? max_align : 1;
        offset             = (offset + final_align - 1) / final_align * final_align; // align struct size

        struct_type->size                  = offset;
        struct_type->alignment             = final_align;
        struct_type->composite.fields      = head;
        struct_type->composite.field_count = field_count;

        type_node->type = struct_type;
        return struct_type;
    }

    case AST_TYPE_UNI:
    {
        // named union - look it up
        if (type_node->type_uni.name)
        {
            Symbol *sym = symbol_lookup_scope(ctx->current_scope, type_node->type_uni.name);
            if (!sym || sym->kind != SYMBOL_TYPE)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "unknown type '%s'", type_node->type_uni.name);
                return NULL;
            }
            type_node->type = sym->type;
            return sym->type;
        }

        // anonymous union - create inline
        Type   *union_type  = type_union_create(NULL);
        Symbol *head        = NULL;
        Symbol *tail        = NULL;
        size_t  max_size    = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        if (type_node->type_uni.fields)
        {
            for (int i = 0; i < type_node->type_uni.fields->count; i++)
            {
                AstNode *field_node = type_node->type_uni.fields->items[i];
                Type    *field_type = resolve_type_in_context(driver, ctx, field_node->field_stmt.type);
                if (!field_type)
                {
                    // cleanup
                    Symbol *curr = head;
                    while (curr)
                    {
                        Symbol *next = curr->next;
                        symbol_destroy(curr);
                        curr = next;
                    }
                    free(union_type);
                    return NULL;
                }

                // resolve alias to get actual type with proper size
                Type *resolved_type = type_resolve_alias(field_type);
                if (resolved_type)
                    field_type = resolved_type;

                Symbol *field_symbol       = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);
                field_symbol->field.offset = 0; // all union fields start at offset 0

                if (!head)
                {
                    head = tail = field_symbol;
                }
                else
                {
                    tail->next = field_symbol;
                    tail       = field_symbol;
                }

                size_t field_size  = type_sizeof(field_type);
                size_t field_align = type_alignof(field_type);
                if (field_size > max_size)
                    max_size = field_size;
                if (field_align > max_align)
                    max_align = field_align;
                field_count++;
            }
        }

        size_t final_align = max_align ? max_align : 1;
        size_t final_size  = (max_size + final_align - 1) / final_align * final_align; // align to max_align

        union_type->size                  = final_size;
        union_type->alignment             = final_align;
        union_type->composite.fields      = head;
        union_type->composite.field_count = field_count;

        type_node->type = union_type;
        return union_type;
    }

    default:
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, type_node, ctx->file_path, "unsupported type kind in resolution");
        return NULL;
    }
}

// enqueue module for analysis (loads and collects dependencies)
static bool enqueue_module(SemanticDriver *driver, const char *module_path)
{
    Module *module = load_module_deferred(driver, module_path);
    if (!module)
        return false;

    // scan for use statements and enqueue dependencies
    if (module->ast && module->ast->kind == AST_PROGRAM)
    {
        for (int i = 0; i < module->ast->program.stmts->count; i++)
        {
            AstNode *stmt = module->ast->program.stmts->items[i];
            if (stmt->kind == AST_STMT_USE)
            {
                const char *dep_path = stmt->use_stmt.module_path;
                if (dep_path)
                {
                    // recursively enqueue dependencies
                    if (!enqueue_module(driver, dep_path))
                        return false;
                }
            }
        }
    }

    return true;
}

// import public symbols from module into current scope
static bool import_public_symbols(SemanticDriver *driver, const AnalysisContext *ctx, Module *src_module, AstNode *stmt, Scope *alias_scope)
{
    if (!src_module || !src_module->symbols)
        return false;

    Scope *export_scope = src_module->symbols->module_scope ? src_module->symbols->module_scope : src_module->symbols->global_scope;
    if (!export_scope)
        return false;

    bool import_into_current_scope = (alias_scope == NULL);

    for (Symbol *sym = export_scope->symbols; sym; sym = sym->next)
    {
        // skip modules and non-public symbols
        if (sym->kind == SYMBOL_MODULE || !sym->is_public)
            continue;

        // skip imported symbols (don't re-export transitive imports)
        if (sym->is_imported)
            continue;

        if (import_into_current_scope)
        {
            Symbol *existing = symbol_lookup_scope(ctx->current_scope, sym->name);
            if (existing)
            {
                // allow re-importing the same symbol from the same module (idempotent)
                if (existing->is_imported && existing->import_module && src_module->name && strcmp(existing->import_module, src_module->name) == 0)
                {
                    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "symbol '%s' conflicts with existing declaration", sym->name);
                    return false;
                }
                continue;
            }

            Symbol *imported = clone_imported_symbol(sym, src_module->name);
            if (!imported)
                return false;

            imported->home_scope = ctx->current_scope;
            symbol_add(ctx->current_scope, imported);
        }

        if (alias_scope)
        {
            if (!symbol_lookup_scope(alias_scope, sym->name))
            {
                Symbol *alias_copy = clone_imported_symbol(sym, src_module->name);
                if (!alias_copy)
                    return false;
                alias_copy->home_scope = alias_scope;
                symbol_add(alias_scope, alias_copy);
            }
        }
    }

    return true;
}

static Symbol *clone_imported_symbol(Symbol *source, const char *module_name)
{
    if (!source)
        return NULL;

    Symbol *clone = symbol_create(source->kind, source->name, source->type, source->decl);
    if (!clone)
        return NULL;

    clone->is_imported   = true;
    clone->is_public     = source->is_public;
    clone->has_const_i64 = source->has_const_i64;
    clone->const_i64     = source->const_i64;
    clone->import_module = module_name;
    clone->import_origin = source->import_origin ? source->import_origin : source;

    free(clone->module_name);
    if (source->module_name)
        clone->module_name = strdup(source->module_name);
    else if (module_name)
        clone->module_name = strdup(module_name);
    else
        clone->module_name = NULL;

    switch (source->kind)
    {
    case SYMBOL_VAR:
    case SYMBOL_VAL:
        clone->var.is_global = source->var.is_global;
        clone->var.is_const  = source->var.is_const;
        free(clone->var.mangled_name);
        clone->var.mangled_name = source->var.mangled_name ? strdup(source->var.mangled_name) : NULL;
        break;
    case SYMBOL_FUNC:
        clone->func.is_external                    = source->func.is_external;
        clone->func.is_defined                     = source->func.is_defined;
        clone->func.uses_mach_varargs              = source->func.uses_mach_varargs;
        clone->func.extern_name                    = source->func.extern_name ? strdup(source->func.extern_name) : NULL;
        clone->func.convention                     = source->func.convention ? strdup(source->func.convention) : NULL;
        clone->func.mangled_name                   = source->func.mangled_name ? strdup(source->func.mangled_name) : NULL;
        clone->func.is_generic                     = source->func.is_generic;
        clone->func.generic_param_count            = source->func.generic_param_count;
        clone->func.generic_specializations        = NULL;
        clone->func.is_specialized_instance        = source->func.is_specialized_instance;
        clone->func.is_method                      = source->func.is_method;
        clone->func.method_owner                   = source->func.method_owner;
        clone->func.method_forwarded_generic_count = source->func.method_forwarded_generic_count;
        clone->func.method_receiver_is_pointer     = source->func.method_receiver_is_pointer;
        clone->func.method_receiver_name           = source->func.method_receiver_name ? strdup(source->func.method_receiver_name) : NULL;
        if (source->func.generic_param_count > 0 && source->func.generic_param_names)
        {
            clone->func.generic_param_names = malloc(sizeof(char *) * source->func.generic_param_count);
            if (!clone->func.generic_param_names)
            {
                symbol_destroy(clone);
                return NULL;
            }
            for (size_t i = 0; i < source->func.generic_param_count; i++)
            {
                clone->func.generic_param_names[i] = source->func.generic_param_names[i] ? strdup(source->func.generic_param_names[i]) : NULL;
            }
        }
        else
        {
            clone->func.generic_param_names = NULL;
        }
        break;
    case SYMBOL_TYPE:
        clone->type_def.is_alias                = source->type_def.is_alias;
        clone->type_def.is_generic              = source->type_def.is_generic;
        clone->type_def.generic_param_count     = source->type_def.generic_param_count;
        clone->type_def.generic_specializations = NULL;
        clone->type_def.is_specialized_instance = source->type_def.is_specialized_instance;
        clone->type_def.methods                 = NULL;
        if (source->type_def.generic_param_count > 0 && source->type_def.generic_param_names)
        {
            clone->type_def.generic_param_names = malloc(sizeof(char *) * source->type_def.generic_param_count);
            if (!clone->type_def.generic_param_names)
            {
                symbol_destroy(clone);
                return NULL;
            }
            for (size_t i = 0; i < source->type_def.generic_param_count; i++)
            {
                clone->type_def.generic_param_names[i] = source->type_def.generic_param_names[i] ? strdup(source->type_def.generic_param_names[i]) : NULL;
            }
        }
        else
        {
            clone->type_def.generic_param_names = NULL;
        }

        for (Symbol *method = source->type_def.methods; method; method = method->method_next)
        {
            Symbol *method_clone = clone_imported_symbol(method, module_name);
            if (!method_clone)
            {
                symbol_destroy(clone);
                return NULL;
            }
            method_clone->func.method_owner = clone;
            method_clone->method_next       = NULL;
            type_add_method(clone, method_clone);
        }
        break;
    case SYMBOL_FIELD:
        clone->field.offset = source->field.offset;
        break;
    case SYMBOL_PARAM:
        clone->param.index = source->param.index;
        break;
    case SYMBOL_MODULE:
        // modules should not be cloned in this path; but handle gracefully
        if (source->module.path)
            clone->module.path = strdup(source->module.path);
        if (source->module.scope)
            clone->module.scope = source->module.scope;
        break;
    }

    return clone;
}

// process use statement (import resolution phase - module must already be loaded)
static bool process_use_statement(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *module_path = stmt->use_stmt.module_path;
    const char *alias       = stmt->use_stmt.alias;

    // find the module (should already be loaded and analyzed)
    Module *module = module_manager_find_module(&driver->module_manager, module_path);
    if (!module)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "module '%s' not found (should have been loaded)", module_path);
        return false;
    }

    Scope *alias_scope = NULL;
    if (alias)
    {
        // aliased import: use io: std.io
        // create isolated module namespace scope
        // check for name collision
        Symbol *existing = symbol_lookup_scope(ctx->current_scope, alias);
        if (existing)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "alias '%s' conflicts with existing symbol", alias);
            return false;
        }

        Symbol *ns_symbol = symbol_create_module(alias, module_path);
        if (!ns_symbol)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "failed to create alias scope for module '%s'", module_path);
            return false;
        }

        ns_symbol->is_imported             = true;
        ns_symbol->is_public               = false;
        ns_symbol->import_module           = module_path;
        ns_symbol->module.scope->parent    = ctx->current_scope;
        ns_symbol->module.scope->is_module = true;

        symbol_add(ctx->current_scope, ns_symbol);
        alias_scope = ns_symbol->module.scope;

        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "module '%s' aliased as '%s'", module_path, alias);
    }

    // import public symbols (either into current scope or alias scope)
    if (!import_public_symbols(driver, ctx, module, stmt, alias_scope))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "failed to import symbols from module '%s'", module_path);
        return false;
    }

    if (!alias)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "imported public symbols from module '%s'", module_path);
    }

    return true;
}

// declare type alias
static bool declare_def_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->def_stmt.name;

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of '%s'", name);
        return false;
    }

    // create placeholder - signature will be resolved in pass B
    Type   *placeholder         = type_alias_create(name, NULL);
    Symbol *symbol              = symbol_create(SYMBOL_TYPE, name, placeholder, stmt);
    symbol->type_def.is_alias   = true;
    symbol->type_def.is_generic = false;
    symbol->is_public           = stmt->def_stmt.is_public;
    symbol->module_name         = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope          = ctx->current_scope;

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

// declare struct
static bool declare_str_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->str_stmt.name;

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of struct '%s'", name);
        return false;
    }

    // create struct type - fields resolved in pass B
    Type   *struct_type       = type_struct_create(name);
    Symbol *symbol            = symbol_create(SYMBOL_TYPE, name, struct_type, stmt);
    symbol->type_def.is_alias = false;
    symbol->is_public         = stmt->str_stmt.is_public;
    symbol->module_name       = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope        = ctx->current_scope;

    // check if generic
    if (stmt->str_stmt.generics && stmt->str_stmt.generics->count > 0)
    {
        // mark as generic template - actual struct creation happens during instantiation
        symbol->type_def.is_generic          = true;
        symbol->type_def.generic_param_count = stmt->str_stmt.generics->count;

        // store generic parameter names for later instantiation
        size_t param_count                   = (size_t)stmt->str_stmt.generics->count;
        symbol->type_def.generic_param_names = malloc(sizeof(char *) * param_count);
        if (symbol->type_def.generic_param_names)
        {
            for (size_t i = 0; i < param_count; i++)
            {
                AstNode    *type_param                  = stmt->str_stmt.generics->items[i];
                const char *param_name                  = type_param->type_param.name;
                symbol->type_def.generic_param_names[i] = param_name ? strdup(param_name) : NULL;
            }
        }

        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "generic struct '%s' registered", name);
    }
    else
    {
        symbol->type_def.is_generic = false;
    }

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

// declare union
static bool declare_uni_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->uni_stmt.name;

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of union '%s'", name);
        return false;
    }

    // create union type - fields resolved in pass B
    Type   *union_type        = type_union_create(name);
    Symbol *symbol            = symbol_create(SYMBOL_TYPE, name, union_type, stmt);
    symbol->type_def.is_alias = false;
    symbol->is_public         = stmt->uni_stmt.is_public;
    symbol->module_name       = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope        = ctx->current_scope;

    // check if generic
    if (stmt->uni_stmt.generics && stmt->uni_stmt.generics->count > 0)
    {
        symbol->type_def.is_generic          = true;
        symbol->type_def.generic_param_count = stmt->uni_stmt.generics->count;

        // store generic parameter names for later instantiation
        size_t param_count                   = (size_t)stmt->uni_stmt.generics->count;
        symbol->type_def.generic_param_names = malloc(sizeof(char *) * param_count);
        if (symbol->type_def.generic_param_names)
        {
            for (size_t i = 0; i < param_count; i++)
            {
                AstNode    *type_param                  = stmt->uni_stmt.generics->items[i];
                const char *param_name                  = type_param->type_param.name;
                symbol->type_def.generic_param_names[i] = param_name ? strdup(param_name) : NULL;
            }
        }

        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "generic union '%s' registered", name);
    }
    else
    {
        symbol->type_def.is_generic = false;
    }

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

// declare method
static bool declare_method_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *method_name = stmt->fun_stmt.name;

    // extract owner type name from method_receiver
    if (!stmt->fun_stmt.method_receiver || stmt->fun_stmt.method_receiver->kind != AST_TYPE_NAME)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "method '%s' missing valid receiver type", method_name);
        return false;
    }

    const char *owner_name = stmt->fun_stmt.method_receiver->type_name.name;

    // find owner type symbol
    Symbol *owner_sym = symbol_lookup_scope(ctx->current_scope, owner_name);
    if (!owner_sym)
        owner_sym = symbol_lookup_scope(ctx->module_scope, owner_name);
    if (!owner_sym)
        owner_sym = symbol_lookup_scope(ctx->global_scope, owner_name);

    if (!owner_sym || owner_sym->kind != SYMBOL_TYPE)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "method owner type '%s' not found", owner_name);
        return false;
    }

    // check if method or owner is generic
    bool   is_generic           = (stmt->fun_stmt.generics && stmt->fun_stmt.generics->count > 0) || (owner_sym->type_def.is_generic);
    size_t method_generic_count = stmt->fun_stmt.generics ? (size_t)stmt->fun_stmt.generics->count : 0;
    size_t owner_generic_count  = owner_sym->type_def.is_generic ? owner_sym->type_def.generic_param_count : 0;
    size_t total_generic_count  = method_generic_count + owner_generic_count;

    if (is_generic)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "generic method '%s.%s' registered (%zu owner params + %zu method params)", owner_name, method_name, owner_generic_count, method_generic_count);
    }

    // create method symbol - signature resolved in pass B
    // mangle method name with owner type: Type.method → Type__method
    char mangled[512];
    snprintf(mangled, sizeof(mangled), "%s__%s", owner_name, method_name);

    Type   *placeholder                         = type_function_create(NULL, NULL, 0, stmt->fun_stmt.is_variadic);
    Symbol *symbol                              = symbol_create(SYMBOL_FUNC, strdup(mangled), placeholder, stmt);
    symbol->func.is_external                    = false;
    symbol->func.is_method                      = true;
    symbol->func.is_defined                     = (stmt->fun_stmt.body != NULL);
    symbol->func.is_generic                     = is_generic;
    symbol->func.uses_mach_varargs              = stmt->fun_stmt.is_variadic; // mark variadic methods
    symbol->func.method_owner                   = owner_sym;
    symbol->func.method_forwarded_generic_count = owner_generic_count;

    // store all generic parameter names (owner params first, then method params)
    if (is_generic && total_generic_count > 0)
    {
        symbol->func.generic_param_count = total_generic_count;
        symbol->func.generic_param_names = malloc(sizeof(char *) * total_generic_count);
        if (symbol->func.generic_param_names)
        {
            size_t idx = 0;

            // first, add owner's type parameters
            if (owner_sym->type_def.is_generic && owner_sym->type_def.generic_param_names)
            {
                for (size_t i = 0; i < owner_generic_count; i++)
                {
                    symbol->func.generic_param_names[idx++] = owner_sym->type_def.generic_param_names[i] ? strdup(owner_sym->type_def.generic_param_names[i]) : NULL;
                }
            }

            // then, add method's own type parameters
            if (stmt->fun_stmt.generics)
            {
                for (size_t i = 0; i < method_generic_count; i++)
                {
                    AstNode    *type_param                  = stmt->fun_stmt.generics->items[i];
                    const char *param_name                  = type_param->type_param.name;
                    symbol->func.generic_param_names[idx++] = param_name ? strdup(param_name) : NULL;
                }
            }
        }
    }
    symbol->is_public   = stmt->fun_stmt.is_public;
    symbol->module_name = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope  = ctx->current_scope;

    // set mangled name for methods: check for #@symbol directive, otherwise use module__Type__method
    if (stmt->fun_stmt.mangle_name && stmt->fun_stmt.mangle_name[0])
    {
        // explicit symbol name from #@symbol directive
        symbol->func.extern_name = strdup(stmt->fun_stmt.mangle_name);
    }
    else if (!is_generic && ctx->module_name && ctx->module_name[0])
    {
        // non-generic methods get mangled with module name and owner type
        symbol->func.mangled_name = mangle_method(ctx->module_name, owner_name, method_name, false);
    }

    // add to current scope with mangled name
    symbol_add(ctx->current_scope, symbol);

    stmt->symbol = symbol;
    return true;
}

// declare function
static bool declare_fun_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->fun_stmt.name;

    // check if method - handle separately
    if (stmt->fun_stmt.is_method)
        return declare_method_stmt(driver, ctx, stmt);

    // check if generic
    bool is_generic = (stmt->fun_stmt.generics && stmt->fun_stmt.generics->count > 0);

    if (is_generic)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_NOTE, stmt, ctx->file_path, "generic function '%s' registered", name);
    }

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of function '%s'", name);
        return false;
    }

    // create function symbol - signature resolved in pass B
    Type   *placeholder            = type_function_create(NULL, NULL, 0, stmt->fun_stmt.is_variadic);
    Symbol *symbol                 = symbol_create(SYMBOL_FUNC, name, placeholder, stmt);
    symbol->func.is_external       = false; // regular functions are not external
    symbol->func.is_defined        = (stmt->fun_stmt.body != NULL);
    symbol->func.is_generic        = is_generic;
    symbol->func.uses_mach_varargs = stmt->fun_stmt.is_variadic; // mark variadic functions
    if (is_generic)
    {
        size_t generic_count             = (size_t)stmt->fun_stmt.generics->count;
        symbol->func.generic_param_count = generic_count;
        symbol->func.generic_param_names = malloc(sizeof(char *) * generic_count);
        if (symbol->func.generic_param_names)
        {
            for (size_t i = 0; i < generic_count; i++)
            {
                AstNode    *type_param              = stmt->fun_stmt.generics->items[i];
                const char *param_name              = type_param->type_param.name;
                symbol->func.generic_param_names[i] = param_name ? strdup(param_name) : NULL;
            }
        }
    }
    symbol->is_public   = stmt->fun_stmt.is_public;
    symbol->module_name = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope  = ctx->current_scope;

    // set mangled name: use #@symbol directive if present, otherwise mangle with module name
    if (stmt->fun_stmt.mangle_name && stmt->fun_stmt.mangle_name[0])
    {
        // explicit symbol name from #@symbol directive
        symbol->func.extern_name = strdup(stmt->fun_stmt.mangle_name);
    }
    else if (!is_generic && ctx->module_name && ctx->module_name[0])
    {
        // non-generic functions get mangled with module name
        symbol->func.mangled_name = mangle_global_symbol(ctx->module_name, name);
    }

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

// declare external function
static bool declare_ext_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->ext_stmt.name;

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of external function '%s'", name);
        return false;
    }

    // create external function symbol - signature resolved in pass B
    Type   *placeholder      = type_function_create(NULL, NULL, 0, false);
    Symbol *symbol           = symbol_create(SYMBOL_FUNC, name, placeholder, stmt);
    symbol->func.is_external = true;
    symbol->func.is_defined  = false; // external functions have no body
    symbol->func.is_generic  = false;
    symbol->is_public        = stmt->ext_stmt.is_public;
    symbol->module_name      = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope       = ctx->current_scope;

    // external functions use the explicit symbol name if provided
    if (stmt->ext_stmt.symbol && stmt->ext_stmt.symbol[0])
    {
        symbol->func.extern_name = strdup(stmt->ext_stmt.symbol);
    }

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

// declare global variable
static bool declare_var_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name = stmt->var_stmt.name;

    // check for redefinition
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of '%s'", name);
        return false;
    }

    // create variable symbol - type resolved in pass B
    Symbol *symbol        = symbol_create(stmt->var_stmt.is_val ? SYMBOL_VAL : SYMBOL_VAR, name, NULL, stmt);
    symbol->var.is_global = true;
    symbol->var.is_const  = stmt->var_stmt.is_val;
    symbol->is_public     = stmt->var_stmt.is_public;
    symbol->module_name   = ctx->module_name ? strdup(ctx->module_name) : NULL;
    symbol->home_scope    = ctx->current_scope;

    // mangle global variable/constant names with module prefix to avoid collisions
    if (ctx->module_name && ctx->module_name[0])
    {
        symbol->var.mangled_name = mangle_global_symbol(ctx->module_name, name);
    }

    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;
    return true;
}

static bool analyze_pass_a_declarations(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root)
{
    if (!root || root->kind != AST_PROGRAM)
        return false;

    bool success = true;

    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        switch (stmt->kind)
        {
        case AST_STMT_DEF:
            if (!declare_def_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_STR:
            if (!declare_str_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_UNI:
            if (!declare_uni_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_FUN:
            if (!declare_fun_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_EXT:
            if (!declare_ext_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_VAL:
        case AST_STMT_VAR:
            if (!declare_var_stmt(driver, ctx, stmt))
                success = false;
            break;

        case AST_STMT_USE:
            // defer use statement processing until after Pass A
            // this prevents infinite recursion during module loading
            break;

        default:
            break;
        }
    }

    return success;
}

// resolve struct fields
static bool resolve_str_fields(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol || !stmt->symbol->type || stmt->symbol->type->kind != TYPE_STRUCT)
        return false;

    Type *struct_type = stmt->symbol->type;

    if (!stmt->str_stmt.fields || stmt->str_stmt.fields->count == 0)
        return true; // empty struct is valid

    Symbol *field_list_head = NULL;
    Symbol *field_list_tail = NULL;
    size_t  field_count     = 0;
    size_t  offset          = 0;
    size_t  max_alignment   = 1;

    for (int i = 0; i < stmt->str_stmt.fields->count; i++)
    {
        AstNode *field_node = stmt->str_stmt.fields->items[i];
        if (field_node->kind != AST_STMT_FIELD)
            continue;

        const char *field_name = field_node->field_stmt.name;
        Type       *field_type = resolve_type_in_context(driver, ctx, field_node->field_stmt.type);

        if (!field_type)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, field_node, ctx->file_path, "cannot resolve type for field '%s'", field_name);
            return false;
        }

        // create field symbol
        Symbol *field_sym = symbol_create(SYMBOL_FIELD, field_name, field_type, field_node);

        // calculate offset with alignment
        size_t field_align = type_alignof(field_type);
        if (field_align > 0)
        {
            size_t remainder = offset % field_align;
            if (remainder != 0)
                offset += field_align - remainder;
        }
        field_sym->field.offset = offset;

        // link to list
        if (!field_list_head)
        {
            field_list_head = field_list_tail = field_sym;
        }
        else
        {
            field_list_tail->next = field_sym;
            field_list_tail       = field_sym;
        }

        offset += type_sizeof(field_type);
        if (field_align > max_alignment)
            max_alignment = field_align;
        field_count++;

        field_node->symbol = field_sym;
        field_node->type   = field_type;
    }

    // align final struct size
    if (max_alignment > 0)
    {
        size_t remainder = offset % max_alignment;
        if (remainder != 0)
            offset += max_alignment - remainder;
    }

    struct_type->composite.fields      = field_list_head;
    struct_type->composite.field_count = field_count;
    struct_type->size                  = offset;
    struct_type->alignment             = max_alignment;

    return true;
}

// resolve union variants
static bool resolve_uni_fields(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol || !stmt->symbol->type || stmt->symbol->type->kind != TYPE_UNION)
        return false;

    Type *union_type = stmt->symbol->type;

    if (!stmt->uni_stmt.fields || stmt->uni_stmt.fields->count == 0)
        return true; // empty union is valid

    Symbol *field_list_head = NULL;
    Symbol *field_list_tail = NULL;
    size_t  field_count     = 0;
    size_t  max_size        = 0;
    size_t  max_alignment   = 1;

    for (int i = 0; i < stmt->uni_stmt.fields->count; i++)
    {
        AstNode *field_node = stmt->uni_stmt.fields->items[i];
        if (field_node->kind != AST_STMT_FIELD)
            continue;

        const char *field_name = field_node->field_stmt.name;
        Type       *field_type = resolve_type_in_context(driver, ctx, field_node->field_stmt.type);

        if (!field_type)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, field_node, ctx->file_path, "cannot resolve type for field '%s'", field_name);
            return false;
        }

        // create field symbol (all at offset 0 in union)
        Symbol *field_sym       = symbol_create(SYMBOL_FIELD, field_name, field_type, field_node);
        field_sym->field.offset = 0;

        // link to list
        if (!field_list_head)
        {
            field_list_head = field_list_tail = field_sym;
        }
        else
        {
            field_list_tail->next = field_sym;
            field_list_tail       = field_sym;
        }

        size_t field_size  = type_sizeof(field_type);
        size_t field_align = type_alignof(field_type);

        if (field_size > max_size)
            max_size = field_size;
        if (field_align > max_alignment)
            max_alignment = field_align;
        field_count++;

        field_node->symbol = field_sym;
        field_node->type   = field_type;
    }

    // align final union size
    if (max_alignment > 0)
    {
        size_t remainder = max_size % max_alignment;
        if (remainder != 0)
            max_size += max_alignment - remainder;
    }

    union_type->composite.fields      = field_list_head;
    union_type->composite.field_count = field_count;
    union_type->size                  = max_size;
    union_type->alignment             = max_alignment;

    return true;
}

// resolve function signature
static bool resolve_fun_signature(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol || stmt->symbol->kind != SYMBOL_FUNC)
        return false;

    // resolve return type
    Type *return_type = NULL;
    if (stmt->fun_stmt.return_type)
    {
        return_type = resolve_type_in_context(driver, ctx, stmt->fun_stmt.return_type);
        if (!return_type)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "cannot resolve return type for function '%s'", stmt->fun_stmt.name);
            return false;
        }
    }

    // resolve parameters
    size_t param_count = stmt->fun_stmt.params ? stmt->fun_stmt.params->count : 0;
    Type **param_types = NULL;

    if (param_count > 0)
    {
        param_types = malloc(sizeof(Type *) * param_count);

        for (size_t i = 0; i < param_count; i++)
        {
            AstNode *param = stmt->fun_stmt.params->items[i];
            if (param->kind != AST_STMT_PARAM)
            {
                free(param_types);
                return false;
            }

            // skip variadic sentinel
            if (param->param_stmt.is_variadic)
            {
                param_types[i] = NULL;
                continue;
            }

            Type *param_type = resolve_type_in_context(driver, ctx, param->param_stmt.type);
            if (!param_type)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, param, ctx->file_path, "cannot resolve type for parameter '%s'", param->param_stmt.name);
                free(param_types);
                return false;
            }

            param_types[i] = param_type;
            param->type    = param_type;
        }
    }

    // create function type
    Type *func_type = type_function_create(return_type, param_types, param_count, stmt->fun_stmt.is_variadic);
    free(param_types);

    // update symbol
    stmt->symbol->type = func_type;
    stmt->type         = func_type;

    return true;
}

// resolve external function signature
static bool resolve_ext_signature(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol || stmt->symbol->kind != SYMBOL_FUNC)
        return false;

    // resolve return type from ext_stmt.type (full function type)
    Type *func_type = resolve_type_in_context(driver, ctx, stmt->ext_stmt.type);
    if (!func_type || func_type->kind != TYPE_FUNCTION)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "invalid type for external function '%s'", stmt->ext_stmt.name);
        return false;
    }

    stmt->symbol->type = func_type;
    stmt->type         = func_type;

    return true;
}

// resolve variable type
static bool resolve_var_type(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol)
        return false;

    Type *var_type = NULL;

    // explicit type annotation
    if (stmt->var_stmt.type)
    {
        var_type = resolve_type_in_context(driver, ctx, stmt->var_stmt.type);
        if (!var_type)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "cannot resolve type for '%s'", stmt->var_stmt.name);
            return false;
        }
    }
    else if (stmt->var_stmt.init)
    {
        // type inference from initializer - defer to pass C
        // for now, require explicit types
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "type inference not yet implemented for '%s'", stmt->var_stmt.name);
        return false;
    }
    else
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "variable '%s' must have explicit type or initializer", stmt->var_stmt.name);
        return false;
    }

    stmt->symbol->type = var_type;
    stmt->type         = var_type;

    return true;
}

static bool analyze_pass_b_signatures(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root)
{
    if (!root || root->kind != AST_PROGRAM)
        return false;

    bool success = true;

    // resolve type aliases
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if (stmt->kind == AST_STMT_DEF && stmt->symbol)
        {
            Type *resolved = resolve_type_in_context(driver, ctx, stmt->def_stmt.type);
            if (resolved)
            {
                stmt->symbol->type->alias.target = resolved;
                stmt->type                       = resolved;
            }
            else
            {
                success = false;
            }
        }
    }

    // resolve struct fields
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if (stmt->kind == AST_STMT_STR && stmt->symbol && !stmt->symbol->type_def.is_generic)
        {
            if (!resolve_str_fields(driver, ctx, stmt))
                success = false;
        }
    }

    // resolve union fields
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if (stmt->kind == AST_STMT_UNI && stmt->symbol && !stmt->symbol->type_def.is_generic)
        {
            if (!resolve_uni_fields(driver, ctx, stmt))
                success = false;
        }
    }

    // resolve function signatures
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if (stmt->kind == AST_STMT_FUN && stmt->symbol && !stmt->symbol->func.is_generic)
        {
            // handle both regular functions and methods
            if (!resolve_fun_signature(driver, ctx, stmt))
                success = false;
        }

        // also handle external functions
        if (stmt->kind == AST_STMT_EXT && stmt->symbol)
        {
            if (!resolve_ext_signature(driver, ctx, stmt))
                success = false;
        }
    }

    // resolve variable types
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if ((stmt->kind == AST_STMT_VAL || stmt->kind == AST_STMT_VAR) && stmt->symbol)
        {
            if (!resolve_var_type(driver, ctx, stmt))
                success = false;
        }
    }

    return success;
}

// instantiate generic struct
static Symbol *instantiate_generic_struct(SemanticDriver *driver, const AnalysisContext *ctx, Symbol *generic_sym, Type **type_args, size_t arg_count)
{
    if (!generic_sym || !generic_sym->decl || arg_count == 0)
        return NULL;

    Symbol *origin_sym = generic_sym->import_origin ? generic_sym->import_origin : generic_sym;
    generic_sym        = origin_sym;

    // check cache first
    Symbol *cached = specialization_cache_find(&driver->spec_cache, generic_sym, type_args, arg_count);
    if (cached)
        return cached;

    // create specialized type name
    const char *module_name      = generic_sym->module_name ? generic_sym->module_name : ctx->module_name;
    char       *specialized_name = mangle_generic_type(module_name, generic_sym->name, type_args, arg_count);
    if (!specialized_name)
        return NULL;

    // create specialized struct type
    Type   *specialized_type             = type_struct_create(specialized_name);
    Symbol *specialized_sym              = symbol_create(SYMBOL_TYPE, specialized_name, specialized_type, generic_sym->decl);
    specialized_sym->type_def.is_alias   = false;
    specialized_sym->type_def.is_generic = false;
    specialized_sym->is_public           = generic_sym->is_public;
    specialized_sym->module_name         = module_name ? strdup(module_name) : NULL;
    specialized_sym->home_scope          = generic_sym->home_scope;

    // store type arguments in the specialized type
    specialized_type->generic_origin = generic_sym;
    specialized_type->type_arg_count = arg_count;
    if (arg_count > 0)
    {
        specialized_type->type_args = malloc(sizeof(Type *) * arg_count);
        memcpy(specialized_type->type_args, type_args, sizeof(Type *) * arg_count);
    }

    // build binding context from type parameters
    AstNode          *generic_decl = generic_sym->decl;
    GenericBindingCtx bindings     = ctx->bindings;

    if (generic_decl->kind == AST_STMT_STR && generic_decl->str_stmt.generics)
    {
        for (size_t i = 0; i < arg_count && i < (size_t)generic_decl->str_stmt.generics->count; i++)
        {
            AstNode    *param      = generic_decl->str_stmt.generics->items[i];
            const char *param_name = param->type_param.name;
            bindings               = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
        }
    }
    else if (generic_decl->kind == AST_STMT_STR && generic_sym->type_def.generic_param_names)
    {
        // fallback: use stored generic parameter names from symbol
        for (size_t i = 0; i < arg_count && i < generic_sym->type_def.generic_param_count; i++)
        {
            const char *param_name = generic_sym->type_def.generic_param_names[i];
            bindings               = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
        }
    }
    else if (generic_decl->kind == AST_STMT_STR)
    {
        // debug: neither source worked
        diagnostic_emit(&driver->diagnostics,
                        DIAG_ERROR,
                        NULL,
                        ctx->file_path,
                        "internal error: generic struct '%s' has no parameter names (generics=%p, param_names=%p, count=%zu)",
                        generic_sym->name,
                        (void *)generic_decl->str_stmt.generics,
                        (void *)generic_sym->type_def.generic_param_names,
                        generic_sym->type_def.generic_param_count);
    }

    // create context with bindings and update file_path to point to the generic template's file
    AnalysisContext specialized_ctx = *ctx;
    specialized_ctx.bindings        = bindings;
    if (generic_sym->home_scope)
    {
        specialized_ctx.current_scope = generic_sym->home_scope;
        specialized_ctx.module_scope  = generic_sym->home_scope;
    }
    specialized_ctx.module_name = module_name;
    if (generic_sym->module_name)
        specialized_ctx.module_name = generic_sym->module_name;

    // CRITICAL: update file_path to point to the generic template's source file
    // This ensures diagnostics during instantiation show the correct file
    FOR_EACH_MODULE(&driver->module_manager, mod)
    {
        // check if this module contains the generic symbol's declaration
        if (mod->symbols && mod->symbols->global_scope)
        {
            for (Symbol *sym = mod->symbols->global_scope->symbols; sym; sym = sym->next)
            {
                if (sym == generic_sym || (generic_sym->import_origin && sym == generic_sym->import_origin))
                {
                    if (mod->file_path)
                    {
                        specialized_ctx.file_path = mod->file_path;
                    }
                    goto found_module;
                }
            }
        }
    }
found_module:;

    // resolve fields with specialized context
    if (generic_decl->str_stmt.fields)
    {
        Symbol *field_head  = NULL;
        Symbol *field_tail  = NULL;
        size_t  offset      = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        for (int i = 0; i < generic_decl->str_stmt.fields->count; i++)
        {
            AstNode *field_node = generic_decl->str_stmt.fields->items[i];
            Type    *field_type = resolve_type_in_context(driver, &specialized_ctx, field_node->field_stmt.type);

            if (!field_type)
            {
                free(specialized_name);
                return NULL;
            }

            Symbol *field_sym = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);

            size_t field_align = type_alignof(field_type);
            if (field_align > 0)
            {
                size_t remainder = offset % field_align;
                if (remainder != 0)
                    offset += field_align - remainder;
            }
            field_sym->field.offset = offset;

            if (!field_head)
                field_head = field_tail = field_sym;
            else
            {
                field_tail->next = field_sym;
                field_tail       = field_sym;
            }

            offset += type_sizeof(field_type);
            if (field_align > max_align)
                max_align = field_align;
            field_count++;
        }

        if (max_align > 0)
        {
            size_t remainder = offset % max_align;
            if (remainder != 0)
                offset += max_align - remainder;
        }

        specialized_type->composite.fields      = field_head;
        specialized_type->composite.field_count = field_count;
        specialized_type->size                  = offset;
        specialized_type->alignment             = max_align;
    }

    // cache the specialization
    specialization_cache_insert(&driver->spec_cache, generic_sym, type_args, arg_count, specialized_sym);

    free(specialized_name);
    return specialized_sym;
}

// instantiate generic union
static Symbol *instantiate_generic_union(SemanticDriver *driver, const AnalysisContext *ctx, Symbol *generic_sym, Type **type_args, size_t arg_count)
{
    if (!generic_sym || !generic_sym->decl || arg_count == 0)
        return NULL;

    Symbol *origin_sym = generic_sym->import_origin ? generic_sym->import_origin : generic_sym;
    generic_sym        = origin_sym;

    // check cache first
    Symbol *cached = specialization_cache_find(&driver->spec_cache, generic_sym, type_args, arg_count);
    if (cached)
        return cached;

    // create specialized type name
    const char *module_name      = generic_sym->module_name ? generic_sym->module_name : ctx->module_name;
    char       *specialized_name = mangle_generic_type(module_name, generic_sym->name, type_args, arg_count);
    if (!specialized_name)
        return NULL;

    // create specialized union type
    Type   *specialized_type             = type_union_create(specialized_name);
    Symbol *specialized_sym              = symbol_create(SYMBOL_TYPE, specialized_name, specialized_type, generic_sym->decl);
    specialized_sym->type_def.is_alias   = false;
    specialized_sym->type_def.is_generic = false;
    specialized_sym->is_public           = generic_sym->is_public;
    specialized_sym->module_name         = module_name ? strdup(module_name) : NULL;
    specialized_sym->home_scope          = generic_sym->home_scope;

    // store type arguments in the specialized type
    specialized_type->generic_origin = generic_sym;
    specialized_type->type_arg_count = arg_count;
    if (arg_count > 0)
    {
        specialized_type->type_args = malloc(sizeof(Type *) * arg_count);
        memcpy(specialized_type->type_args, type_args, sizeof(Type *) * arg_count);
    }

    // build binding context
    AstNode          *generic_decl = generic_sym->decl;
    GenericBindingCtx bindings     = ctx->bindings;

    if (generic_decl->kind == AST_STMT_UNI && generic_decl->uni_stmt.generics)
    {
        for (size_t i = 0; i < arg_count && i < (size_t)generic_decl->uni_stmt.generics->count; i++)
        {
            AstNode    *param      = generic_decl->uni_stmt.generics->items[i];
            const char *param_name = param->type_param.name;
            bindings               = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
        }
    }
    else if (generic_decl->kind == AST_STMT_UNI && generic_sym->type_def.generic_param_names)
    {
        // fallback: use stored generic parameter names from symbol
        for (size_t i = 0; i < arg_count && i < generic_sym->type_def.generic_param_count; i++)
        {
            const char *param_name = generic_sym->type_def.generic_param_names[i];
            bindings               = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
        }
    }

    AnalysisContext specialized_ctx = *ctx;
    specialized_ctx.bindings        = bindings;
    if (generic_sym->home_scope)
    {
        specialized_ctx.current_scope = generic_sym->home_scope;
        specialized_ctx.module_scope  = generic_sym->home_scope;
    }

    // update file_path to point to the generic template's source file
    FOR_EACH_MODULE(&driver->module_manager, mod)
    {
        if (mod->symbols && mod->symbols->global_scope)
        {
            for (Symbol *sym = mod->symbols->global_scope->symbols; sym; sym = sym->next)
            {
                if (sym == generic_sym || (generic_sym->import_origin && sym == generic_sym->import_origin))
                {
                    if (mod->file_path)
                    {
                        specialized_ctx.file_path = mod->file_path;
                    }
                    goto found_union_module;
                }
            }
        }
    }
found_union_module:;

    // resolve variants with specialized context
    if (generic_decl->uni_stmt.fields)
    {
        Symbol *field_head  = NULL;
        Symbol *field_tail  = NULL;
        size_t  max_size    = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        for (int i = 0; i < generic_decl->uni_stmt.fields->count; i++)
        {
            AstNode *field_node = generic_decl->uni_stmt.fields->items[i];
            Type    *field_type = resolve_type_in_context(driver, &specialized_ctx, field_node->field_stmt.type);

            if (!field_type)
            {
                free(specialized_name);
                return NULL;
            }

            // resolve aliases to get actual type with size
            Type *resolved_field_type = type_resolve_alias(field_type);
            if (resolved_field_type)
                field_type = resolved_field_type;

            Symbol *field_sym       = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);
            field_sym->field.offset = 0;

            if (!field_head)
                field_head = field_tail = field_sym;
            else
            {
                field_tail->next = field_sym;
                field_tail       = field_sym;
            }

            size_t field_size  = type_sizeof(field_type);
            size_t field_align = type_alignof(field_type);

            if (field_size > max_size)
                max_size = field_size;
            if (field_align > max_align)
                max_align = field_align;
            field_count++;
        }

        if (max_align > 0)
        {
            size_t remainder = max_size % max_align;
            if (remainder != 0)
                max_size += max_align - remainder;
        }

        specialized_type->composite.fields      = field_head;
        specialized_type->composite.field_count = field_count;
        specialized_type->size                  = max_size;
        specialized_type->alignment             = max_align;
    }

    // cache the specialization
    specialization_cache_insert(&driver->spec_cache, generic_sym, type_args, arg_count, specialized_sym);

    free(specialized_name);
    return specialized_sym;
}

// instantiate generic function
static Symbol *instantiate_generic_function(SemanticDriver *driver, const AnalysisContext *ctx, Symbol *generic_sym, Type **type_args, size_t arg_count)
{
    if (!generic_sym || !generic_sym->decl || arg_count == 0)
        return NULL;

    Symbol *origin_sym = generic_sym->import_origin ? generic_sym->import_origin : generic_sym;
    generic_sym        = origin_sym;

    // check cache first
    Symbol *cached = specialization_cache_find(&driver->spec_cache, generic_sym, type_args, arg_count);
    if (cached)
        return cached;

    // create specialized function name
    const char *module_name      = generic_sym->module_name ? generic_sym->module_name : ctx->module_name;
    char       *specialized_name = mangle_generic_function(module_name, generic_sym->name, type_args, arg_count);
    if (!specialized_name)
        return NULL;

    // build binding context
    AstNode          *generic_decl = generic_sym->decl;
    GenericBindingCtx bindings     = ctx->bindings;

    // use stored generic_param_names which includes owner type params for methods
    if (generic_sym->func.generic_param_names && generic_sym->func.generic_param_count > 0)
    {
        for (size_t i = 0; i < arg_count && i < generic_sym->func.generic_param_count; i++)
        {
            const char *param_name = generic_sym->func.generic_param_names[i];
            if (param_name)
            {
                bindings = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
            }
        }
    }
    else if (generic_decl->kind == AST_STMT_FUN && generic_decl->fun_stmt.generics)
    {
        // fallback for standalone generic functions
        for (size_t i = 0; i < arg_count && i < (size_t)generic_decl->fun_stmt.generics->count; i++)
        {
            AstNode    *param      = generic_decl->fun_stmt.generics->items[i];
            const char *param_name = param->type_param.name;
            bindings               = generic_binding_ctx_push(&bindings, param_name, type_args[i]);
        }
    }

    AnalysisContext specialized_ctx = *ctx;
    specialized_ctx.bindings        = bindings;
    if (generic_sym->home_scope)
    {
        specialized_ctx.current_scope = generic_sym->home_scope;
        specialized_ctx.module_scope  = generic_sym->home_scope;
    }
    specialized_ctx.module_name = module_name;

    // update file_path to point to the generic template's source file
    FOR_EACH_MODULE(&driver->module_manager, mod)
    {
        if (mod->symbols && mod->symbols->global_scope)
        {
            for (Symbol *sym = mod->symbols->global_scope->symbols; sym; sym = sym->next)
            {
                if (sym == generic_sym || (generic_sym->import_origin && sym == generic_sym->import_origin))
                {
                    if (mod->file_path)
                    {
                        specialized_ctx.file_path = mod->file_path;
                    }
                    goto found_func_module;
                }
            }
        }
    }
found_func_module:;

    // resolve return type
    Type *ret_type = NULL;
    if (generic_decl->fun_stmt.return_type)
    {
        ret_type = resolve_type_in_context(driver, &specialized_ctx, generic_decl->fun_stmt.return_type);
        if (!ret_type)
        {
            free(specialized_name);
            return NULL;
        }
    }

    // resolve parameters
    size_t param_count = generic_decl->fun_stmt.params ? generic_decl->fun_stmt.params->count : 0;
    Type **param_types = NULL;

    if (param_count > 0)
    {
        param_types = malloc(sizeof(Type *) * param_count);
        for (size_t i = 0; i < param_count; i++)
        {
            AstNode *param = generic_decl->fun_stmt.params->items[i];
            if (param->param_stmt.is_variadic)
            {
                param_types[i] = NULL;
                continue;
            }

            param_types[i] = resolve_type_in_context(driver, &specialized_ctx, param->param_stmt.type);
            if (!param_types[i])
            {
                free(param_types);
                free(specialized_name);
                return NULL;
            }
        }
    }

    // create specialized function type
    Type *func_type = type_function_create(ret_type, param_types, param_count, generic_decl->fun_stmt.is_variadic);
    free(param_types);

    // create specialized symbol
    Symbol *specialized_sym                       = symbol_create(SYMBOL_FUNC, specialized_name, func_type, generic_decl);
    specialized_sym->func.is_external             = generic_sym->func.is_external;
    specialized_sym->func.is_defined              = generic_sym->func.is_defined;
    specialized_sym->func.is_generic              = false;
    specialized_sym->func.is_specialized_instance = true;
    specialized_sym->func.uses_mach_varargs       = generic_sym->func.uses_mach_varargs;
    specialized_sym->func.is_method               = generic_sym->func.is_method;
    specialized_sym->func.method_owner            = generic_sym->func.method_owner;
    specialized_sym->is_public                    = generic_sym->is_public;
    specialized_sym->module_name                  = module_name ? strdup(module_name) : NULL;
    specialized_sym->home_scope                   = generic_sym->home_scope;
    specialized_sym->func.mangled_name            = strdup(specialized_name);

    // add specialized symbol to the generic's home scope (where it's defined)
    if (generic_sym->home_scope)
    {
        symbol_add(generic_sym->home_scope, specialized_sym);
    }

    // cache the specialization
    specialization_cache_insert(&driver->spec_cache, generic_sym, type_args, arg_count, specialized_sym);

    // if function has body, clone AST and analyze it with specialized context
    if (generic_decl->fun_stmt.body)
    {
        // clone the generic template AST so this specialization has independent scope bindings
        AstNode *specialized_decl = ast_clone(generic_decl);
        if (!specialized_decl)
        {
            free(specialized_name);
            return NULL;
        }

        // set the specialized symbol and type on the cloned AST
        specialized_decl->symbol = specialized_sym;
        specialized_decl->type   = func_type;        // set the specialized function type
        specialized_sym->decl    = specialized_decl; // update symbol to point to cloned AST

        if (!analyze_function_body(driver, &specialized_ctx, specialized_decl))
        {
            ast_node_dnit(specialized_decl);
            free(specialized_decl);
            free(specialized_name);
            return NULL; // body analysis failed
        }
    }

    free(specialized_name);
    return specialized_sym;
}

// process instantiation queue
static bool process_instantiation_queue(SemanticDriver *driver, const AnalysisContext *ctx)
{
    bool         success        = true;
    size_t       iterations     = 0;
    const size_t max_iterations = 1000; // prevent infinite loops

    while (driver->inst_queue.count > 0 && iterations < max_iterations)
    {
        InstantiationRequest *req = instantiation_queue_pop(&driver->inst_queue);
        if (!req)
            break;

        Symbol *specialized = NULL;

        switch (req->kind)
        {
        case INST_STRUCT:
            specialized = instantiate_generic_struct(driver, ctx, req->generic_symbol, req->type_args, req->type_arg_count);
            break;

        case INST_UNION:
            specialized = instantiate_generic_union(driver, ctx, req->generic_symbol, req->type_args, req->type_arg_count);
            break;

        case INST_FUNCTION:
            specialized = instantiate_generic_function(driver, ctx, req->generic_symbol, req->type_args, req->type_arg_count);
            break;
        }

        if (!specialized)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, req->call_site, ctx->file_path, "failed to instantiate generic");
            success = false;
        }

        // clean up request
        free(req->type_args);
        free(req);

        iterations++;
    }

    if (iterations >= max_iterations)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, ctx->file_path, "generic instantiation exceeded maximum depth (possible infinite recursion)");
        success = false;
    }

    return success;
}

// helper to request instantiation for type node with generic args
static Symbol *request_generic_type_instantiation(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *type_node)
{
    if (!type_node || type_node->kind != AST_TYPE_NAME)
        return NULL;

    if (!type_node->type_name.generic_args || type_node->type_name.generic_args->count == 0)
        return NULL;

    // look up base generic symbol
    Symbol *generic_sym = symbol_lookup_scope(ctx->current_scope, type_node->type_name.name);
    if (!generic_sym)
        generic_sym = symbol_lookup_scope(ctx->module_scope, type_node->type_name.name);
    if (!generic_sym)
        generic_sym = symbol_lookup_scope(ctx->global_scope, type_node->type_name.name);

    if (!generic_sym || generic_sym->kind != SYMBOL_TYPE)
        return NULL;

    if (generic_sym->import_origin)
        generic_sym = generic_sym->import_origin;

    Symbol *origin_sym = generic_sym->import_origin ? generic_sym->import_origin : generic_sym;
    generic_sym        = origin_sym;

    // resolve type arguments
    size_t arg_count = type_node->type_name.generic_args->count;
    Type **type_args = malloc(sizeof(Type *) * arg_count);

    for (size_t i = 0; i < arg_count; i++)
    {
        type_args[i] = resolve_type_in_context(driver, ctx, type_node->type_name.generic_args->items[i]);
        if (!type_args[i])
        {
            free(type_args);
            return NULL;
        }
    }

    // check if already specialized
    Symbol *specialized = specialization_cache_find(&driver->spec_cache, generic_sym, type_args, arg_count);
    if (specialized)
    {
        free(type_args);
        return specialized;
    }

    // determine kind and instantiate immediately (synchronous for types)
    InstantiationKind kind = INST_STRUCT;
    if (generic_sym->type && generic_sym->type->kind == TYPE_UNION)
        kind = INST_UNION;

    if (kind == INST_STRUCT)
        specialized = instantiate_generic_struct(driver, ctx, generic_sym, type_args, arg_count);
    else
        specialized = instantiate_generic_union(driver, ctx, generic_sym, type_args, arg_count);

    free(type_args);
    return specialized;
}

// forward declarations for mutual recursion
static Type *analyze_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr);
static bool  analyze_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt);

// expression analysis
static bool integer_fits_in_type(unsigned long long value, Type *type)
{
    if (!type)
        return false;

    switch (type->kind)
    {
    case TYPE_U8:
        return value <= UINT8_MAX;
    case TYPE_U16:
        return value <= UINT16_MAX;
    case TYPE_U32:
        return value <= UINT32_MAX;
    case TYPE_U64:
        return true;
    case TYPE_I8:
        return value <= (unsigned long long)INT8_MAX;
    case TYPE_I16:
        return value <= (unsigned long long)INT16_MAX;
    case TYPE_I32:
        return value <= (unsigned long long)INT32_MAX;
    case TYPE_I64:
        return value <= (unsigned long long)INT64_MAX;
    default:
        return false;
    }
}

static Type *analyze_lit_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected)
{
    (void)driver;
    (void)ctx;

    Type *resolved_expected = expected ? type_resolve_alias(expected) : NULL;

    switch (expr->lit_expr.kind)
    {
    case TOKEN_LIT_INT:
    {
        unsigned long long value = expr->lit_expr.int_val;

        if (resolved_expected)
        {
            if (type_is_pointer_like(resolved_expected) && value == 0)
            {
                expr->type = expected;
                return expr->type;
            }

            if (type_is_integer(resolved_expected) && integer_fits_in_type(value, resolved_expected))
            {
                expr->type = expected;
                return expr->type;
            }
        }

        // fall back to default integer size (i32)
        expr->type = type_i32();
        return expr->type;
    }
    case TOKEN_LIT_FLOAT:
        if (resolved_expected && type_is_float(resolved_expected))
        {
            expr->type = expected;
            return expr->type;
        }
        expr->type = type_f64();
        return expr->type;
    case TOKEN_LIT_CHAR:
        if (resolved_expected && type_is_integer(resolved_expected) && resolved_expected->size >= type_u8()->size)
        {
            expr->type = expected;
            return expr->type;
        }
        expr->type = type_u8();
        return expr->type;
    case TOKEN_LIT_STRING:
        expr->type = type_array_create(type_u8());
        return expr->type;
    default:
        return NULL;
    }
}

// helper: lookup symbol traversing scope chain
static Symbol *lookup_in_scope_chain(Scope *current_scope, Scope *module_scope, Scope *global_scope, const char *name)
{
    // traverse current scope chain (handles nested blocks)
    for (Scope *scope = current_scope; scope; scope = scope->parent)
    {
        Symbol *sym = symbol_lookup_scope(scope, name);
        if (sym)
            return sym;
    }

    // try module scope if not already checked
    if (module_scope && module_scope != current_scope)
    {
        Symbol *sym = symbol_lookup_scope(module_scope, name);
        if (sym)
            return sym;
    }

    // try global scope if not already checked
    if (global_scope && global_scope != current_scope && global_scope != module_scope)
    {
        Symbol *sym = symbol_lookup_scope(global_scope, name);
        if (sym)
            return sym;
    }

    return NULL;
}

static Type *analyze_null_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected)
{

    Type *resolved = expected ? type_resolve_alias(expected) : NULL;
    if (resolved)
    {
        if (!type_is_pointer_like(resolved))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "'nil' expects a pointer-like context");
            return NULL;
        }
        expr->type = expected;
        return expr->type;
    }

    expr->type = type_ptr();
    return expr->type;
}

static Type *analyze_ident_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    const char *name = expr->ident_expr.name;

    // if symbol is already set (e.g., from method call transformation), use it directly
    Symbol *sym = expr->symbol;
    if (!sym)
    {
        sym = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, name);

        if (!sym)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "undefined identifier '%s'", name);
            return NULL;
        }
    }

    if (sym->import_origin && sym->import_origin->type)
    {
        sym->type = sym->import_origin->type;
        if (!sym->has_const_i64 && sym->import_origin->has_const_i64)
        {
            sym->has_const_i64 = true;
            sym->const_i64     = sym->import_origin->const_i64;
        }

        if (sym->kind == SYMBOL_FUNC)
        {
            sym->func.is_external                    = sym->import_origin->func.is_external;
            sym->func.is_defined                     = sym->import_origin->func.is_defined;
            sym->func.is_generic                     = sym->import_origin->func.is_generic;
            sym->func.is_method                      = sym->import_origin->func.is_method;
            sym->func.method_owner                   = sym->import_origin->func.method_owner;
            sym->func.method_forwarded_generic_count = sym->import_origin->func.method_forwarded_generic_count;
            sym->func.method_receiver_is_pointer     = sym->import_origin->func.method_receiver_is_pointer;
        }
    }

    expr->symbol = sym;
    expr->type   = sym->type;
    return sym->type;
}

static bool is_lvalue_expr(const AstNode *expr)
{
    if (!expr)
        return false;

    switch (expr->kind)
    {
    case AST_EXPR_IDENT:
        if (expr->symbol)
        {
            switch (expr->symbol->kind)
            {
            case SYMBOL_VAR:
            case SYMBOL_VAL:
            case SYMBOL_PARAM:
            case SYMBOL_FIELD:
                return true;
            default:
                break;
            }
        }
        return true;

    case AST_EXPR_INDEX:
        return true;

    case AST_EXPR_FIELD:
        return is_lvalue_expr(expr->field_expr.object);

    case AST_EXPR_UNARY:
        return expr->unary_expr.op == TOKEN_AT;

    default:
        return false;
    }
}

static Type *analyze_binary_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    Type *left  = analyze_expr(driver, ctx, expr->binary_expr.left);
    Type *right = analyze_expr(driver, ctx, expr->binary_expr.right);

    if (!left || !right)
        return NULL;

    left  = type_resolve_alias(left);
    right = type_resolve_alias(right);

    TokenKind op        = expr->binary_expr.op;
    bool      left_ptr  = type_is_pointer_like(left);
    bool      right_ptr = type_is_pointer_like(right);

    if ((op == TOKEN_PLUS || op == TOKEN_MINUS) && (left_ptr || right_ptr))
    {
        if (left_ptr && right_ptr)
        {
            if (op == TOKEN_MINUS)
            {
                expr->type = type_u64();
                return expr->type;
            }

            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot add two pointers");
            return NULL;
        }

        Type *pointer_type = left_ptr ? left : right;
        Type *other_type   = left_ptr ? right : left;

        if (!type_is_integer(other_type))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "pointer arithmetic requires integer offset");
            return NULL;
        }

        expr->type = pointer_type;
        return expr->type;
    }

    // arithmetic operators
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_STAR || op == TOKEN_SLASH || op == TOKEN_PERCENT)
    {
        if (!type_is_numeric(left) || !type_is_numeric(right))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "arithmetic requires numeric operands");
            return NULL;
        }

        // type promotion: choose wider type
        Type *result_type = left;

        // if sizes differ, use the larger
        if (left->size < right->size)
            result_type = right;
        else if (left->size == right->size)
        {
            // same size: prefer unsigned over signed
            if (type_is_integer(left) && type_is_integer(right))
            {
                // if one is unsigned and one is signed, use unsigned
                bool left_signed  = (strcmp(left->name, "i8") == 0 || strcmp(left->name, "i16") == 0 || strcmp(left->name, "i32") == 0 || strcmp(left->name, "i64") == 0);
                bool right_signed = (strcmp(right->name, "i8") == 0 || strcmp(right->name, "i16") == 0 || strcmp(right->name, "i32") == 0 || strcmp(right->name, "i64") == 0);

                if (!left_signed && right_signed)
                    result_type = left; // left is unsigned
                else if (left_signed && !right_signed)
                    result_type = right; // right is unsigned
            }
        }

        expr->type = result_type;
        return expr->type;
    }

    // bitwise operators
    if (op == TOKEN_PIPE || op == TOKEN_AMPERSAND || op == TOKEN_CARET)
    {
        if (!type_is_integer(left) || !type_is_integer(right))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "bitwise operators require integer operands");
            return NULL;
        }

        // result type follows arithmetic promotion rules
        Type *result_type = left;
        if (left->size < right->size)
            result_type = right;

        expr->type = result_type;
        return expr->type;
    }

    // bitshift operators
    if (op == TOKEN_LESS_LESS || op == TOKEN_GREATER_GREATER)
    {
        if (!type_is_integer(left) || !type_is_integer(right))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "bitshift operators require integer operands");
            return NULL;
        }

        expr->type = left;
        return expr->type;
    }

    // comparison operators
    if (op == TOKEN_EQUAL_EQUAL || op == TOKEN_BANG_EQUAL || op == TOKEN_LESS || op == TOKEN_LESS_EQUAL || op == TOKEN_GREATER || op == TOKEN_GREATER_EQUAL)
    {
        expr->type = type_u8(); // boolean result as u8
        return expr->type;
    }

    // logical operators
    if (op == TOKEN_AMPERSAND_AMPERSAND || op == TOKEN_PIPE_PIPE)
    {
        expr->type = type_u8(); // boolean result as u8
        return expr->type;
    }

    // assignment
    if (op == TOKEN_EQUAL)
    {
        if (!is_lvalue_expr(expr->binary_expr.left))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot assign to non-lvalue");
            return NULL;
        }

        if (!type_equals(left, right) && !type_can_assign_to(right, left))
        {
            char *lhs = type_to_string(left);
            char *rhs = type_to_string(right);
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "incompatible types in assignment (%s <- %s)", lhs ? lhs : "<unknown>", rhs ? rhs : "<unknown>");
            free(lhs);
            free(rhs);
            return NULL;
        }
        expr->type = left;
        return expr->type;
    }

    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "unsupported binary operator");
    return NULL;
}

static Type *analyze_unary_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    Type *operand = analyze_expr(driver, ctx, expr->unary_expr.expr);
    if (!operand)
        return NULL;
    operand = type_resolve_alias(operand);

    TokenKind op = expr->unary_expr.op;

    if (op == TOKEN_MINUS || op == TOKEN_PLUS)
    {
        if (!type_is_numeric(operand))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "unary arithmetic requires numeric operand");
            return NULL;
        }
        expr->type = operand;
        return expr->type;
    }

    if (op == TOKEN_BANG)
    {
        expr->type = type_u8(); // logical not result as u8
        return expr->type;
    }

    if (op == TOKEN_AT) // dereference (@expr)
    {
        if (!type_is_pointer_like(operand))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot dereference non-pointer type");
            return NULL;
        }
        if (operand->kind == TYPE_POINTER)
            expr->type = operand->pointer.base;
        else if (operand->kind == TYPE_ARRAY)
            expr->type = operand->array.elem_type;
        else if (operand->kind == TYPE_FUNCTION)
            expr->type = operand;
        else
            expr->type = operand;
        return expr->type;
    }

    if (op == TOKEN_QUESTION) // address-of (?expr)
    {
        if (!is_lvalue_expr(expr->unary_expr.expr))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "address-of requires lvalue");
            return NULL;
        }
        expr->type = type_pointer_create(operand);
        return expr->type;
    }

    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "unsupported unary operator");
    return NULL;
}

static Type *analyze_call_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    // Handle potential method calls FIRST: obj.method(args) -> method(obj, args)
    // This must happen before analyzing the func to avoid rejecting valid method calls
    if (expr->call_expr.func && expr->call_expr.func->kind == AST_EXPR_FIELD)
    {
        AstNode *field_expr = expr->call_expr.func;
        AstNode *receiver   = field_expr->field_expr.object;

        // Special case: check if this is module.function() where object is an identifier for a module
        if (receiver->kind == AST_EXPR_IDENT)
        {
            const char *obj_name = receiver->ident_expr.name;
            Symbol     *obj_sym  = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, obj_name);

            if (obj_sym && obj_sym->kind == SYMBOL_MODULE)
            {
                // This is module.function() - fall through to normal call analysis
                // The field expression will look up the function in the module scope
                goto normal_call_analysis;
            }
        }

        // Potential method call: analyze receiver and try method lookup
        // analyze receiver to get its type
        Type *receiver_type = analyze_expr(driver, ctx, receiver);
        if (!receiver_type)
            return NULL;

        // look up method in the receiver's type
        const char *method_name = field_expr->field_expr.field;

        Symbol *method_sym        = NULL;
        char    mangled_name[512] = {0};

        // dereference pointer types for method lookup (methods can be called on pointer receivers)
        Type *lookup_type = receiver_type;
        if (lookup_type->kind == TYPE_POINTER)
            lookup_type = lookup_type->pointer.base;

        // try to find method on alias type before resolving (for types like string = []char)
        if (lookup_type->kind == TYPE_ALIAS && lookup_type->name)
        {
                snprintf(mangled_name, sizeof(mangled_name), "%s__%s", lookup_type->name, method_name);
                method_sym = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, mangled_name);
            }

            // Store the original type before resolving, in case we need it later
            Type *original_type = lookup_type;

            // resolve aliases for further processing
            lookup_type = type_resolve_alias(lookup_type);

            // try looking up methods on the resolved type
            if (!method_sym && (lookup_type->kind == TYPE_STRUCT || lookup_type->kind == TYPE_UNION || lookup_type->kind == TYPE_ARRAY))
            {
                // For array types that came from an alias, use the alias name
                const char *type_name_for_lookup = NULL;
                if (lookup_type->kind == TYPE_ARRAY && original_type->kind == TYPE_ALIAS && original_type->name)
                {
                    type_name_for_lookup = original_type->name;
                }
                else if (lookup_type->name)
                {
                    type_name_for_lookup = lookup_type->name;
                }
                
                if (type_name_for_lookup)
                {
                    // first try: look up fully specialized method
                    // "std_types_result__Result$string$string" -> "std_types_result__Result$string$string__is_err"
                    snprintf(mangled_name, sizeof(mangled_name), "%s__%s", type_name_for_lookup, method_name);
                    method_sym = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, mangled_name);

                    // if not found and type is specialized, instantiate generic method
                    if (!method_sym && lookup_type->generic_origin && lookup_type->type_arg_count > 0)
                    {
                        // use stored type arguments from the specialized type
                        Symbol *generic_type_sym = lookup_type->generic_origin;

                        // construct generic method name from the generic type
                        char generic_method_name[256];
                        snprintf(generic_method_name, sizeof(generic_method_name), "%s__%s", generic_type_sym->name, method_name);

                        Symbol *generic_method = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, generic_method_name);

                        if (!generic_method)
                        {
                            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "generic method '%s' not found in any scope", generic_method_name);
                            return NULL;
                        }

                        if (generic_method->kind != SYMBOL_FUNC)
                        {
                            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "symbol '%s' is not a function (kind=%d)", generic_method_name, generic_method->kind);
                            return NULL;
                        }

                        if (!generic_method->func.is_generic)
                        {
                            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "function '%s' is not generic", generic_method_name);
                            return NULL;
                        }

                        if (!generic_method->func.is_method)
                        {
                            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "function '%s' is not a method", generic_method_name);
                            return NULL;
                        }

                        // instantiate generic method with type arguments from the specialized type
                        method_sym = instantiate_generic_function(driver, ctx, generic_method, lookup_type->type_args, lookup_type->type_arg_count);
                        if (!method_sym)
                        {
                            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "failed to instantiate generic method '%s' with %zu type arguments", generic_method_name, lookup_type->type_arg_count);
                            return NULL;
                        }
                    }
                }
            }

            if (method_sym && method_sym->kind == SYMBOL_FUNC && method_sym->func.is_method)
            {
                // check if method expects pointer receiver and coerce if needed
                AstNode *receiver_arg = receiver;
                if (method_sym->type && method_sym->type->kind == TYPE_FUNCTION && method_sym->type->function.param_count > 0)
                {
                    Type *first_param_type = method_sym->type->function.param_types[0];
                    if (first_param_type && first_param_type->kind == TYPE_POINTER)
                    {
                        // method expects pointer receiver, but we have value type
                        if (receiver_type->kind != TYPE_POINTER)
                        {
                            // create address-of operation
                            AstNode *addr_of = malloc(sizeof(AstNode));
                            ast_node_init(addr_of, AST_EXPR_UNARY);
                            addr_of->token           = NULL; // don't share token to avoid double-free
                            addr_of->unary_expr.op   = TOKEN_QUESTION;
                            addr_of->unary_expr.expr = receiver;
                            addr_of->type            = type_pointer_create(receiver_type);
                            receiver_arg             = addr_of;
                        }
                    }
                }

                // transform method call: prepend receiver as first argument
                if (!expr->call_expr.args)
                {
                    expr->call_expr.args = malloc(sizeof(AstList));
                    ast_list_init(expr->call_expr.args);
                }

                // insert receiver at the beginning
                ast_list_prepend(expr->call_expr.args, receiver_arg);

                // replace func with identifier pointing to method symbol
                AstNode *ident = malloc(sizeof(AstNode));
                ast_node_init(ident, AST_EXPR_IDENT);
                ident->token           = field_expr->token;
                ident->ident_expr.name = strdup(mangled_name);
                ident->symbol          = method_sym;
                ident->type            = method_sym->type;

                // clear receiver from field expr to prevent double-free
                // receiver is now owned by either:
                // - the addr_of node (if wrapped), which is in args
                // - directly in args (if not wrapped)
                field_expr->field_expr.object = NULL;
                field_expr->token             = NULL;

                // replace func
                expr->call_expr.func           = ident;
                expr->call_expr.is_method_call = true;

                // free the old field expression
                ast_node_dnit(field_expr);
                free(field_expr);

                // continue with normal call analysis below
            }
            else
            {
                // not a method - might be a function stored in a field, which isn't supported yet
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, 
                                "no method '%s' found on type (kind=%d, name=%s)", 
                                method_name,
                                receiver_type ? receiver_type->kind : -1,
                                receiver_type && receiver_type->name ? receiver_type->name : "(null)");
                return NULL;
            }
    }

normal_call_analysis:
    ;  // label must be followed by a statement
    
    if (expr->call_expr.func && expr->call_expr.func->kind == AST_EXPR_IDENT)
    {
        const char *intr_name = expr->call_expr.func->ident_expr.name;
        size_t      arg_count = expr->call_expr.args ? expr->call_expr.args->count : 0;

        if (strcmp(intr_name, "size_of") == 0)
        {
            if (arg_count != 1)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "size_of expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = NULL;

            // Check if argument is a type node (e.g., size_of(*u64))
            if (arg->kind >= AST_TYPE_NAME && arg->kind <= AST_TYPE_UNI)
            {
                arg_type = resolve_type_in_context(driver, ctx, arg);
                // Store the resolved type on the arg node for codegen
                arg->type = arg_type;
            }
            // Check if it's an identifier - try to resolve as a type name first
            else if (arg->kind == AST_EXPR_IDENT)
            {
                // Create a temporary type node and try resolving it
                AstNode type_node                = {0};
                type_node.kind                   = AST_TYPE_NAME;
                type_node.type_name.name         = arg->ident_expr.name;
                type_node.type_name.generic_args = NULL;

                // Try to resolve it as a type (handles builtins and user types)
                arg_type = resolve_type_in_context(driver, ctx, &type_node);

                // If it resolved to error type, it's not a valid type identifier
                // So reject it rather than falling back to expression analysis
                if (!arg_type || arg_type == type_error())
                {
                    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "size_of expects a type, but '%s' is not a valid type", arg->ident_expr.name);
                    return NULL;
                }

                // Store the resolved type on the arg node for codegen
                arg->type = arg_type;
            }
            else
            {
                // Reject expressions - size_of should only accept types
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "size_of expects a type argument, not an expression");
                return NULL;
            }

            if (!arg_type)
                return NULL;

            expr->type = type_u64();
            return expr->type;
        }

        if (strcmp(intr_name, "align_of") == 0)
        {
            if (arg_count != 1)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "align_of expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = NULL;

            // Check if argument is a type node (e.g., align_of(*u64))
            if (arg->kind >= AST_TYPE_NAME && arg->kind <= AST_TYPE_UNI)
            {
                arg_type = resolve_type_in_context(driver, ctx, arg);
                // Store the resolved type on the arg node for codegen
                arg->type = arg_type;
            }
            // Check if it's an identifier - try to resolve as a type name first
            else if (arg->kind == AST_EXPR_IDENT)
            {
                // Create a temporary type node and try resolving it
                AstNode type_node                = {0};
                type_node.kind                   = AST_TYPE_NAME;
                type_node.type_name.name         = arg->ident_expr.name;
                type_node.type_name.generic_args = NULL;

                // Try to resolve it as a type (handles builtins and user types)
                arg_type = resolve_type_in_context(driver, ctx, &type_node);

                // If it resolved to error type, it's not a valid type identifier
                if (!arg_type || arg_type == type_error())
                {
                    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "align_of expects a type, but '%s' is not a valid type", arg->ident_expr.name);
                    return NULL;
                }

                // Store the resolved type on the arg node for codegen
                arg->type = arg_type;
            }
            else
            {
                // Reject expressions - align_of should only accept types
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "align_of expects a type argument, not an expression");
                return NULL;
            }

            if (!arg_type)
                return NULL;

            expr->type = type_u64();
            return expr->type;
        }

        if (strcmp(intr_name, "va_count") == 0)
        {
            if (!ctx->current_function || !ctx->current_function->type || !ctx->current_function->type->function.is_variadic)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "va_count used outside variadic function");
                return NULL;
            }

            if (arg_count != 0)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "va_count expects no arguments");
                return NULL;
            }

            expr->type = type_u64();
            return expr->type;
        }

        if (strcmp(intr_name, "va_arg") == 0)
        {
            if (!ctx->current_function || !ctx->current_function->type || !ctx->current_function->type->function.is_variadic)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "va_arg used outside variadic function");
                return NULL;
            }

            if (arg_count != 1)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "va_arg expects exactly one argument");
                return NULL;
            }

            AstNode *index_expr = expr->call_expr.args->items[0];
            Type    *index_type = analyze_expr(driver, ctx, index_expr);
            if (!index_type)
                return NULL;

            index_type = type_resolve_alias(index_type);
            if (!type_is_integer(index_type))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "va_arg index must be integer");
                return NULL;
            }

            expr->type = type_pointer_create(type_u8());
            return expr->type;
        }
    }

    Type *func_type = analyze_expr(driver, ctx, expr->call_expr.func);
    if (!func_type)
        return NULL;

    func_type = type_resolve_alias(func_type);
    if (!func_type)
        return NULL;

    Symbol *func_sym       = expr->call_expr.func ? expr->call_expr.func->symbol : NULL;
    Symbol *base_sym       = func_sym && func_sym->import_origin ? func_sym->import_origin : func_sym;
    size_t  type_arg_count = expr->call_expr.type_args ? expr->call_expr.type_args->count : 0;

    // specialize generic functions on demand (e.g. mem.alloc<T>)
    if (base_sym && base_sym->kind == SYMBOL_FUNC && base_sym->func.is_generic)
    {
        AstNode *decl                = base_sym->decl;
        size_t   generic_param_count = (decl && decl->kind == AST_STMT_FUN && decl->fun_stmt.generics) ? (size_t)decl->fun_stmt.generics->count : 0;
        if (generic_param_count == 0)
        {
            // defensive: symbol marked generic but no params
            base_sym->func.is_generic = false;
        }
        else
        {
            if (type_arg_count != generic_param_count)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "generic function '%s' requires %zu type arguments, got %zu", base_sym->name, generic_param_count, type_arg_count);
                return NULL;
            }

            Type **type_args = NULL;
            if (type_arg_count > 0)
            {
                type_args = malloc(sizeof(Type *) * type_arg_count);
                for (size_t i = 0; i < type_arg_count; i++)
                {
                    AstNode *type_arg_node = expr->call_expr.type_args->items[i];
                    type_args[i]           = resolve_type_in_context(driver, ctx, type_arg_node);
                    if (!type_args[i])
                    {
                        free(type_args);
                        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "failed to resolve type argument %zu for generic call", i + 1);
                        return NULL;
                    }
                }
            }

            Symbol *specialized = instantiate_generic_function(driver, ctx, base_sym, type_args, type_arg_count);
            free(type_args);

            if (!specialized)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "failed to instantiate generic function '%s'", base_sym->name);
                return NULL;
            }

            func_sym  = specialized;
            base_sym  = specialized;
            func_type = specialized->type;

            if (expr->call_expr.func)
            {
                expr->call_expr.func->symbol = specialized;
                expr->call_expr.func->type   = func_type;
            }
        }
    }

    if (func_type->kind != TYPE_FUNCTION)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot call non-function type");
        return NULL;
    }

    if (func_sym)
        expr->symbol = func_sym;

    // check argument count
    size_t expected = func_type->function.param_count;
    size_t provided = expr->call_expr.args ? expr->call_expr.args->count : 0;

    if (!func_type->function.is_variadic && provided != expected)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "function expects %zu arguments, got %zu", expected, provided);
        return NULL;
    }

    // type check arguments
    for (size_t i = 0; i < provided && i < expected; i++)
    {
        AstNode *arg_node = expr->call_expr.args->items[i];
        Type    *arg_type = analyze_expr(driver, ctx, arg_node);
        if (!arg_type)
            return NULL;

        Type *param_type = func_type->function.param_types[i];
        if (!type_equals(arg_type, param_type) && !type_can_assign_to(arg_type, param_type))
        {
            char *arg_type_str   = type_to_string(arg_type);
            char *param_type_str = type_to_string(param_type);
            diagnostic_emit(&driver->diagnostics,
                            DIAG_ERROR,
                            expr->call_expr.args->items[i],
                            ctx->file_path,
                            "argument %zu has incompatible type (expected '%s', got '%s')",
                            i + 1,
                            param_type_str ? param_type_str : "<unknown>",
                            arg_type_str ? arg_type_str : "<unknown>");
            free(arg_type_str);
            free(param_type_str);
            return NULL;
        }
    }

    // for variadic functions, analyze remaining arguments
    if (func_type->function.is_variadic)
    {
        for (size_t i = expected; i < provided; i++)
        {
            AstNode *arg_node = expr->call_expr.args->items[i];
            Type    *arg_type = analyze_expr(driver, ctx, arg_node);
            if (!arg_type)
                return NULL;
        }
    }

    expr->type = func_type->function.return_type;
    return expr->type;
}

static Type *analyze_field_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    if (expr->field_expr.object->kind == AST_EXPR_IDENT)
    {
        const char *name = expr->field_expr.object->ident_expr.name;
        Symbol     *sym  = lookup_in_scope_chain(ctx->current_scope, ctx->module_scope, ctx->global_scope, name);
        if (sym && sym->kind == SYMBOL_MODULE)
        {
            const char *member_name = expr->field_expr.field;
            Symbol     *member      = symbol_lookup_scope(sym->module.scope, member_name);
            if (!member)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "module '%s' has no member '%s'", name, member_name);
                return NULL;
            }
            if (!member->is_public)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "member '%s' of module '%s' is not public", member_name, name);
                return NULL;
            }

            Symbol *target = member;
            if ((!member->type || member->type->kind == TYPE_ERROR) && member->import_origin && member->import_origin->type)
                member->type = member->import_origin->type;
            if (member->import_origin && member->import_origin->type)
                target = member->import_origin;
            if (!member->has_const_i64 && member->import_origin && member->import_origin->has_const_i64)
            {
                member->has_const_i64 = true;
                member->const_i64     = member->import_origin->const_i64;
            }

            expr->field_expr.object->symbol = sym;
            expr->symbol                    = target;
            expr->type                      = target->type;
            return expr->type;
        }
    }

    Type *object_type = analyze_expr(driver, ctx, expr->field_expr.object);
    if (!object_type)
        return NULL;

    if (object_type->kind == TYPE_POINTER)
        object_type = object_type->pointer.base;

    object_type = type_resolve_alias(object_type);

    if (object_type->kind == TYPE_ARRAY)
    {
        if (strcmp(expr->field_expr.field, "len") == 0)
        {
            expr->type = type_u64();
            return expr->type;
        }
        if (strcmp(expr->field_expr.field, "data") == 0)
        {
            expr->type = type_pointer_create(object_type->array.elem_type);
            return expr->type;
        }
    }

    if (object_type->kind != TYPE_STRUCT && object_type->kind != TYPE_UNION)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "member access on non-composite type");
        return NULL;
    }

    for (Symbol *field = object_type->composite.fields; field; field = field->next)
    {
        if (field->name && strcmp(field->name, expr->field_expr.field) == 0)
        {
            expr->symbol = field;
            expr->type   = field->type;
            return field->type;
        }
    }

    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "no member named '%s'", expr->field_expr.field);
    return NULL;
}

static Type *analyze_index_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    Type *array_type = analyze_expr(driver, ctx, expr->index_expr.array);
    Type *index_type = analyze_expr(driver, ctx, expr->index_expr.index);

    if (!array_type || !index_type)
        return NULL;

    // Store original type to preserve aliases
    Type *original_array_type = array_type;

    // dereference pointer
    if (array_type->kind == TYPE_POINTER)
        array_type = array_type->pointer.base;

    array_type = type_resolve_alias(array_type);

    if (array_type->kind != TYPE_ARRAY)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot index non-array type");
        return NULL;
    }

    if (!type_is_integer(index_type))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr->index_expr.index, ctx->file_path, "array index must be integer");
        return NULL;
    }

    // Try to get element type from original (unresolved) type to preserve aliases
    Type *elem_type = array_type->array.elem_type;
    if (original_array_type->kind == TYPE_ARRAY)
    {
        // Original was already an array, use its element type
        elem_type = original_array_type->array.elem_type;
    }

    expr->type = elem_type;
    return expr->type;
}

static Type *analyze_cast_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    Type *source = analyze_expr(driver, ctx, expr->cast_expr.expr);
    Type *target = resolve_type_in_context(driver, ctx, expr->cast_expr.type);

    if (!source || !target)
        return NULL;

    // validate cast is legal
    bool valid = false;

    // same size types can always be cast (reinterpret bits)
    if (source->size == target->size)
        valid = true;

    // numeric to numeric (different sizes - truncate or extend)
    else if (type_is_numeric(source) && type_is_numeric(target))
        valid = true;

    // pointer to pointer (always allowed)
    else if (source->kind == TYPE_POINTER && target->kind == TYPE_POINTER)
        valid = true;

    // integer to pointer or pointer to integer (unsafe but allowed)
    else if ((type_is_integer(source) && target->kind == TYPE_POINTER) || (source->kind == TYPE_POINTER && type_is_integer(target)))
        valid = true;

    if (!valid)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "invalid cast from '%s' to '%s'", source->name, target->name);
        return NULL;
    }

    expr->type = target;
    return target;
}

static Type *analyze_struct_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    // resolve struct/union type
    Type *struct_type = NULL;
    if (expr->struct_expr.type)
    {
        struct_type = resolve_type_in_context(driver, ctx, expr->struct_expr.type);
        if (!struct_type)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot resolve type in struct literal");
            return NULL;
        }
    }
    else if (expr->type)
    {
        // type was pre-set by parent (e.g., field initializer)
        struct_type = expr->type;
    }
    else
    {
        // Type inference not yet implemented - for now require explicit type
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "struct literal requires explicit type (type inference not yet implemented)");
        return NULL;
    }

    // resolve type aliases
    Type *resolved_type = type_resolve_alias(struct_type);
    if (!resolved_type)
        resolved_type = struct_type;

    // validate union literal
    if (expr->struct_expr.is_union_literal && resolved_type->kind != TYPE_UNION)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "'uni' literal requires a union type");
        return NULL;
    }

    // analyze field initializers
    // Look up field types from the struct/union and use them for type inference
    if (expr->struct_expr.fields && resolved_type->kind == TYPE_STRUCT)
    {
        for (int i = 0; i < expr->struct_expr.fields->count; i++)
        {
            AstNode *field_init = expr->struct_expr.fields->items[i];
            if (field_init && field_init->kind == AST_EXPR_FIELD)
            {
                const char *field_name = field_init->field_expr.field;
                AstNode    *value_expr = field_init->field_expr.object;

                // find field in struct type
                Symbol *field_sym  = resolved_type->composite.fields;
                Type   *field_type = NULL;
                while (field_sym)
                {
                    if (strcmp(field_sym->name, field_name) == 0)
                    {
                        field_type = field_sym->type;
                        break;
                    }
                    field_sym = field_sym->next;
                }

                // if value is also a struct literal without type, inject the field's type
                Type *value_type = analyze_expr_with_hint(driver, ctx, value_expr, field_type);
                if (!value_type)
                    return NULL;

                if (!ensure_assignable(driver, ctx, field_type, value_type, value_expr, "field initializer"))
                    return NULL;
            }
        }
    }
    else if (expr->struct_expr.fields)
    {
        // for unions or other cases, just analyze without inference
        for (int i = 0; i < expr->struct_expr.fields->count; i++)
        {
            AstNode *field_init = expr->struct_expr.fields->items[i];
            if (field_init && field_init->kind == AST_EXPR_FIELD)
            {
                if (!analyze_expr(driver, ctx, field_init->field_expr.object))
                    return NULL;
            }
        }
    }

    expr->type = struct_type;
    return struct_type;
}

static bool ensure_assignable(SemanticDriver *driver, const AnalysisContext *ctx, Type *expected, Type *actual, AstNode *site, const char *what)
{
    if (!expected || !actual)
        return false;

    if (type_equals(expected, actual) || type_can_assign_to(actual, expected))
        return true;

    char *expected_str = type_to_string(expected);
    char *actual_str   = type_to_string(actual);
    diagnostic_emit(&driver->diagnostics, DIAG_ERROR, site, ctx->file_path, "%s has incompatible type (expected %s, got %s)", what ? what : "value", expected_str ? expected_str : "<unknown>", actual_str ? actual_str : "<unknown>");
    free(expected_str);
    free(actual_str);
    return false;
}

static Type *analyze_array_as_struct(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *specified_type, Type *resolved_type)
{
    if (!expr->array_expr.elems)
    {
        expr->type = specified_type;
        return specified_type;
    }

    size_t field_count = 0;
    for (Symbol *field = resolved_type->composite.fields; field; field = field->next)
        field_count++;

    size_t elem_count = (size_t)expr->array_expr.elems->count;
    if (elem_count > field_count)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "too many initializers for composite literal (expected %zu, got %zu)", field_count, elem_count);
        return NULL;
    }

    Symbol *field_sym = resolved_type->composite.fields;
    for (size_t i = 0; i < elem_count && field_sym; i++, field_sym = field_sym->next)
    {
        AstNode *value      = expr->array_expr.elems->items[i];
        Type    *value_type = analyze_expr_with_hint(driver, ctx, value, field_sym->type);
        if (!value_type)
            return NULL;

        if (!ensure_assignable(driver, ctx, field_sym->type, value_type, value, "initializer"))
            return NULL;
    }

    expr->type = specified_type;
    return specified_type;
}

static Type *analyze_array_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    if (!expr->array_expr.type)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "array literal requires an explicit type");
        return NULL;
    }

    Type *specified_type = resolve_type_in_context(driver, ctx, expr->array_expr.type);
    if (!specified_type)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "cannot resolve array literal type");
        return NULL;
    }

    Type *resolved_type = type_resolve_alias(specified_type);
    if (!resolved_type)
        resolved_type = specified_type;

    if (resolved_type->kind == TYPE_ARRAY)
    {
        size_t elem_count     = expr->array_expr.elems ? (size_t)expr->array_expr.elems->count : 0;
        bool   treat_as_slice = false;

        if (expr->array_expr.type && expr->array_expr.type->kind == AST_TYPE_ARRAY)
        {
            treat_as_slice = (expr->array_expr.type->type_array.size == NULL);
        }
        else if ((!expr->array_expr.type || expr->array_expr.type->kind == AST_TYPE_NAME) && elem_count <= 2)
        {
            // type aliases to slices appear as names; treat them as slices as well
            treat_as_slice = true;
        }

        Type *elem_type = resolved_type->array.elem_type;

        if (treat_as_slice)
        {
            expr->array_expr.is_slice_literal = true;

            Type *data_type = type_pointer_create(elem_type);
            Type *len_type  = type_u64();

            if (elem_count == 0)
            {
                expr->type = specified_type;
                return specified_type;
            }

            if (elem_count > 2)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "slice literal expects at most data and length expressions");
                return NULL;
            }

            AstNode *data_expr = expr->array_expr.elems->items[0];
            Type    *data_val  = analyze_expr_with_hint(driver, ctx, data_expr, data_type);
            if (!data_val)
                return NULL;
            if (!ensure_assignable(driver, ctx, data_type, data_val, data_expr, "slice data"))
                return NULL;

            if (elem_count == 1)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "slice literal missing length expression");
                return NULL;
            }

            AstNode *len_expr = expr->array_expr.elems->items[1];
            Type    *len_val  = analyze_expr_with_hint(driver, ctx, len_expr, len_type);
            if (!len_val)
                return NULL;
            if (!ensure_assignable(driver, ctx, len_type, len_val, len_expr, "slice length"))
                return NULL;

            expr->type = specified_type;
            return specified_type;
        }

        for (size_t i = 0; i < elem_count; i++)
        {
            AstNode *elem       = expr->array_expr.elems->items[i];
            Type    *value_type = analyze_expr_with_hint(driver, ctx, elem, elem_type);
            if (!value_type)
                return NULL;

            if (!ensure_assignable(driver, ctx, elem_type, value_type, elem, "array element"))
                return NULL;
        }

        expr->type = specified_type;
        return specified_type;
    }

    if (resolved_type->kind == TYPE_STRUCT || resolved_type->kind == TYPE_UNION)
        return analyze_array_as_struct(driver, ctx, expr, specified_type, resolved_type);

    Type  *elem_type  = resolved_type;
    size_t elem_count = expr->array_expr.elems ? (size_t)expr->array_expr.elems->count : 0;

    for (size_t i = 0; i < elem_count; i++)
    {
        AstNode *elem       = expr->array_expr.elems->items[i];
        Type    *value_type = analyze_expr_with_hint(driver, ctx, elem, elem_type);
        if (!value_type)
            return NULL;

        if (!ensure_assignable(driver, ctx, elem_type, value_type, elem, "array element"))
            return NULL;
    }

    expr->type = type_array_create(elem_type);
    return expr->type;
}

static Type *analyze_expr_with_hint(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr, Type *expected)
{
    if (!expr)
        return NULL;

    switch (expr->kind)
    {
    case AST_EXPR_LIT:
        return analyze_lit_expr_with_hint(driver, ctx, expr, expected);
    case AST_EXPR_NULL:
        return analyze_null_expr_with_hint(driver, ctx, expr, expected);
    case AST_EXPR_ARRAY:
        return analyze_array_expr(driver, ctx, expr);
    case AST_EXPR_STRUCT:
        if (expected && !expr->struct_expr.type)
            expr->type = expected;
        return analyze_struct_expr(driver, ctx, expr);
    default:
        return analyze_expr(driver, ctx, expr);
    }
}

static Type *analyze_expr(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *expr)
{
    if (!expr)
        return NULL;

    switch (expr->kind)
    {
    case AST_EXPR_LIT:
        return analyze_lit_expr_with_hint(driver, ctx, expr, NULL);
    case AST_EXPR_IDENT:
        return analyze_ident_expr(driver, ctx, expr);
    case AST_EXPR_BINARY:
        return analyze_binary_expr(driver, ctx, expr);
    case AST_EXPR_UNARY:
        return analyze_unary_expr(driver, ctx, expr);
    case AST_EXPR_CALL:
        return analyze_call_expr(driver, ctx, expr);
    case AST_EXPR_FIELD:
        return analyze_field_expr(driver, ctx, expr);
    case AST_EXPR_INDEX:
        return analyze_index_expr(driver, ctx, expr);
    case AST_EXPR_CAST:
        return analyze_cast_expr(driver, ctx, expr);
    case AST_EXPR_STRUCT:
        return analyze_struct_expr(driver, ctx, expr);
    case AST_EXPR_ARRAY:
        return analyze_array_expr(driver, ctx, expr);
    case AST_EXPR_VARARGS:
        // varargs forwarding - only valid in variadic functions
        if (!ctx->current_function || !ctx->current_function->type || !ctx->current_function->type->function.is_variadic)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "varargs forwarding (...) can only be used in variadic functions");
            return NULL;
        }
        // varargs forwarding is a special marker - give it a placeholder type
        expr->type = type_u64();
        return expr->type;
    case AST_EXPR_NULL:
        return analyze_null_expr_with_hint(driver, ctx, expr, NULL);
    default:
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, expr, ctx->file_path, "unsupported expression kind");
        return NULL;
    }
}

// statement analysis
static bool analyze_block_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->block_stmt.stmts)
        return true;

    // create a new scope for the block
    Scope          *block_scope = scope_create(ctx->current_scope, "block");
    AnalysisContext block_ctx   = analysis_context_with_scope(ctx, block_scope);

    bool success = true;
    for (int i = 0; i < stmt->block_stmt.stmts->count; i++)
    {
        AstNode *inner = stmt->block_stmt.stmts->items[i];
        if (!analyze_stmt(driver, &block_ctx, inner))
        {
            success = false;
        }
    }

    // note: we don't free the scope - it's owned by the parent scope's children
    return success;
}

static bool analyze_expr_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    Type *type = analyze_expr(driver, ctx, stmt->expr_stmt.expr);
    if (!type)
    {
        if (stmt->expr_stmt.expr && stmt->expr_stmt.expr->kind == AST_EXPR_CALL)
        {
            Type *func_type = NULL;
            if (stmt->expr_stmt.expr->symbol && stmt->expr_stmt.expr->symbol->type && stmt->expr_stmt.expr->symbol->type->kind == TYPE_FUNCTION)
            {
                func_type = stmt->expr_stmt.expr->symbol->type;
            }
            else if (stmt->expr_stmt.expr->call_expr.func && stmt->expr_stmt.expr->call_expr.func->type && stmt->expr_stmt.expr->call_expr.func->type->kind == TYPE_FUNCTION)
            {
                func_type = stmt->expr_stmt.expr->call_expr.func->type;
            }

            if (func_type && func_type->function.return_type == NULL)
            {
                // void call used as expression is valid
                return true;
            }
        }

        return false;
    }
    return true;
}

static bool analyze_var_stmt_body(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    const char *name   = stmt->var_stmt.name;
    bool        is_val = stmt->var_stmt.is_val;

    // check if this is a global variable (symbol already created in Pass A)
    if (stmt->symbol && stmt->symbol->kind != SYMBOL_PARAM && ((stmt->symbol->kind == SYMBOL_VAR || stmt->symbol->kind == SYMBOL_VAL) && stmt->symbol->var.is_global))
    {
        // global variable - just analyze initializer
        if (stmt->var_stmt.init)
        {
            Type *var_type = stmt->type;
            // pass var_type as hint for correct literal typing
            Type *init_type = analyze_expr_with_hint(driver, ctx, stmt->var_stmt.init, var_type);
            if (!init_type)
            {
                return false;
            }

            if (var_type && !type_equals(var_type, init_type) && !type_can_assign_to(init_type, var_type))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "incompatible initializer type for variable '%s'", name);
                return false;
            }
        }
        return true;
    }

    // local variable - create symbol and analyze
    Type *var_type  = stmt->type;
    Type *init_type = NULL;

    if (!var_type)
    {
        if (stmt->var_stmt.type)
        {
            // explicit type annotation
            var_type = resolve_type_in_context(driver, ctx, stmt->var_stmt.type);
            if (!var_type)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "cannot resolve type for variable '%s'", name);
                var_type = type_error();
            }
        }
        else if (stmt->var_stmt.init)
        {
            // infer type from initializer
            init_type = analyze_expr(driver, ctx, stmt->var_stmt.init);
            if (!init_type)
                init_type = type_error();
            var_type = init_type;
        }
        else
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "variable '%s' has no type or initializer", name);
            var_type = type_error();
        }

        stmt->type = var_type;
    }

    // check for redefinition in current scope before installing symbol
    Symbol *existing = symbol_lookup_scope(ctx->current_scope, name);
    if (existing)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "redefinition of '%s'", name);
        return false;
    }

    // ensure symbol exists even if initializer fails, to avoid cascading undefined identifier errors
    SymbolKind kind       = is_val ? SYMBOL_VAL : SYMBOL_VAR;
    Symbol    *symbol     = symbol_create(kind, name, var_type, stmt);
    symbol->var.is_global = false;
    symbol->var.is_const  = is_val;
    symbol->home_scope    = ctx->current_scope;
    symbol_add(ctx->current_scope, symbol);
    stmt->symbol = symbol;

    // analyze initializer (if annotation present we may not have inspected yet)
    if (stmt->var_stmt.init && !init_type)
    {
        // pass var_type as hint so integer literals get the correct type
        init_type = analyze_expr_with_hint(driver, ctx, stmt->var_stmt.init, var_type);
        if (!init_type)
            return false;
    }

    if (init_type && init_type != type_error() && var_type && var_type != type_error())
    {
        if (!type_equals(var_type, init_type) && !type_can_assign_to(init_type, var_type))
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "incompatible initializer type for variable '%s'", name);
            return false;
        }
    }

    if (!var_type || var_type == type_error())
    {
        return false;
    }

    symbol->type = var_type;
    stmt->type   = var_type;

    return true;
}

static bool analyze_ret_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!ctx->current_function)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "return statement outside function");
        return false;
    }

    Type *func_ret = ctx->current_function->type->function.return_type;

    if (!stmt->ret_stmt.expr)
    {
        if (func_ret)
        {
            diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "function must return a value");
            return false;
        }
        return true;
    }

    Type *ret_type = analyze_expr(driver, ctx, stmt->ret_stmt.expr);
    if (!ret_type)
        return false;

    if (!func_ret)
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "void function cannot return a value");
        return false;
    }

    if (!type_equals(ret_type, func_ret) && !type_can_assign_to(ret_type, func_ret))
    {
        char *ret_type_str = type_to_string(ret_type);
        char *func_ret_str = type_to_string(func_ret);
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, ctx->file_path, "incompatible return type (expected '%s', got '%s')", func_ret_str ? func_ret_str : "<unknown>", ret_type_str ? ret_type_str : "<unknown>");
        free(ret_type_str);
        free(func_ret_str);
        return false;
    }

    return true;
}

static bool analyze_if_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    Type *cond_type = NULL;
    if (stmt->cond_stmt.cond)
    {
        cond_type = analyze_expr(driver, ctx, stmt->cond_stmt.cond);
        if (!cond_type)
        {
            return false;
        }
    }

    bool success = analyze_stmt(driver, ctx, stmt->cond_stmt.body);

    if (stmt->cond_stmt.stmt_or)
        success = analyze_stmt(driver, ctx, stmt->cond_stmt.stmt_or) && success;

    return success;
}

static bool analyze_for_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (stmt->for_stmt.cond)
    {
        Type *cond_type = analyze_expr(driver, ctx, stmt->for_stmt.cond);
        if (!cond_type)
        {
            return false;
        }
    }

    return analyze_stmt(driver, ctx, stmt->for_stmt.body);
}

static bool analyze_stmt(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt)
        return true;

    switch (stmt->kind)
    {
    case AST_STMT_BLOCK:
        return analyze_block_stmt(driver, ctx, stmt);
    case AST_STMT_EXPR:
        return analyze_expr_stmt(driver, ctx, stmt);
    case AST_STMT_VAR:
    case AST_STMT_VAL:
        return analyze_var_stmt_body(driver, ctx, stmt);
    case AST_STMT_RET:
        return analyze_ret_stmt(driver, ctx, stmt);
    case AST_STMT_IF:
    case AST_STMT_OR:
        return analyze_if_stmt(driver, ctx, stmt);
    case AST_STMT_FOR:
        return analyze_for_stmt(driver, ctx, stmt);
    default:
        return true;
    }
}

static bool analyze_function_body(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *stmt)
{
    if (!stmt->fun_stmt.body)
        return true; // external function

    // create a dedicated scope for function-local symbols
    const char *scope_name = stmt->fun_stmt.name ? stmt->fun_stmt.name : "<lambda>";
    Scope      *func_scope = scope_create(ctx->current_scope, scope_name);

    // analysis context for function body
    AnalysisContext func_ctx = analysis_context_with_function(ctx, stmt->symbol);
    func_ctx                 = analysis_context_with_scope(&func_ctx, func_scope);

    // add parameters to function local scope
    if (stmt->fun_stmt.params)
    {
        size_t param_count        = (size_t)stmt->fun_stmt.params->count;
        Type **symbol_params      = NULL;
        size_t symbol_param_count = 0;
        if (stmt->symbol && stmt->symbol->type && stmt->symbol->type->kind == TYPE_FUNCTION)
        {
            symbol_params      = stmt->symbol->type->function.param_types;
            symbol_param_count = stmt->symbol->type->function.param_count;
        }

        for (size_t i = 0; i < param_count; i++)
        {
            AstNode *param = stmt->fun_stmt.params->items[i];
            if (param->kind != AST_STMT_PARAM)
                continue;

            // for specialized functions, always use symbol's param types (not AST param->type which may be generic)
            Type *param_type = NULL;
            if (stmt->symbol && !stmt->symbol->func.is_generic && symbol_params && i < symbol_param_count)
            {
                // specialized function - use resolved types from symbol
                param_type = symbol_params[i];
            }
            else
            {
                // normal or generic template function - use AST type
                param_type = param->type;
                if (!param_type && symbol_params && i < symbol_param_count)
                    param_type = symbol_params[i];
            }

            if (!param_type)
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, param, ctx->file_path, "parameter '%s' has no resolved type", param->param_stmt.name);
                continue;
            }

            // only update AST node for non-specialized functions to avoid modifying shared generic templates
            if (stmt->symbol && stmt->symbol->func.is_generic)
            {
                // leave generic template AST unchanged
            }
            else
            {
                param->type = param_type;
            }

            if (symbol_lookup_scope(func_ctx.current_scope, param->param_stmt.name))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, param, ctx->file_path, "parameter '%s' conflicts with existing symbol", param->param_stmt.name);
                continue;
            }

            // parameters behave like immutable locals
            Symbol *param_sym      = symbol_create(SYMBOL_PARAM, param->param_stmt.name, param_type, param);
            param_sym->param.index = (size_t)i;
            param->symbol          = param_sym; // set symbol on parameter node
            symbol_add(func_ctx.current_scope, param_sym);
        }
    }

    bool success = analyze_stmt(driver, &func_ctx, stmt->fun_stmt.body);

    // function scope currently leaks; TODO: manage scope lifetime if needed
    return success;
}

static bool analyze_pass_c_bodies(SemanticDriver *driver, const AnalysisContext *ctx, AstNode *root)
{
    if (!root || root->kind != AST_PROGRAM)
        return false;

    bool success = true;

    // analyze global initializers
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if ((stmt->kind == AST_STMT_VAL || stmt->kind == AST_STMT_VAR) && stmt->var_stmt.init)
        {
            if (!analyze_var_stmt_body(driver, ctx, stmt))
            {
                fprintf(stderr, "error: failed to analyze global variable '%s'\n", stmt->var_stmt.name ? stmt->var_stmt.name : "<anon>");
                success = false;
            }
        }
    }

    // analyze function bodies (including methods)
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];

        if (stmt->kind == AST_STMT_FUN && stmt->symbol && !stmt->symbol->func.is_generic)
        {
            if (!analyze_function_body(driver, ctx, stmt))
            {
                fprintf(stderr, "error: failed to analyze function '%s'\n", stmt->fun_stmt.name ? stmt->fun_stmt.name : "<anon>");
                success = false;
            }
        }
    }

    return success;
}

bool semantic_driver_analyze(SemanticDriver *driver, AstNode *root, const char *module_name, const char *module_path)
{
    driver->program_root      = root;
    driver->entry_module_name = module_path;

    // initialize type system
    type_system_init();

    // create global scope and store in driver for codegen
    symbol_table_init(&driver->symbol_table);

    Scope          *global_scope = driver->symbol_table.global_scope;
    AnalysisContext ctx          = analysis_context_create(global_scope, NULL, module_name, module_path);

    bool success = true;

    // Enqueue dependencies from entry module
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];
        if (stmt->kind == AST_STMT_USE)
        {
            const char *dep_path = stmt->use_stmt.module_path;
            if (dep_path && !enqueue_module(driver, dep_path))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, module_path, "failed to load module '%s'", dep_path);
                success = false;
            }
        }
    }

    if (!success)
    {
        if (driver->diagnostics.count > 0)
            diagnostic_print_all(&driver->diagnostics, &driver->module_manager);
        return false;
    }

    // First, declare symbols in entry module
    if (!analyze_pass_a_declarations(driver, &ctx, root))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "declaration pass failed for entry module");
        success = false;
    }

    // Then, declare symbols in all loaded modules
    FOR_EACH_MODULE(&driver->module_manager, module)
    {
        if (success && module->ast && module->ast->kind == AST_PROGRAM)
        {
            Scope          *module_scope = module->symbols->global_scope;
            AnalysisContext module_ctx   = analysis_context_create(module_scope, module_scope, module->name, module->file_path);

            if (!analyze_pass_a_declarations(driver, &module_ctx, module->ast))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module->name, "declaration pass failed for module '%s'", module->name);
                success = false;
            }
        }
    }

    if (!success)
    {
        if (driver->diagnostics.count > 0)
            diagnostic_print_all(&driver->diagnostics, &driver->module_manager);
        return false;
    }

    // Process imports in entry module
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];
        if (stmt->kind == AST_STMT_USE)
        {
            if (!process_use_statement(driver, &ctx, stmt))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, module_path, "failed to process use statement");
                success = false;
            }
        }
    }

    // Process imports in all loaded modules
    FOR_EACH_MODULE(&driver->module_manager, module)
    {
        if (success && module->ast && module->ast->kind == AST_PROGRAM)
        {
            Scope          *module_scope = module->symbols->global_scope;
            AnalysisContext module_ctx   = analysis_context_create(module_scope, module_scope, module->name, module->file_path);

            for (int j = 0; j < module->ast->program.stmts->count; j++)
            {
                AstNode *stmt = module->ast->program.stmts->items[j];
                if (stmt->kind == AST_STMT_USE)
                {
                    if (!process_use_statement(driver, &module_ctx, stmt))
                    {
                        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, stmt, module->name, "failed to process use statement in module '%s'", module->name);
                        success = false;
                    }
                }
            }
        }
    }

    if (!success)
    {
        if (driver->diagnostics.count > 0)
            diagnostic_print_all(&driver->diagnostics, &driver->module_manager);
        return false;
    }

    // Resolve signatures in entry module
    if (!analyze_pass_b_signatures(driver, &ctx, root))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "signature resolution pass failed for entry module");
        success = false;
    }

    // Resolve signatures in all loaded modules
    FOR_EACH_MODULE(&driver->module_manager, module)
    {
        if (success && module->ast && module->ast->kind == AST_PROGRAM)
        {
            Scope          *module_scope = module->symbols->global_scope;
            AnalysisContext module_ctx   = analysis_context_create(module_scope, module_scope, module->name, module->file_path);

            if (!analyze_pass_b_signatures(driver, &module_ctx, module->ast))
            {
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module->name, "signature resolution pass failed for module '%s'", module->name);
                success = false;
            }
        }
    }

    if (!success)
    {
        if (driver->diagnostics.count > 0)
            diagnostic_print_all(&driver->diagnostics, &driver->module_manager);
        return false;
    }

    // Analyze bodies in entry module
    if (!analyze_pass_c_bodies(driver, &ctx, root))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "body analysis pass failed for entry module");
        success = false;
    }

    // Analyze bodies in all loaded modules
    FOR_EACH_MODULE(&driver->module_manager, module)
    {
        if (success && module->ast && module->ast->kind == AST_PROGRAM)
        {
            Scope          *module_scope = module->symbols->global_scope;
            AnalysisContext module_ctx   = analysis_context_create(module_scope, module_scope, module->name, module->file_path);

            if (!analyze_pass_c_bodies(driver, &module_ctx, module->ast))
            {
                fprintf(stderr, "error: body analysis failed in module '%s'\n", module->name ? module->name : "<unknown>");
                diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module->name, "body analysis pass failed for module '%s'", module->name);
                success = false;
            }
        }
    }

    if (!success)
    {
        if (driver->diagnostics.count > 0)
            diagnostic_print_all(&driver->diagnostics, &driver->module_manager);
        return false;
    }

    // Mark all modules as analyzed and needing linking
    FOR_EACH_MODULE(&driver->module_manager, module)
    {
        module->is_analyzed = true;
        // mark module for linking if it has an AST with statements
        if (module->ast && module->ast->kind == AST_PROGRAM && module->ast->program.stmts && module->ast->program.stmts->count > 0)
        {
            module->needs_linking = true;
        }
    }

    if (!process_instantiation_queue(driver, &ctx))
    {
        diagnostic_emit(&driver->diagnostics, DIAG_ERROR, NULL, module_path, "generic instantiation failed");
        success = false;
    }

    // print diagnostics
    if (driver->diagnostics.count > 0)
    {
        // count actual errors
        size_t error_count = 0;
        for (size_t i = 0; i < driver->diagnostics.count; i++)
        {
            if (driver->diagnostics.entries[i].level == DIAG_ERROR)
                error_count++;
        }

        diagnostic_print_all(&driver->diagnostics, &driver->module_manager);

        if (error_count > 0)
        {
            fprintf(stderr, "semantic analysis failed with %zu error(s)\n", error_count);
        }
    }

    return success && !driver->diagnostics.has_errors;
}
