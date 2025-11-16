#include "module.h"
#include "codegen.h"
#include "config.h"
#include "filesystem.h"
#include "ioutil.h"
#include "lexer.h"
#include "symbol.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char *module_manager_vformat(const char *fmt, va_list args)
{
    va_list copy;
    va_copy(copy, args);
    int needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0)
        return NULL;

    char *buf = malloc((size_t)needed + 1);
    if (!buf)
        return NULL;

    if (vsnprintf(buf, (size_t)needed + 1, fmt, args) < 0)
    {
        free(buf);
        return NULL;
    }
    return buf;
}

static char *module_manager_format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *result = module_manager_vformat(fmt, args);
    va_end(args);
    return result;
}

static char *module_strndup_local(const char *start, size_t len)
{
    char *out = malloc(len + 1);
    if (!out)
        return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int module_path_is_absolute(const char *path)
{
    if (!path || path[0] == '\0')
        return 0;
    if (path[0] == '/' || path[0] == '\\')
        return 1;
    if (strlen(path) >= 2 && path[1] == ':')
        return 1;
    return 0;
}

static char *module_manager_build_vendor_dir(ModuleManager *manager, const char *dep_name)
{
    if (!manager || !manager->project_dir || !dep_name)
        return NULL;

    ProjectConfig *config = (ProjectConfig *)manager->config;
    if (!config)
        return NULL;

    char *dep_root = config_resolve_dep_dir(config, manager->project_dir);
    if (!dep_root)
        return NULL;

    size_t len  = strlen(dep_root) + 1 + strlen(dep_name) + 1;
    char  *path = malloc(len);
    if (!path)
    {
        free(dep_root);
        return NULL;
    }
    snprintf(path, len, "%s/%s", dep_root, dep_name);
    free(dep_root);
    return path;
}

static char *module_manager_resolve_dep_path(ModuleManager *manager, DepSpec *dep)
{
    if (!dep || !dep->path || dep->path[0] == '\0')
        return NULL;

    if (module_path_is_absolute(dep->path) || !manager || !manager->project_dir)
        return strdup(dep->path);

    size_t len  = strlen(manager->project_dir) + 1 + strlen(dep->path) + 1;
    char  *path = malloc(len);
    if (!path)
        return NULL;
    snprintf(path, len, "%s/%s", manager->project_dir, dep->path);
    return path;
}

static char *module_manager_build_resolution_message(ModuleManager *manager, const char *import_name, const char *canonical_name)
{
    if (!manager || !manager->config || !manager->project_dir || !canonical_name)
        return NULL;

    const char *use_name = import_name ? import_name : canonical_name;
    const char *dot      = strchr(canonical_name, '.');
    if (!dot)
        return module_manager_format("module '%s' must include a package prefix", use_name);

    size_t prefix_len = (size_t)(dot - canonical_name);
    char  *prefix     = module_strndup_local(canonical_name, prefix_len);
    if (!prefix)
        return NULL;

    ProjectConfig *config = (ProjectConfig *)manager->config;

    if (config->id && strlen(config->id) == prefix_len && strncmp(config->id, canonical_name, prefix_len) == 0)
    {
        char *src_dir = config_resolve_src_dir(config, manager->project_dir);
        char *message;
        if (src_dir)
        {
            message = module_manager_format("module '%s' not found under %s", use_name, src_dir);
            free(src_dir);
        }
        else
        {
            message = module_manager_format("module '%s' not found; failed to resolve src directory", use_name);
        }
        free(prefix);
        return message;
    }

    DepSpec *dep = config_get_dep(config, prefix);
    if (!dep)
    {
        char *message = module_manager_format("unknown module prefix '%s' in import '%s'", prefix, use_name);
        free(prefix);
        return message;
    }

    char *vendor_dir = module_manager_build_vendor_dir(manager, dep->name ? dep->name : prefix);
    if (vendor_dir && !fs_is_directory(vendor_dir))
    {
        char *message = module_manager_format("dependency '%s' is not vendored (expected %s)", prefix, vendor_dir);
        free(vendor_dir);
        free(prefix);
        return message;
    }
    if (vendor_dir)
        free(vendor_dir);

    char *dep_path = module_manager_resolve_dep_path(manager, dep);
    if (dep_path && !fs_is_directory(dep_path))
    {
        char *message = module_manager_format("dependency '%s' path '%s' does not exist", prefix, dep_path);
        free(dep_path);
        free(prefix);
        return message;
    }
    if (dep_path)
        free(dep_path);

    free(prefix);
    return NULL;
}

// error handling functions
void module_error_list_init(ModuleErrorList *list)
{
    list->errors   = NULL;
    list->count    = 0;
    list->capacity = 0;
}

void module_error_list_dnit(ModuleErrorList *list)
{
    for (int i = 0; i < list->count; i++)
    {
        free(list->errors[i]->module_path);
        free(list->errors[i]->file_path);
        free(list->errors[i]->message);
        free(list->errors[i]);
    }
    free(list->errors);
}

void module_error_list_add(ModuleErrorList *list, const char *module_path, const char *file_path, const char *message)
{
    if (list->count >= list->capacity)
    {
        list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->errors   = realloc(list->errors, list->capacity * sizeof(ModuleError *));
    }

    ModuleError *error = malloc(sizeof(ModuleError));
    error->module_path = strdup(module_path);
    error->file_path   = strdup(file_path);
    error->message     = strdup(message);

    list->errors[list->count++] = error;
}

void module_error_list_print(ModuleErrorList *list)
{
    for (int i = 0; i < list->count; i++)
    {
        ModuleError *error = list->errors[i];
        fprintf(stderr, "error in module '%s' (%s): %s\n", error->module_path, error->file_path, error->message);
    }
}

// simple hash function for module names
static unsigned int hash_module_name(const char *name, int capacity)
{
    unsigned int hash = 5381;
    for (int i = 0; name[i]; i++)
    {
        hash = ((hash << 5) + hash) + name[i];
    }
    return hash % capacity;
}

static Module *module_manager_find_canonical(ModuleManager *manager, const char *canonical_name)
{
    unsigned int index  = hash_module_name(canonical_name, manager->capacity);
    Module      *module = manager->modules[index];

    while (module)
    {
        if (strcmp(module->name, canonical_name) == 0)
        {
            return module;
        }
        module = module->next;
    }

    return NULL;
}

Module *module_manager_find_by_file_path(ModuleManager *manager, const char *file_path)
{
    if (!manager || !file_path)
        return NULL;

    // iterate through all modules and find by file_path
    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            if (module->file_path && strcmp(module->file_path, file_path) == 0)
            {
                return module;
            }
            module = module->next;
        }
    }

    return NULL;
}

