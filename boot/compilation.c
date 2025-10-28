#include "compilation.h"
#include "filesystem.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void string_vec_push(StringVec *v, const char *s)
{
    if (v->count == v->cap)
    {
        int   n  = v->cap ? v->cap * 2 : 4;
        void *nn = realloc(v->items, n * sizeof(char *));
        if (!nn)
        {
            fprintf(stderr, "error: out of memory\n");
            return;
        }
        v->items = nn;
        v->cap   = n;
    }
    v->items[v->count++] = strdup(s);
}

static void string_vec_dnit(StringVec *v)
{
    for (int i = 0; i < v->count; i++)
        free(v->items[i]);
    free(v->items);
    v->items = NULL;
    v->count = 0;
    v->cap   = 0;
}

static void alias_vec_push(AliasVec *v, const char *name, const char *dir)
{
    if (v->count == v->cap)
    {
        int    ncap = v->cap ? v->cap * 2 : 4;
        char **nn   = realloc(v->names, ncap * sizeof(char *));
        char **nd   = realloc(v->dirs, ncap * sizeof(char *));
        if (!nn || !nd)
        {
            fprintf(stderr, "error: out of memory\n");
            return;
        }
        v->names = nn;
        v->dirs  = nd;
        v->cap   = ncap;
    }
    v->names[v->count] = strdup(name);
    v->dirs[v->count]  = strdup(dir);
    v->count++;
}

static void alias_vec_dnit(AliasVec *v)
{
    for (int i = 0; i < v->count; i++)
    {
        free(v->names[i]);
        free(v->dirs[i]);
    }
    free(v->names);
    free(v->dirs);
    v->names = NULL;
    v->dirs  = NULL;
    v->count = 0;
    v->cap   = 0;
}

// build hierarchical path from module name: "std.io.console" + "obj" + ".o" -> "obj/std/io/console.o"
static char *build_module_artifact_path(const char *base_dir, const char *module_name, const char *extension)
{
    if (!module_name || !extension)
        return NULL;

    size_t base_len   = base_dir ? strlen(base_dir) : 0;
    size_t module_len = strlen(module_name);
    size_t ext_len    = strlen(extension);
    size_t total_len  = base_len + (base_len ? 1 : 0) + module_len + ext_len + 1;

    char *path = malloc(total_len);
    if (!path)
        return NULL;

    size_t pos = 0;
    if (base_len)
    {
        memcpy(path, base_dir, base_len);
        pos = base_len;
        if (base_dir[base_len - 1] != '/')
            path[pos++] = '/';
    }

    // convert module name to path: dots become slashes
    for (size_t i = 0; i < module_len; i++)
    {
        char c      = module_name[i];
        path[pos++] = (c == '.') ? '/' : c;
    }

    memcpy(path + pos, extension, ext_len);
    path[pos + ext_len] = '\0';

    return path;
}

void build_options_init(BuildOptions *opts)
{
    memset(opts, 0, sizeof(BuildOptions));
    opts->opt_level  = 2;
    opts->link_exe   = 1;
    opts->debug_info = 1;
}

void build_options_dnit(BuildOptions *opts)
{
    string_vec_dnit(&opts->include_paths);
    string_vec_dnit(&opts->link_objects);
    alias_vec_dnit(&opts->aliases);
}

void build_options_add_include(BuildOptions *opts, const char *path)
{
    string_vec_push(&opts->include_paths, path);
}

void build_options_add_link_object(BuildOptions *opts, const char *obj)
{
    string_vec_push(&opts->link_objects, obj);
}

void build_options_add_alias(BuildOptions *opts, const char *name, const char *dir)
{
    alias_vec_push(&opts->aliases, name, dir);
}

