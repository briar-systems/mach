#include "module.h"
#include "codegen.h"
#include "config.h"
#include "filesystem.h"
#include "ioutil.h"
#include "lexer.h"
#include "preprocessor.h"
#include "symbol.h"
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// forward declarations
static char       *generate_target_module_source(ModuleManager *manager);
static void        module_manager_update_target_info(ModuleManager *manager);
static char       *append_os_suffix(const char *path, const char *os_name);
static const char *normalize_os_name(const char *os_part);
static bool        path_has_platform_suffix(const char *path, const char *os_name);
static void        split_triple(const char *triple, char **arch_out, char **os_out);

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
    manager->capacity      = 32;
    manager->count         = 0;
    manager->modules       = calloc(manager->capacity, sizeof(Module *));
    manager->search_paths  = NULL;
    manager->search_count  = 0;
    manager->alias_names   = NULL;
    manager->alias_paths   = NULL;
    manager->alias_count   = 0;
    manager->config        = NULL;
    manager->project_dir   = NULL;
    manager->target_triple = NULL;
    manager->target_os     = NULL;
    manager->target_arch   = NULL;

    manager->cached_constants       = NULL;
    manager->cached_constants_count = 0;

    module_error_list_init(&manager->errors);
    manager->had_error = false;

    module_manager_update_target_info(manager);
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

    free(manager->target_triple);
    free(manager->target_os);
    free(manager->target_arch);

    free(manager->cached_constants);

    // clean up errors
    module_error_list_dnit(&manager->errors);
}

static const char *normalize_os_name(const char *os_part)
{
    if (!os_part)
        return "unknown";
    if (strstr(os_part, "linux"))
        return "linux";
    if (strstr(os_part, "darwin") || strstr(os_part, "apple"))
        return "darwin";
    if (strstr(os_part, "windows") || strstr(os_part, "mingw") || strstr(os_part, "win32"))
        return "windows";
    if (strstr(os_part, "freebsd"))
        return "freebsd";
    if (strstr(os_part, "netbsd"))
        return "netbsd";
    if (strstr(os_part, "openbsd"))
        return "openbsd";
    if (strstr(os_part, "dragonfly"))
        return "dragonfly";
    if (strstr(os_part, "wasm"))
        return "wasm";
    return "unknown";
}

static bool path_has_platform_suffix(const char *path, const char *os_name)
{
    if (!path || !os_name || !*os_name)
        return false;

    const char *ext = strrchr(path, '.');
    if (!ext || ext == path)
        return false;

    size_t os_len = strlen(os_name);
    if ((size_t)(ext - path) < os_len + 1)
        return false;

    const char *suffix = ext - (os_len + 1);
    if (*suffix != '.')
        return false;

    return strncmp(suffix + 1, os_name, os_len) == 0;
}

static char *append_os_suffix(const char *path, const char *os_name)
{
    if (!path || !os_name || !*os_name)
        return NULL;

    const char *ext = strrchr(path, '.');
    if (!ext)
        return NULL;

    size_t base_len = (size_t)(ext - path);
    size_t os_len   = strlen(os_name);
    size_t ext_len  = strlen(ext);

    char *out = malloc(base_len + 1 + os_len + ext_len + 1);
    if (!out)
        return NULL;

    memcpy(out, path, base_len);
    out[base_len] = '.';
    memcpy(out + base_len + 1, os_name, os_len);
    memcpy(out + base_len + 1 + os_len, ext, ext_len + 1);

    return out;
}

// helper to build path from base, module name parts, and tail
static char *build_module_path(const char *base, const char *head, size_t head_len, const char *tail, const char *platform)
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

    // check if base path exists
    if (fs_file_exists(path))
        return path;

    // try platform-specific variant
    if (platform)
    {
        char *plat_path = append_os_suffix(path, platform);
        if (plat_path && fs_file_exists(plat_path))
        {
            free(path);
            return plat_path;
        }
        free(plat_path);
    }

    free(path);
    return NULL;
}