static void module_manager_resize(ModuleManager *manager)
{
    int      old_capacity = manager->capacity;
    Module **old_modules  = manager->modules;

    manager->capacity = old_capacity * 2;
    manager->modules  = calloc(manager->capacity, sizeof(Module *));

    // rehash all modules
    for (int i = 0; i < old_capacity; i++)
    {
        Module *module = old_modules[i];
        while (module)
        {
            Module      *next       = module->next;
            unsigned int index      = hash_module_name(module->name, manager->capacity);
            module->next            = manager->modules[index];
            manager->modules[index] = module;
            module                  = next;
        }
    }

    free(old_modules);
}

static char *module_manager_expand_module(ModuleManager *manager, const char *module_fqn)
{
    if (!module_fqn)
        return NULL;
    if (!manager->config)
        return strdup(module_fqn);
    char *expanded = config_expand_module_path((ProjectConfig *)manager->config, module_fqn);
    // if no expansion occurred, config returns a copy - check if it's the same
    if (expanded && strcmp(expanded, module_fqn) == 0)
        return expanded;
    return expanded ? expanded : strdup(module_fqn);
}

// build a best-guess file path for diagnostics even if the file doesn't exist
static char *module_guess_file_path(ModuleManager *manager, const char *module_fqn)
{
    if (!module_fqn)
        return NULL;

    // if config is present, try config-based resolve (even if it may point to non-existent)
    if (manager->config && manager->project_dir)
    {
        char *guess = config_resolve_module_fqn((ProjectConfig *)manager->config, manager->project_dir, module_fqn);
        if (guess)
            return guess;
    }

    const char *dot      = strchr(module_fqn, '.');
    size_t      head_len = dot ? (size_t)(dot - module_fqn) : strlen(module_fqn);
    const char *tail     = dot ? dot + 1 : NULL;

    // alias paths take precedence
    for (int i = 0; i < manager->alias_count; i++)
    {
        if (strlen(manager->alias_names[i]) == head_len && strncmp(manager->alias_names[i], module_fqn, head_len) == 0)
        {
            const char *base     = manager->alias_paths[i];
            size_t      base_len = strlen(base);
            if (tail)
            {
                size_t tail_len = strlen(tail);
                char  *tail_buf = malloc(tail_len + 1);
                if (!tail_buf)
                    return NULL;
                memcpy(tail_buf, tail, tail_len + 1);
                for (char *p = tail_buf; *p; ++p)
                    if (*p == '.')
                        *p = '/';
                size_t path_len = base_len + 1 + tail_len + 5 + 1;
                char  *path     = malloc(path_len);
                if (!path)
                {
                    free(tail_buf);
                    return NULL;
                }
                snprintf(path, path_len, "%s/%s.mach", base, tail_buf);
                free(tail_buf);
                return path;
            }
            else
            {
                size_t path_len = base_len + 1 + 4 + 5 + 1; // "/" + "main" + ".mach" + NUL
                char  *path     = malloc(path_len);
                if (!path)
                    return NULL;
                snprintf(path, path_len, "%s/main.mach", base);
                return path;
            }
        }
    }

    // fallback to first search path, if any
    if (manager->search_count > 0)
    {
        const char *base     = manager->search_paths[0];
        size_t      base_len = strlen(base);
        if (tail)
        {
            size_t tail_len = strlen(tail);
            char  *tail_buf = malloc(tail_len + 1);
            if (!tail_buf)
                return NULL;
            memcpy(tail_buf, tail, tail_len + 1);
            for (char *p = tail_buf; *p; ++p)
                if (*p == '.')
                    *p = '/';
            size_t path_len = base_len + 1 + head_len + 1 + tail_len + 5 + 1;
            char  *path     = malloc(path_len);
            if (!path)
            {
                free(tail_buf);
                return NULL;
            }
            snprintf(path, path_len, "%s/%.*s/%s.mach", base, (int)head_len, module_fqn, tail_buf);
            free(tail_buf);
            return path;
        }
        else
        {
            size_t path_len = base_len + 1 + head_len + 1 + 4 + 5 + 1;
            char  *path     = malloc(path_len);
            if (!path)
                return NULL;
            snprintf(path, path_len, "%s/%.*s/main.mach", base, (int)head_len, module_fqn);
            return path;
        }
    }

    return NULL;
}