char *derive_module_name(const char *filename, const AliasVec *aliases)
{
    if (!filename || !aliases || aliases->count == 0)
        return NULL;

    char *abs_file = realpath(filename, NULL);
    if (!abs_file)
        return NULL;

    char *module_name = NULL;
    for (int i = 0; i < aliases->count && !module_name; i++)
    {
        char *abs_dir = realpath(aliases->dirs[i], NULL);
        if (!abs_dir)
            continue;

        size_t dir_len  = strlen(abs_dir);
        size_t file_len = strlen(abs_file);
        if (file_len >= dir_len && strncmp(abs_file, abs_dir, dir_len) == 0 && (abs_file[dir_len] == '/' || abs_file[dir_len] == '\\' || abs_file[dir_len] == '\0'))
        {
            const char *rel = abs_file + dir_len;
            if (*rel == '/' || *rel == '\\')
                rel++;

            size_t rel_len = strlen(rel);
            if (rel_len >= 5 && strcmp(rel + rel_len - 5, ".mach") == 0)
                rel_len -= 5;

            size_t alias_len = strlen(aliases->names[i]);
            size_t total_len = alias_len + (rel_len > 0 ? (1 + rel_len) : 0);
            module_name      = malloc(total_len + 1);
            if (module_name)
            {
                memcpy(module_name, aliases->names[i], alias_len);
                size_t pos = alias_len;
                if (rel_len > 0)
                {
                    module_name[pos++] = '.';
                    for (size_t j = 0; j < rel_len; j++)
                    {
                        char c = rel[j];
                        if (c == '/' || c == '\\')
                            c = '.';
                        module_name[pos++] = c;
                    }
                }
                module_name[pos] = '\0';
            }
        }

        free(abs_dir);
    }

    free(abs_file);
    return module_name;
}

bool compilation_context_init(CompilationContext *ctx, BuildOptions *opts)
{
    memset(ctx, 0, sizeof(CompilationContext));
    ctx->options = opts;
    ctx->driver  = semantic_driver_create();
    if (!ctx->driver)
    {
        fprintf(stderr, "error: failed to create semantic driver\n");
        return false;
    }
    return true;
}

void compilation_context_dnit(CompilationContext *ctx)
{
    if (ctx->dep_objects)
    {
        for (int i = 0; i < ctx->dep_count; i++)
            free(ctx->dep_objects[i]);
        free(ctx->dep_objects);
    }

    if (ctx->codegen_initialized)
        codegen_context_dnit(&ctx->codegen);
    if (ctx->parser_initialized)
        parser_dnit(&ctx->parser);
    if (ctx->lexer_initialized)
        lexer_dnit(&ctx->lexer);

    if (ctx->config)
    {
        config_dnit(ctx->config);
        free(ctx->config);
    }
    if (ctx->driver)
        semantic_driver_destroy(ctx->driver);
    if (ctx->ast)
    {
        ast_node_dnit(ctx->ast);
        free(ctx->ast);
    }
    free(ctx->module_name);
    free(ctx->project_root);
    free(ctx->source);
    memset(ctx, 0, sizeof(CompilationContext));
}

bool compilation_load_and_preprocess(CompilationContext *ctx)
{
    ctx->project_root = fs_find_project_root(ctx->options->input_file);
    if (!ctx->project_root)
    {
        fprintf(stderr, "error: could not find project root\n");
        return false;
    }

    ctx->config = config_load_from_dir(ctx->project_root);
    if (ctx->config)
    {
        module_manager_set_config(&ctx->driver->module_manager, ctx->config, ctx->project_root);
    }

    for (int i = 0; i < ctx->options->include_paths.count; i++)
        module_manager_add_search_path(&ctx->driver->module_manager, ctx->options->include_paths.items[i]);

    for (int i = 0; i < ctx->options->aliases.count; i++)
        module_manager_add_alias(&ctx->driver->module_manager, ctx->options->aliases.names[i], ctx->options->aliases.dirs[i]);

    ctx->source = fs_read_file(ctx->options->input_file);
    if (!ctx->source)
    {
        fprintf(stderr, "error: could not read '%s'\n", ctx->options->input_file);
        return false;
    }

    return true;
}

bool compilation_parse(CompilationContext *ctx)
{
    lexer_init(&ctx->lexer, ctx->source);
    ctx->lexer_initialized = true;
    parser_init(&ctx->parser, &ctx->lexer);
    ctx->parser_initialized = true;
    ctx->ast                = parser_parse_program(&ctx->parser);

    if (ctx->parser.had_error)
    {
        fprintf(stderr, "parsing failed with %d error(s):\n", ctx->parser.errors.count);
        parser_error_list_print(&ctx->parser.errors, &ctx->lexer, ctx->options->input_file);
        return false;
    }

    return true;
}