static void module_manager_update_target_info(ModuleManager *manager)
{
    if (!manager)
        return;

    const char *triple       = NULL;
    char       *owned_triple = NULL;

    if (manager->config)
    {
        ProjectConfig *pc = (ProjectConfig *)manager->config;
        TargetConfig  *tc = config_get_default_target(pc);
        if (tc && tc->target_triple)
            triple = tc->target_triple;
    }

    if (!triple)
    {
        owned_triple = LLVMGetDefaultTargetTriple();
        triple       = owned_triple;
    }

    if (manager->target_triple)
    {
        free(manager->target_triple);
        manager->target_triple = NULL;
    }
    if (triple)
        manager->target_triple = strdup(triple);

    if (manager->target_os)
    {
        free(manager->target_os);
        manager->target_os = NULL;
    }
    if (manager->target_arch)
    {
        free(manager->target_arch);
        manager->target_arch = NULL;
    }

    if (triple)
    {
        char *arch_part = NULL;
        char *os_part   = NULL;
        split_triple(triple, &arch_part, &os_part);
        const char *norm = normalize_os_name(os_part);
        if (norm)
            manager->target_os = strdup(norm);
        if (arch_part)
            manager->target_arch = strdup(arch_part);
        free(arch_part);
        free(os_part);
    }

    if (owned_triple)
        LLVMDisposeMessage(owned_triple);

    // rebuild cached constants
    free(manager->cached_constants);
    manager->cached_constants       = malloc(32 * sizeof(PreprocessorConstant));
    manager->cached_constants_count = 0;

    if (manager->cached_constants)
    {
        manager->cached_constants_count = module_manager_collect_constants(manager, manager->cached_constants, 32);
    }
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
    module_manager_update_target_info(manager);
}