void module_manager_init(ModuleManager *manager)
{
    manager->capacity     = 32;
    manager->count        = 0;
    manager->modules      = calloc(manager->capacity, sizeof(Module *));
    manager->search_paths = NULL;
    manager->search_count = 0;
    manager->alias_names  = NULL;
    manager->alias_paths  = NULL;
    manager->alias_count  = 0;
    manager->config       = NULL;
    manager->project_dir  = NULL;

    module_error_list_init(&manager->errors);
    manager->had_error = false;
}

void module_manager_dnit(ModuleManager *manager)
{
    // clean up modules
    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            Module *next = module->next;
            module_dnit(module);
            free(module);
            module = next;
        }
    }
    free(manager->modules);

    // clean up search paths
    for (int i = 0; i < manager->search_count; i++)
    {
        free(manager->search_paths[i]);
    }
    free(manager->search_paths);

    // clean up aliases
    for (int i = 0; i < manager->alias_count; i++)
    {
        free(manager->alias_names[i]);
        free(manager->alias_paths[i]);
    }
    free(manager->alias_names);
    free(manager->alias_paths);

    // clean up errors
    module_error_list_dnit(&manager->errors);
}

// helper to build path from base, module name parts, and tail
static char *build_module_path(const char *base, const char *head, size_t head_len, const char *tail)
{
    if (!base)
        return NULL;

    size_t base_len = strlen(base);
    char  *path     = NULL;

    if (tail)
    {
        // convert dots to slashes in tail
        size_t tail_len = strlen(tail);
        char  *tail_buf = malloc(tail_len + 1);
        if (!tail_buf)
            return NULL;
        memcpy(tail_buf, tail, tail_len + 1);
        for (char *p = tail_buf; *p; ++p)
            if (*p == '.')
                *p = '/';

        if (head)
        {
            size_t path_len = base_len + 1 + head_len + 1 + tail_len + 5 + 1;
            path            = malloc(path_len);
            if (path)
                snprintf(path, path_len, "%s/%.*s/%s.mach", base, (int)head_len, head, tail_buf);
        }
        else
        {
            size_t path_len = base_len + 1 + tail_len + 5 + 1;
            path            = malloc(path_len);
            if (path)
                snprintf(path, path_len, "%s/%s.mach", base, tail_buf);
        }
        free(tail_buf);
    }
    else
    {
        // no tail means we want main.mach
        if (head)
        {
            size_t path_len = base_len + 1 + head_len + 1 + 9 + 1;
            path            = malloc(path_len);
            if (path)
                snprintf(path, path_len, "%s/%.*s/main.mach", base, (int)head_len, head);
        }
        else
        {
            size_t path_len = base_len + 1 + 9 + 1;
            path            = malloc(path_len);
            if (path)
                snprintf(path, path_len, "%s/main.mach", base);
        }
    }

    if (!path)
        return NULL;

    // check if path exists
    if (fs_file_exists(path))
        return path;

    free(path);
    return NULL;
}