bool compilation_analyze(CompilationContext *ctx)
{
    ctx->module_name                 = derive_module_name(ctx->options->input_file, &ctx->options->aliases);
    const char *semantic_module_name = ctx->module_name ? ctx->module_name : ctx->options->input_file;

    if (!semantic_driver_analyze(ctx->driver, ctx->ast, semantic_module_name, ctx->options->input_file))
    {
        if (ctx->driver->module_manager.had_error)
        {
            fprintf(stderr, "module loading failed with %d error(s):\n", ctx->driver->module_manager.errors.count);
            module_error_list_print(&ctx->driver->module_manager.errors);
        }
        return false;
    }

    return true;
}

bool compilation_codegen(CompilationContext *ctx)
{
    const char *semantic_module_name = ctx->module_name ? ctx->module_name : ctx->options->input_file;

    codegen_context_init(&ctx->codegen, semantic_module_name, ctx->options->no_pie);
    ctx->codegen_initialized  = true;
    ctx->codegen.opt_level    = ctx->options->opt_level;
    ctx->codegen.debug_info   = ctx->options->debug_info;
    ctx->codegen.source_file  = ctx->options->input_file;
    ctx->codegen.source_lexer = &ctx->lexer;
    ctx->codegen.spec_cache   = &ctx->driver->spec_cache;

    if (!codegen_generate(&ctx->codegen, ctx->ast, &ctx->driver->symbol_table))
    {
        fprintf(stderr, "code generation failed:\n");
        codegen_print_errors(&ctx->codegen);
        return false;
    }

    return true;
}