char *module_path_to_file_path(ModuleManager *manager, const char *module_fqn)
{
    if (!module_fqn)
        return NULL;

    const char *platform = (manager->target_os && strcmp(manager->target_os, "unknown") != 0) ? manager->target_os : NULL;

    // prefer config resolution if available, but still honor platform variants
    if (manager->config && manager->project_dir)
    {
        char *resolved = config_resolve_module_fqn((ProjectConfig *)manager->config, manager->project_dir, module_fqn);
        if (resolved)
        {
            if (fs_file_exists(resolved))
            {
                return resolved;
            }

            if (platform)
            {
                char *plat_path = append_os_suffix(resolved, platform);
                if (plat_path && fs_file_exists(plat_path))
                {
                    free(resolved);
                    return plat_path;
                }
                free(plat_path);
            }

            return resolved;
        }
    }

    const char *dot      = strchr(module_fqn, '.');
    size_t      head_len = dot ? (size_t)(dot - module_fqn) : strlen(module_fqn);
    const char *tail     = dot ? dot + 1 : NULL;

    // aliases: name -> base dir
    for (int i = 0; i < manager->alias_count; i++)
    {
        if (strlen(manager->alias_names[i]) == head_len && strncmp(manager->alias_names[i], module_fqn, head_len) == 0)
        {
            char *path = build_module_path(manager->alias_paths[i], NULL, 0, tail, platform);
            if (path)
                return path;
        }
    }

    // search paths: base/name/(tail or main).mach
    for (int si = 0; si < manager->search_count; si++)
    {
        char *path = build_module_path(manager->search_paths[si], module_fqn, head_len, tail, platform);
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

    // special synthetic builtin module: "target"
    bool  is_target_builtin = (strcmp(canonical, "target") == 0);
    char *file_path         = NULL;
    char *source            = NULL;

    if (is_target_builtin)
    {
        file_path = strdup("<builtin:target>");
        source    = generate_target_module_source(manager);
        if (!source)
        {
            module_error_list_add(&manager->errors, canonical, file_path, "failed to generate target module");
            manager->had_error = true;
            free(file_path);
            free(canonical);
            return NULL;
        }
    }
    else
    {
        // find file path via canonical FQN
        file_path = module_path_to_file_path(manager, canonical);
        if (!file_path)
        {
            char *guess = module_guess_file_path(manager, canonical);
            module_error_list_add(&manager->errors, canonical, guess ? guess : "<unknown>", "Could not find module file");
            if (guess)
                free(guess);
            manager->had_error = true;
            free(canonical);
            return NULL;
        }

        // no info output

        // read and parse module
        source = read_file(file_path);
        if (!source)
        {
            module_error_list_add(&manager->errors, canonical, file_path, "Could not read module file");
            manager->had_error = true;
            free(file_path);
            free(canonical);
            return NULL;
        }

        const char *platform = (manager->target_os && strcmp(manager->target_os, "unknown") != 0) ? manager->target_os : NULL;

        if (platform && !path_has_platform_suffix(file_path, platform))
        {
            char *platform_path = append_os_suffix(file_path, platform);
            if (platform_path && fs_file_exists(platform_path))
            {
                char *platform_source = read_file(platform_path);
                if (!platform_source)
                {
                    module_error_list_add(&manager->errors, canonical, platform_path, "Could not read platform-specific module file");
                    manager->had_error = true;
                    free(platform_path);
                    free(source);
                    free(file_path);
                    free(canonical);
                    return NULL;
                }

                size_t base_len = strlen(source);
                size_t plat_len = strlen(platform_source);
                char  *combined = malloc(base_len + plat_len + 2);
                if (!combined)
                {
                    module_error_list_add(&manager->errors, canonical, platform_path, "Out of memory while combining platform module");
                    manager->had_error = true;
                    free(platform_source);
                    free(platform_path);
                    free(source);
                    free(file_path);
                    free(canonical);
                    return NULL;
                }

                memcpy(combined, source, base_len);
                combined[base_len] = '\n';
                memcpy(combined + base_len + 1, platform_source, plat_len + 1);

                free(source);
                free(platform_source);
                source = combined;
            }
            free(platform_path);
        }

        PreprocessorConstant constants[32];
        size_t               constant_count = module_manager_collect_constants(manager, constants, sizeof(constants) / sizeof(constants[0]));

        PreprocessorOutput pp_output;
        preprocessor_output_init(&pp_output);

        if (!preprocessor_run(source, constants, constant_count, &pp_output))
        {
            const char *msg = pp_output.message ? pp_output.message : "conditional directive failure";
            char        error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "#! directive error on line %d: %s", pp_output.line, msg);
            module_error_list_add(&manager->errors, canonical, file_path, error_msg);
            manager->had_error = true;
            preprocessor_output_dnit(&pp_output);
            free(source);
            free(file_path);
            free(canonical);
            return NULL;
        }

        free(source);
        source           = pp_output.source;
        pp_output.source = NULL;
        preprocessor_output_dnit(&pp_output);
    }

    if (is_target_builtin)
    {
        // no info output
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
    module->ast       = ast;
    module->source    = source; // cache source for debug info and diagnostics
    module->is_parsed = true;
    if (is_target_builtin)
        module->needs_linking = true; // ensure builtin target object is emitted
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

// helper: map substring presence (case sensitive) in triple to numeric constants
static unsigned int map_os_id(const char *os_part)
{
    if (!os_part)
        return 255u;
    if (strstr(os_part, "linux"))
        return 1u;
    if (strstr(os_part, "darwin") || strstr(os_part, "apple"))
        return 2u;
    if (strstr(os_part, "windows") || strstr(os_part, "mingw") || strstr(os_part, "win32"))
        return 3u;
    if (strstr(os_part, "freebsd"))
        return 4u;
    if (strstr(os_part, "netbsd"))
        return 5u;
    if (strstr(os_part, "openbsd"))
        return 6u;
    if (strstr(os_part, "dragonfly"))
        return 7u;
    if (strstr(os_part, "wasm"))
        return 8u;
    return 255u; // unknown
}

static unsigned int map_arch_id(const char *arch_part)
{
    if (!arch_part)
        return 255u;
    if (strncmp(arch_part, "x86_64", 6) == 0)
        return 1u;
    if (strncmp(arch_part, "aarch64", 7) == 0)
        return 2u;
    if (strncmp(arch_part, "riscv64", 7) == 0)
        return 3u;
    if (strncmp(arch_part, "wasm32", 6) == 0)
        return 4u;
    if (strncmp(arch_part, "wasm64", 6) == 0)
        return 5u;
    return 255u; // unknown
}

size_t module_manager_collect_constants(ModuleManager *manager, PreprocessorConstant *out, size_t max_count)
{
    if (!out || max_count == 0)
    {
        return 0;
    }

    // use cached constants if available
    if (manager && manager->cached_constants && manager->cached_constants_count > 0)
    {
        size_t copy_count = manager->cached_constants_count < max_count ? manager->cached_constants_count : max_count;
        memcpy(out, manager->cached_constants, copy_count * sizeof(PreprocessorConstant));
        return copy_count;
    }

    // fallback: build constants from scratch
    size_t count = 0;

    const char *triple    = manager ? manager->target_triple : NULL;
    char       *arch_part = NULL;
    char       *os_part   = NULL;

    if (triple && *triple)
    {
        split_triple(triple, &arch_part, &os_part);
    }

    unsigned int os_id   = map_os_id(os_part);
    unsigned int arch_id = map_arch_id(arch_part);

    unsigned int ptr_width = (unsigned int)(sizeof(void *) * 8);
    union
    {
        uint16_t      v;
        unsigned char b[2];
    } u;
    u.v                 = 0x0102;
    unsigned int endian = (u.b[0] == 0x02) ? 0u : 1u;

    unsigned int debug_flag = 0u;

#define ADD_CONST(NAME, VALUE)                     \
    do                                             \
    {                                              \
        if (count < max_count)                     \
        {                                          \
            out[count].name  = (NAME);             \
            out[count].value = (long long)(VALUE); \
            count++;                               \
        }                                          \
    } while (0)

    ADD_CONST("OS_LINUX", 1u);
    ADD_CONST("OS_DARWIN", 2u);
    ADD_CONST("OS_WINDOWS", 3u);
    ADD_CONST("OS_FREEBSD", 4u);
    ADD_CONST("OS_NETBSD", 5u);
    ADD_CONST("OS_OPENBSD", 6u);
    ADD_CONST("OS_DRAGONFLY", 7u);
    ADD_CONST("OS_WASM", 8u);
    ADD_CONST("OS_UNKNOWN", 255u);

    ADD_CONST("ARCH_X86_64", 1u);
    ADD_CONST("ARCH_AARCH64", 2u);
    ADD_CONST("ARCH_RISCV64", 3u);
    ADD_CONST("ARCH_WASM32", 4u);
    ADD_CONST("ARCH_WASM64", 5u);
    ADD_CONST("ARCH_UNKNOWN", 255u);

    ADD_CONST("OS", os_id);
    ADD_CONST("ARCH", arch_id);
    ADD_CONST("PTR_WIDTH", ptr_width);
    ADD_CONST("ENDIAN", endian);
    ADD_CONST("DEBUG", debug_flag);

#undef ADD_CONST

    free(arch_part);
    free(os_part);

    return count;
}

static char *dup_fragment(const char *start, size_t len)
{
    char *s = malloc(len + 1);
    if (!s)
        return NULL;
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

static void split_triple(const char *triple, char **arch_out, char **os_out)
{
    *arch_out = NULL;
    *os_out   = NULL;
    if (!triple)
        return;
    const char *first = strchr(triple, '-');
    if (!first)
    {
        *arch_out = strdup(triple);
        return;
    }
    *arch_out        = dup_fragment(triple, (size_t)(first - triple));
    const char *rest = first + 1; // vendor or os
    // find second dash to locate os portion
    const char *second = strchr(rest, '-');
    if (!second)
    {
        *os_out = strdup(rest);
        return;
    }
    const char *third = strchr(second + 1, '-');
    if (!third)
    {
        // assume second segment is vendor, third (second->end) is os
        *os_out = strdup(second + 1);
    }
    else
    {
        // treat segment between second and third as os
        *os_out = dup_fragment(second + 1, (size_t)(third - (second + 1)));
    }
}

static char *generate_target_module_source(ModuleManager *manager)
{
    const char *triple = NULL;
    if (manager && manager->config)
    {
        ProjectConfig *pc = (ProjectConfig *)manager->config;
        TargetConfig  *tc = config_get_default_target(pc);
        if (tc && tc->target_triple)
            triple = tc->target_triple;
    }
    if (!triple)
    {
        triple = LLVMGetDefaultTargetTriple();
    }

    char *arch_part = NULL;
    char *os_part   = NULL;
    split_triple(triple, &arch_part, &os_part);

    unsigned int os_id   = map_os_id(os_part);
    unsigned int arch_id = map_arch_id(arch_part);

    // pointer width & endianness
    unsigned int ptr_width = (unsigned int)(sizeof(void *) * 8);
    union
    {
        uint16_t      v;
        unsigned char b[2];
    } u;
    u.v                 = 0x0102;
    unsigned int endian = (u.b[0] == 0x02) ? 0u : 1u; // 0 little, 1 big

    // debug flag heuristic: treat non-optimized builds as debug (opt_level 0 or 1)
    unsigned int debug_flag = 0u;
    // we cannot easily read opt level here without invasive changes; leave 0 for now

    // build source buffer
    char buffer[2048];
    snprintf(buffer,
             sizeof(buffer),
             "pub val OS_LINUX: u32 = 1;\n"
             "pub val OS_DARWIN: u32 = 2;\n"
             "pub val OS_WINDOWS: u32 = 3;\n"
             "pub val OS_FREEBSD: u32 = 4;\n"
             "pub val OS_NETBSD: u32 = 5;\n"
             "pub val OS_OPENBSD: u32 = 6;\n"
             "pub val OS_DRAGONFLY: u32 = 7;\n"
             "pub val OS_WASM: u32 = 8;\n"
             "pub val OS_UNKNOWN: u32 = 255;\n"
             "pub val ARCH_X86_64: u32 = 1;\n"
             "pub val ARCH_AARCH64: u32 = 2;\n"
             "pub val ARCH_RISCV64: u32 = 3;\n"
             "pub val ARCH_WASM32: u32 = 4;\n"
             "pub val ARCH_WASM64: u32 = 5;\n"
             "pub val ARCH_UNKNOWN: u32 = 255;\n"
             "pub val OS: u32 = %u;\n"
             "pub val ARCH: u32 = %u;\n"
             "pub val PTR_WIDTH: u8 = %u;\n"
             "pub val ENDIAN: u8 = %u;\n"
             "pub val DEBUG: u8 = %u;\n"
             "pub val OS_NAME: []u8 = \"%s\";\n"
             "pub val ARCH_NAME: []u8 = \"%s\";\n",
             os_id,
             arch_id,
             ptr_width,
             endian,
             debug_flag,
             os_part ? os_part : "unknown",
             arch_part ? arch_part : "unknown");

    char *out = strdup(buffer);
    free(arch_part);
    free(os_part);
    // if we allocated triple via LLVM fallback (char*) we leak if not freed; acceptable minimal impact
    return out;
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
static bool compile_module_to_object(ModuleManager *manager, Module *module, const char *output_dir, int opt_level, bool no_pie, bool debug_info, SpecializationCache *spec_cache);

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

bool module_manager_compile_dependencies(ModuleManager *manager, const char *output_dir, int opt_level, bool no_pie, bool debug_info, SpecializationCache *spec_cache)
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

            if (!compile_module_to_object(manager, module, output_dir, opt_level, no_pie, debug_info, spec_cache))
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

static bool compile_module_to_object(ModuleManager *manager, Module *module, const char *output_dir, int opt_level, bool no_pie, bool debug_info, SpecializationCache *spec_cache)
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