void module_manager_add_search_path(ModuleManager *manager, const char *path)
{
    manager->search_paths                        = realloc(manager->search_paths, (manager->search_count + 1) * sizeof(char *));
    manager->search_paths[manager->search_count] = strdup(path);
    manager->search_count++;
}

void module_manager_add_alias(ModuleManager *manager, const char *name, const char *base_dir)
{
    if (!name || !base_dir)
        return;
    manager->alias_names                       = realloc(manager->alias_names, (manager->alias_count + 1) * sizeof(char *));
    manager->alias_paths                       = realloc(manager->alias_paths, (manager->alias_count + 1) * sizeof(char *));
    manager->alias_names[manager->alias_count] = strdup(name);
    manager->alias_paths[manager->alias_count] = strdup(base_dir);
    manager->alias_count++;
}

void module_manager_set_config(ModuleManager *manager, void *config, const char *project_dir)
{
    manager->config      = config;
    manager->project_dir = project_dir;
}

char *module_path_to_file_path(ModuleManager *manager, const char *module_fqn)
{
    if (!module_fqn)
        return NULL;

    // prefer config resolution if available
    if (manager->config && manager->project_dir)
    {
        char *resolved = config_resolve_module_fqn((ProjectConfig *)manager->config, manager->project_dir, module_fqn);
        if (resolved && fs_file_exists(resolved))
        {
            return resolved;
        }
        free(resolved);
    }

    const char *dot      = strchr(module_fqn, '.');
    size_t      head_len = dot ? (size_t)(dot - module_fqn) : strlen(module_fqn);
    const char *tail     = dot ? dot + 1 : NULL;

    // aliases: name -> base dir
    for (int i = 0; i < manager->alias_count; i++)
    {
        if (strlen(manager->alias_names[i]) == head_len && strncmp(manager->alias_names[i], module_fqn, head_len) == 0)
        {
            char *path = build_module_path(manager->alias_paths[i], NULL, 0, tail);
            if (path)
                return path;
        }
    }

    // search paths: base/name/(tail or main).mach
    for (int si = 0; si < manager->search_count; si++)
    {
        char *path = build_module_path(manager->search_paths[si], module_fqn, head_len, tail);
        if (path)
            return path;
    }

    return NULL;
}

Module *module_manager_find_module(ModuleManager *manager, const char *name)
{
    char *canonical = module_manager_expand_module(manager, name);
    if (!canonical)
        return NULL;

    Module *module = module_manager_find_canonical(manager, canonical);
    free(canonical);
    return module;
}