bool compilation_emit_artifacts(CompilationContext *ctx)
{
    const char *config_target_name = ctx->options->target_name;

    // Determine emit flags (config overrides or CLI overrides)
    int emit_ast = ctx->options->emit_ast;
    int emit_ir  = ctx->options->emit_ir;
    int emit_asm = ctx->options->emit_asm;

    // For project builds with config, use config-based directory resolution
    char *auto_ast_dir = NULL;
    char *auto_ir_dir  = NULL;
    char *auto_asm_dir = NULL;
    char *auto_obj_dir = NULL;

    if (ctx->config && config_target_name && ctx->project_root)
    {
        // Use config-based directories for project builds
        if (!ctx->options->emit_ast_path && emit_ast)
        {
            auto_ast_dir = config_resolve_ast_dir(ctx->config, ctx->project_root, config_target_name);
        }
        if (!ctx->options->emit_ir_path && emit_ir)
        {
            auto_ir_dir = config_resolve_ir_dir(ctx->config, ctx->project_root, config_target_name);
        }
        if (!ctx->options->emit_asm_path && emit_asm)
        {
            auto_asm_dir = config_resolve_asm_dir(ctx->config, ctx->project_root, config_target_name);
        }
        if (!ctx->options->obj_dir)
        {
            auto_obj_dir = config_resolve_obj_dir(ctx->config, ctx->project_root, config_target_name);
        }
    }

    char *auto_ast = NULL;
    if (emit_ast)
    {
        const char *ast_path = ctx->options->emit_ast_path;
        if (!ast_path || ast_path[0] == '\0')
        {
            // Use config-based directory if available, otherwise derive from obj_dir
            const char *base_dir = auto_ast_dir;
            if (!base_dir)
            {
                base_dir     = ctx->options->obj_dir;
                char *parent = NULL;
                if (base_dir)
                {
                    parent = fs_dirname(base_dir);
                    if (parent)
                    {
                        size_t len = strlen(parent) + 5;
                        auto_ast   = malloc(len);
                        snprintf(auto_ast, len, "%s/ast", parent);
                        free(parent);
                        base_dir = auto_ast;
                    }
                }
                else
                {
                    base_dir = "ast";
                }
            }

            char *ast_full = build_module_artifact_path(base_dir, ctx->module_name, ".ast");
            if (ast_full)
            {
                free(auto_ast);
                auto_ast = ast_full;
            }
            else
            {
                // Fallback: use base filename in the configured directory
                char *base = fs_get_base_filename(ctx->options->input_file);
                if (base_dir && strlen(base_dir) > 0)
                {
                    size_t len = strlen(base_dir) + 1 + strlen(base) + 5;
                    auto_ast   = malloc(len);
                    snprintf(auto_ast, len, "%s/%s.ast", base_dir, base);
                }
                else
                {
                    size_t len = strlen(base) + 5;
                    auto_ast   = malloc(len);
                    snprintf(auto_ast, len, "%s.ast", base);
                }
                free(base);
            }
            ast_path = auto_ast;
        }

        // ensure directory exists
        char *dir = fs_dirname(ast_path);
        if (dir)
        {
            fs_ensure_dir_recursive(dir);
            free(dir);
        }

        if (!ast_emit(ctx->ast, ast_path))
        {
            fprintf(stderr, "error: failed to emit ast file '%s'\n", ast_path);
        }
        free(auto_ast);
    }

    char *auto_ir = NULL;
    if (emit_ir)
    {
        const char *ir_path = ctx->options->emit_ir_path;
        if (!ir_path || ir_path[0] == '\0')
        {
            // Use config-based directory if available, otherwise derive from obj_dir
            const char *base_dir = auto_ir_dir;
            if (!base_dir)
            {
                base_dir     = ctx->options->obj_dir;
                char *parent = NULL;
                if (base_dir)
                {
                    parent = fs_dirname(base_dir);
                    if (parent)
                    {
                        size_t len = strlen(parent) + 4;
                        auto_ir    = malloc(len);
                        snprintf(auto_ir, len, "%s/ir", parent);
                        free(parent);
                        base_dir = auto_ir;
                    }
                }
                else
                {
                    base_dir = "ir";
                }
            }

            char *ir_full = build_module_artifact_path(base_dir, ctx->module_name, ".ll");
            if (ir_full)
            {
                free(auto_ir);
                auto_ir = ir_full;
            }
            else
            {
                // Fallback: use base filename in the configured directory
                char *base = fs_get_base_filename(ctx->options->input_file);
                if (base_dir && strlen(base_dir) > 0)
                {
                    size_t len = strlen(base_dir) + 1 + strlen(base) + 4;
                    auto_ir    = malloc(len);
                    snprintf(auto_ir, len, "%s/%s.ll", base_dir, base);
                }
                else
                {
                    size_t len = strlen(base) + 4;
                    auto_ir    = malloc(len);
                    snprintf(auto_ir, len, "%s.ll", base);
                }
                free(base);
            }
            ir_path = auto_ir;
        }

        // ensure directory exists
        char *dir = fs_dirname(ir_path);
        if (dir)
        {
            fs_ensure_dir_recursive(dir);
            free(dir);
        }

        if (!codegen_emit_llvm_ir(&ctx->codegen, ir_path))
        {
            fprintf(stderr, "error: failed to emit llvm ir '%s'\n", ir_path);
        }
        free(auto_ir);
    }

    char *auto_asm = NULL;
    if (emit_asm)
    {
        const char *asm_path = ctx->options->emit_asm_path;
        if (!asm_path || asm_path[0] == '\0')
        {
            // Use config-based directory if available, otherwise derive from obj_dir
            const char *base_dir = auto_asm_dir;
            if (!base_dir)
            {
                base_dir     = ctx->options->obj_dir;
                char *parent = NULL;
                if (base_dir)
                {
                    parent = fs_dirname(base_dir);
                    if (parent)
                    {
                        size_t len = strlen(parent) + 5;
                        auto_asm   = malloc(len);
                        snprintf(auto_asm, len, "%s/asm", parent);
                        free(parent);
                        base_dir = auto_asm;
                    }
                }
                else
                {
                    base_dir = "asm";
                }
            }

            char *asm_full = build_module_artifact_path(base_dir, ctx->module_name, ".s");
            if (asm_full)
            {
                free(auto_asm);
                auto_asm = asm_full;
            }
            else
            {
                // Fallback: use base filename in the configured directory
                char *base = fs_get_base_filename(ctx->options->input_file);
                if (base_dir && strlen(base_dir) > 0)
                {
                    size_t len = strlen(base_dir) + 1 + strlen(base) + 3;
                    auto_asm   = malloc(len);
                    snprintf(auto_asm, len, "%s/%s.s", base_dir, base);
                }
                else
                {
                    size_t len = strlen(base) + 3;
                    auto_asm   = malloc(len);
                    snprintf(auto_asm, len, "%s.s", base);
                }
                free(base);
            }
            asm_path = auto_asm;
        }

        // ensure directory exists
        char *dir = fs_dirname(asm_path);
        if (dir)
        {
            fs_ensure_dir_recursive(dir);
            free(dir);
        }

        if (!codegen_emit_assembly(&ctx->codegen, asm_path))
        {
            fprintf(stderr, "error: failed to emit assembly '%s'\n", asm_path);
        }
        free(auto_asm);
    }

    // determine object file path and store it in context
    const char *obj_dir_to_use = auto_obj_dir ? auto_obj_dir : ctx->options->obj_dir;

    if (!ctx->options->link_exe)
    {
        // --no-link: object is primary output
        if (ctx->options->output_file)
        {
            ctx->source = strdup(ctx->options->output_file);
        }
        else if (obj_dir_to_use)
        {
            char *obj_path = build_module_artifact_path(obj_dir_to_use, ctx->module_name, ".o");
            if (obj_path)
            {
                ctx->source = obj_path;
            }
            else
            {
                // Fallback: use base filename in the configured directory
                char  *base = fs_get_base_filename(ctx->options->input_file);
                size_t len  = strlen(obj_dir_to_use) + 1 + strlen(base) + 3;
                ctx->source = malloc(len);
                snprintf(ctx->source, len, "%s/%s.o", obj_dir_to_use, base);
                free(base);
            }
        }
        else
        {
            char  *base = fs_get_base_filename(ctx->options->input_file);
            size_t len  = strlen(base) + 3;
            ctx->source = malloc(len);
            snprintf(ctx->source, len, "%s.o", base);
            free(base);
        }
    }
    else
    {
        // linking: place object in obj_dir if specified, else current dir
        if (obj_dir_to_use)
        {
            char *obj_path = build_module_artifact_path(obj_dir_to_use, ctx->module_name, ".o");
            if (obj_path)
            {
                ctx->source = obj_path;
            }
            else
            {
                // Fallback: use base filename in the configured directory
                char  *base = fs_get_base_filename(ctx->options->input_file);
                size_t len  = strlen(obj_dir_to_use) + 1 + strlen(base) + 3;
                ctx->source = malloc(len);
                snprintf(ctx->source, len, "%s/%s.o", obj_dir_to_use, base);
                free(base);
            }
        }
        else
        {
            char  *base = fs_get_base_filename(ctx->options->input_file);
            size_t len  = strlen(base) + 3;
            ctx->source = malloc(len);
            snprintf(ctx->source, len, "%s.o", base);
            free(base);
        }
    }

    // ensure directory exists for object file
    char *obj_dir = fs_dirname(ctx->source);
    if (obj_dir)
    {
        fs_ensure_dir_recursive(obj_dir);
        free(obj_dir);
    }

    if (!codegen_emit_object(&ctx->codegen, ctx->source))
    {
        fprintf(stderr, "error: failed to write object file '%s'\n", ctx->source);
        return false;
    }

    return true;
}