static Module *module_manager_load_module_internal(ModuleManager *manager, const char *module_fqn, const char *base_dir)
{
    (void)base_dir; // currently unused, for future relative path support

    char *canonical = module_manager_expand_module(manager, module_fqn);
    if (!canonical)
    {
        module_error_list_add(&manager->errors, module_fqn, "<unknown>", "could not resolve module path");
        manager->had_error = true;
        return NULL;
    }

    // check if already loaded
    Module *existing = module_manager_find_canonical(manager, canonical);
    if (existing)
    {
        free(canonical);
        return existing;
    }

    // check for circular dependencies in already loaded modules
    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            if (module_has_circular_dependency(manager, module, canonical))
            {
                module_error_list_add(&manager->errors, canonical, "<unknown>", "Circular dependency detected");
                manager->had_error = true;
                free(canonical);
                return NULL;
            }
            module = module->next;
        }
    }

    // find file path via canonical FQN
    char *file_path = module_path_to_file_path(manager, canonical);
    if (!file_path)
    {
        char *guess    = module_guess_file_path(manager, canonical);
        char *diagnose = module_manager_build_resolution_message(manager, module_fqn, canonical);
        module_error_list_add(&manager->errors, canonical, guess ? guess : "<unknown>", diagnose ? diagnose : "Could not find module file");
        if (diagnose)
            free(diagnose);
        if (guess)
            free(guess);
        manager->had_error = true;
        free(canonical);
        return NULL;
    }

    // load source from file
    char *source = read_file(file_path);
    if (!source)
    {
        module_error_list_add(&manager->errors, canonical, file_path, "Failed to read module file");
        manager->had_error = true;
        free(file_path);
        free(canonical);
        return NULL;
    }

    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    AstNode *ast = parser_parse_program(&parser); // modules inside will keep raw paths; normalization handled earlier

    // check for parse errors
    if (!ast || parser.had_error)
    {
        if (parser.had_error)
        {
            fprintf(stderr, "parsing failed with %d error(s):\n", parser.errors.count);
            parser_error_list_print(&parser.errors, &lexer, file_path);

            // add parsing errors to module error list
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "parsing failed with %d error(s)", parser.errors.count);
            module_error_list_add(&manager->errors, canonical, file_path, error_msg);
        }
        else
        {
            module_error_list_add(&manager->errors, canonical, file_path, "Failed to parse module");
        }

        manager->had_error = true;

        if (ast)
        {
            ast_node_dnit(ast);
            free(ast);
        }
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        free(file_path);
        free(canonical);

        return NULL;
    }

    // no info output

    // create module
    Module *module = malloc(sizeof(Module));
    module_init(module, canonical, file_path);
    module->ast         = ast;
    module->source      = source; // cache source for debug info and diagnostics
    module->is_parsed   = true;
    module->is_analyzed = false;

    // resize hash table if load factor exceeds 0.75
    if (manager->count + 1 > manager->capacity * 3 / 4)
    {
        module_manager_resize(manager);
    }

    // add to hash table
    unsigned int index      = hash_module_name(canonical, manager->capacity);
    module->next            = manager->modules[index];
    manager->modules[index] = module;
    manager->count++;

    // clean up
    parser_dnit(&parser);
    lexer_dnit(&lexer);
    // don't free source - it's now cached in module
    free(canonical);

    return module;
}

void module_init(Module *module, const char *name, const char *file_path)
{
    module->name          = strdup(name);
    module->file_path     = strdup(file_path);
    module->object_path   = NULL;
    module->source        = NULL;
    module->ast           = NULL;
    module->symbols       = NULL;
    module->is_parsed     = false;
    module->is_analyzed   = false;
    module->is_compiled   = false;
    module->needs_linking = false;
    module->next          = NULL;
}

void module_dnit(Module *module)
{
    free(module->name);
    free(module->file_path);
    free(module->object_path);
    free(module->source);
    if (module->ast)
    {
        ast_node_dnit(module->ast);
        free(module->ast);
    }
    if (module->symbols)
    {
        symbol_table_dnit(module->symbols);
        free(module->symbols);
    }
}

Module *module_manager_load_module(ModuleManager *manager, const char *module_path)
{
    return module_manager_load_module_internal(manager, module_path, ".");
}

// helper for circular dependency detection with visited tracking
static bool module_has_circular_dependency_internal(ModuleManager *manager, Module *module, const char *target, char **visited, int *visited_count, int visited_capacity)
{
    if (!module || !module->ast)
    {
        return false;
    }

    if (strcmp(module->name, target) == 0)
    {
        return true;
    }

    // check if already visited (avoid redundant work)
    for (int i = 0; i < *visited_count; i++)
    {
        if (strcmp(visited[i], module->name) == 0)
            return false;
    }

    // mark as visited
    if (*visited_count < visited_capacity)
    {
        visited[(*visited_count)++] = (char *)module->name;
    }

    // check all dependencies
    for (int i = 0; i < module->ast->program.stmts->count; i++)
    {
        AstNode *stmt = module->ast->program.stmts->items[i];
        if (stmt->kind == AST_STMT_USE)
        {
            Module *dep = module_manager_find_module(manager, stmt->use_stmt.module_path);
            if (dep && module_has_circular_dependency_internal(manager, dep, target, visited, visited_count, visited_capacity))
            {
                return true;
            }
        }
    }

    return false;
}