bool compilation_compile_dependencies(CompilationContext *ctx)
{
    if (!ctx->config)
        return true;

    // Use the same obj directory structure for dependencies
    const char *target_name = ctx->options->target_name;
    char        dep_out_dir[1024];

    if (target_name && ctx->project_root)
    {
        // For project builds, use target-based directory
        char *obj_dir = config_resolve_obj_dir(ctx->config, ctx->project_root, target_name);
        if (obj_dir)
        {
            snprintf(dep_out_dir, sizeof(dep_out_dir), "%s", obj_dir);
            free(obj_dir);
        }
        else
        {
            snprintf(dep_out_dir, sizeof(dep_out_dir), "%s/out/obj", ctx->project_root);
        }
    }
    else if (ctx->options->obj_dir)
    {
        // Use explicitly provided obj_dir
        snprintf(dep_out_dir, sizeof(dep_out_dir), "%s", ctx->options->obj_dir);
    }
    else
    {
        snprintf(dep_out_dir, sizeof(dep_out_dir), "%s/out/obj", ctx->project_root);
    }
    fs_ensure_dir_recursive(dep_out_dir);

    if (!module_manager_compile_dependencies(
            &ctx->driver->module_manager, dep_out_dir, ctx->options->opt_level, ctx->options->no_pie, ctx->options->debug_info, ctx->options->emit_asm, ctx->options->emit_ir, ctx->options->emit_ast, &ctx->driver->spec_cache))
    {
        fprintf(stderr, "error: failed to compile dependencies\n");
        return false;
    }

    module_manager_get_link_objects(&ctx->driver->module_manager, &ctx->dep_objects, &ctx->dep_count);
    return true;
}