bool module_has_circular_dependency(ModuleManager *manager, Module *module, const char *target)
{
    char *visited[256];
    int   visited_count = 0;
    return module_has_circular_dependency_internal(manager, module, target, visited, &visited_count, 256);
}

// helper declarations
static bool compile_module_to_object(ModuleManager *manager, Module *module, const char *output_dir, int opt_level, bool no_pie, bool debug_info, bool emit_asm, bool emit_ir, bool emit_ast, SpecializationCache *spec_cache);

char *module_make_object_path(const char *output_dir, const char *module_name)
{
    if (!output_dir || !module_name)
        return NULL;

    const char *name = module_name;
    if (strncmp(name, "dep.", 4) == 0)
        name += 4; // strip dep prefix

    size_t len = strlen(name);
    char  *rel = malloc(len + 1);
    if (!rel)
        return NULL;
    for (size_t i = 0; i < len; i++)
        rel[i] = (name[i] == '.') ? '/' : name[i];
    rel[len] = '\0';

    size_t dir_len  = strlen(output_dir);
    size_t path_len = dir_len + 1 + strlen(rel) + 3;
    char  *path     = malloc(path_len);
    if (!path)
    {
        free(rel);
        return NULL;
    }
    snprintf(path, path_len, "%s/%s.o", output_dir, rel);

    // ensure parent directories exist
    char *last_slash = strrchr(path, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        fs_ensure_dir_recursive(path);
        *last_slash = '/';
    }

    free(rel);
    return path;
}

static char *module_make_artifact_path(const char *output_dir, const char *module_name, const char *extension)
{
    if (!output_dir || !module_name || !extension)
        return NULL;

    const char *name = module_name;
    if (strncmp(name, "dep.", 4) == 0)
        name += 4; // strip dep prefix

    size_t len = strlen(name);
    char  *rel = malloc(len + 1);
    if (!rel)
        return NULL;
    for (size_t i = 0; i < len; i++)
        rel[i] = (name[i] == '.') ? '/' : name[i];
    rel[len] = '\0';

    size_t dir_len  = strlen(output_dir);
    size_t ext_len  = strlen(extension);
    size_t path_len = dir_len + 1 + strlen(rel) + ext_len + 1;
    char  *path     = malloc(path_len);
    if (!path)
    {
        free(rel);
        return NULL;
    }
    snprintf(path, path_len, "%s/%s%s", output_dir, rel, extension);

    free(rel);
    return path;
}

bool module_manager_compile_dependencies(ModuleManager *manager, const char *output_dir, int opt_level, bool no_pie, bool debug_info, bool emit_asm, bool emit_ir, bool emit_ast, SpecializationCache *spec_cache)
{
    if (!manager)
        return false;

    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            if (!module->needs_linking)
            {
                module = module->next;
                continue;
            }

            if (!module->ast)
            {
                module_error_list_add(&manager->errors, module->name, module->file_path ? module->file_path : "<unknown>", "module missing AST");
                manager->had_error = true;
                return false;
            }

            if (!module->is_analyzed)
            {
                module_error_list_add(&manager->errors, module->name, module->file_path ? module->file_path : "<unknown>", "module has not been analyzed");
                manager->had_error = true;
                return false;
            }

            if (module->is_compiled && module->object_path)
            {
                module = module->next;
                continue;
            }

            // no info output

            if (!compile_module_to_object(manager, module, output_dir, opt_level, no_pie, debug_info, emit_asm, emit_ir, emit_ast, spec_cache))
            {
                return false;
            }

            module->is_compiled = true;
            module              = module->next;
        }
    }

    return true;
}

bool module_manager_get_link_objects(ModuleManager *manager, char ***object_files, int *count)
{
    // count modules that need linking
    int link_count = 0;
    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            if (module->needs_linking && module->object_path)
            {
                link_count++;
            }
            module = module->next;
        }
    }

    if (link_count == 0)
    {
        *object_files = NULL;
        *count        = 0;
        return true;
    }

    // allocate array for object file paths
    char **objects = malloc(link_count * sizeof(char *));
    int    idx     = 0;

    // collect object file paths
    for (int i = 0; i < manager->capacity; i++)
    {
        Module *module = manager->modules[i];
        while (module)
        {
            if (module->needs_linking && module->object_path)
            {
                objects[idx++] = strdup(module->object_path);
            }
            module = module->next;
        }
    }

    *object_files = objects;
    *count        = link_count;
    return true;
}