bool compilation_link(CompilationContext *ctx)
{
    if (!ctx->options->link_exe)
        return true;

    char *exe = NULL;
    if (!ctx->options->output_file)
    {
        // No -o specified: derive output path
        if (ctx->options->target_name && ctx->project_root && ctx->config)
        {
            // Project mode: use config_resolve_final_output_path for per-target paths
            exe = config_resolve_final_output_path(ctx->config, ctx->project_root, ctx->options->target_name);
            if (exe)
            {
                // Ensure parent directory exists
                char *dir = fs_dirname(exe);
                if (dir)
                {
                    fs_ensure_dir_recursive(dir);
                    free(dir);
                }
            }
            else
            {
                // Fallback to entrypoint basename
                exe = fs_get_base_filename(ctx->options->input_file);
            }
        }
        else
        {
            // Single-file mode: use entrypoint basename
            exe = fs_get_base_filename(ctx->options->input_file);
        }
    }
    else
    {
        // -o specified: use it exactly as provided
        exe = strdup(ctx->options->output_file);

        // Ensure parent directory exists
        char *dir = fs_dirname(exe);
        if (dir)
        {
            fs_ensure_dir_recursive(dir);
            free(dir);
        }
    }
    // use object file path stored in ctx->source from emit_artifacts
    const char *obj_file = ctx->source;
    if (!obj_file)
    {
        fprintf(stderr, "error: object file path not set\n");
        free(exe);
        return false;
    }

    // calculate command size
    size_t cmd_size = 256;
    cmd_size += strlen(exe);
    cmd_size += strlen(obj_file);
    for (int i = 0; i < ctx->dep_count; i++)
        cmd_size += strlen(ctx->dep_objects[i]) + 1;
    for (int i = 0; i < ctx->options->link_objects.count; i++)
        cmd_size += strlen(ctx->options->link_objects.items[i]) + 1;

    char *cmd = malloc(cmd_size);
    if (!cmd)
    {
        fprintf(stderr, "error: out of memory during link\n");
        free(exe);
        return false;
    }

    strcpy(cmd, "cc -nostartfiles -nostdlib");
    if (ctx->options->no_pie)
        strcat(cmd, " -no-pie");
    else
        strcat(cmd, " -pie");
    if (ctx->options->debug_info)
        strcat(cmd, " -g");
    strcat(cmd, " -o ");
    strcat(cmd, exe);
    strcat(cmd, " ");
    strcat(cmd, obj_file);

    for (int i = 0; i < ctx->dep_count; i++)
    {
        strcat(cmd, " ");
        strcat(cmd, ctx->dep_objects[i]);
    }
    for (int i = 0; i < ctx->options->link_objects.count; i++)
    {
        strcat(cmd, " ");
        strcat(cmd, ctx->options->link_objects.items[i]);
    }

    int result = system(cmd);
    if (result != 0)
    {
        fprintf(stderr, "error: failed to link executable '%s'\n", exe);
        free(cmd);
        free(exe);
        return false;
    }

    free(cmd);
    free(exe);
    return true;
}

bool compilation_run(CompilationContext *ctx)
{
    if (!compilation_load_and_preprocess(ctx))
        return false;

    if (!compilation_parse(ctx))
        return false;

    if (!compilation_analyze(ctx))
        return false;

    if (!compilation_codegen(ctx))
        return false;

    if (!compilation_emit_artifacts(ctx))
        return false;

    if (!compilation_compile_dependencies(ctx))
        return false;

    if (!compilation_link(ctx))
        return false;

    return true;
}