static bool compile_module_to_object(ModuleManager *manager, Module *module, const char *output_dir, int opt_level, bool no_pie, bool debug_info, bool emit_asm, bool emit_ir, bool emit_ast, SpecializationCache *spec_cache)
{
    if (!module || !module->ast)
        return false;

    char *object_path = module_make_object_path(output_dir, module->name);
    if (!object_path)
    {
        module_error_list_add(&manager->errors, module->name, module->file_path ? module->file_path : "<unknown>", "failed to allocate object path");
        manager->had_error = true;
        return false;
    }

    if (module->object_path)
    {
        free(module->object_path);
        module->object_path = NULL;
    }
    module->object_path = object_path;

    CodegenContext ctx;
    codegen_context_init(&ctx, module->name, no_pie);
    ctx.opt_level    = opt_level;
    ctx.debug_info   = debug_info;
    ctx.source_file  = module->file_path;
    ctx.source_lexer = NULL;
    ctx.spec_cache   = spec_cache; // pass cache to codegen for generating specialized functions

    Lexer *debug_lexer_ptr = NULL;
    Lexer  debug_lexer;
    if (debug_info && module->source && module->file_path && module->file_path[0] != '<')
    {
        // use cached source instead of reading file again
        lexer_init(&debug_lexer, module->source);
        debug_lexer_ptr  = &debug_lexer;
        ctx.source_lexer = debug_lexer_ptr;
    }

    bool success = codegen_generate(&ctx, module->ast, module->symbols);

    if (success)
    {
        // emit AST if requested
        if (emit_ast)
        {
            char *parent = fs_dirname(output_dir);
            if (parent)
            {
                char ast_dir[1024];
                snprintf(ast_dir, sizeof(ast_dir), "%s/ast", parent);
                char *ast_path = module_make_artifact_path(ast_dir, module->name, ".ast");
                if (ast_path)
                {
                    char *ast_parent = fs_dirname(ast_path);
                    if (ast_parent)
                    {
                        fs_ensure_dir_recursive(ast_parent);
                        free(ast_parent);
                    }
                    ast_emit(module->ast, ast_path);
                    free(ast_path);
                }
                free(parent);
            }
        }

        // emit IR if requested
        if (emit_ir)
        {
            char *parent = fs_dirname(output_dir);
            if (parent)
            {
                char ir_dir[1024];
                snprintf(ir_dir, sizeof(ir_dir), "%s/ir", parent);
                char *ir_path = module_make_artifact_path(ir_dir, module->name, ".ll");
                if (ir_path)
                {
                    char *ir_parent = fs_dirname(ir_path);
                    if (ir_parent)
                    {
                        fs_ensure_dir_recursive(ir_parent);
                        free(ir_parent);
                    }
                    codegen_emit_llvm_ir(&ctx, ir_path);
                    free(ir_path);
                }
                free(parent);
            }
        }

        // emit assembly if requested
        if (emit_asm)
        {
            char *parent = fs_dirname(output_dir);
            if (parent)
            {
                char asm_dir[1024];
                snprintf(asm_dir, sizeof(asm_dir), "%s/asm", parent);
                char *asm_path = module_make_artifact_path(asm_dir, module->name, ".s");
                if (asm_path)
                {
                    char *asm_parent = fs_dirname(asm_path);
                    if (asm_parent)
                    {
                        fs_ensure_dir_recursive(asm_parent);
                        free(asm_parent);
                    }
                    codegen_emit_assembly(&ctx, asm_path);
                    free(asm_path);
                }
                free(parent);
            }
        }

        success = codegen_emit_object(&ctx, module->object_path);
        if (!success)
        {
            module_error_list_add(&manager->errors, module->name, module->file_path ? module->file_path : "<unknown>", "failed to emit object file");
            manager->had_error = true;
        }
    }
    else
    {
        codegen_print_errors(&ctx);
        module_error_list_add(&manager->errors, module->name, module->file_path ? module->file_path : "<unknown>", "code generation failed");
        manager->had_error = true;
    }

    if (debug_lexer_ptr)
    {
        lexer_dnit(debug_lexer_ptr);
        ctx.source_lexer = NULL;
    }
    // no need to free debug_source - using cached module source

    codegen_context_dnit(&ctx);
    return success;
}
