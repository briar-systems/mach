#include "commands/cmd_build.h"
#include "compiler/lexer.h"
#include "compiler/mir/emit.h"
#include "compiler/mir/lower.h"
#include "compiler/mir/target.h"
#include "compiler/parser.h"
#include "compiler/sema.h"
#include "config.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmd_build_help(FILE *stream)
{
    fprintf(stream, "usage: mach build <project|file> [options]\n");
    fprintf(stream, "\n");
    fprintf(stream, "build a Mach project from the specified directory or compile a single Mach source file\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target from mach.toml (required for projects)\n");
    fprintf(stream, "  -o <file>            output file (executable or object)\n");
    fprintf(stream, "  -m <path>            set module path (e.g. 'std.io')\n");
    fprintf(stream, "  -I n=dir             map module prefix 'n' to base directory 'dir'\n");
}

int cmd_build_handle(int argc, char **argv)
{
    // argv[0] = "cmach", argv[1] = "build", argv[2] = input file
    if (argc < 3)
    {
        fprintf(stderr, "error: no input file specified\n");
        cmd_build_help(stderr);
        return 1;
    }

    const char *input_file  = argv[2];
    const char *output_file = NULL;

    // parse options
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            output_file = argv[++i];
        }
    }

    // check if input is a directory
    bool        is_project    = is_directory(input_file);
    const char *project_root  = is_project ? input_file : NULL;
    const char *target_binary = NULL;

    // module resolution info (stored for sema)
    char *project_id = NULL;
    char *src_root   = NULL;
    char *dep_root   = NULL;

    if (is_project)
    {
        char *config_path = path_join(input_file, "mach.toml");
        if (!file_exists(config_path))
        {
            fprintf(stderr, "error: directory '%s' does not contain mach.toml\n", input_file);
            free(config_path);
            return 1;
        }

        Config *config = config_load(config_path);
        free(config_path);

        if (!config)
        {
            return 1; // config_load prints error
        }

        // find target (default to first or specified)
        if (config->target_count == 0)
        {
            fprintf(stderr, "error: no targets defined in mach.toml\n");
            config_dnit(config);
            free(config);
            return 1;
        }

        ConfigTarget *target = config->targets[0];
        if (!target->entrypoint)
        {
            fprintf(stderr, "error: target '%s' has no entrypoint\n", target->name);
            config_dnit(config);
            free(config);
            return 1;
        }

        // Store target binary path for output location
        if (target->binary)
        {
            target_binary = strdup(target->binary);
        }

        // Store module resolution info
        if (config->id)
        {
            project_id = strdup(config->id);
        }

        // construct full path to entrypoint
        char *src_dir_path = path_join(input_file, config->dir_src ? config->dir_src : "src");
        char *entry_path   = path_join(src_dir_path, target->entrypoint);

        // store absolute src_root for module resolution
        src_root = absolutize_path(src_dir_path);

        // store dep_root if configured
        if (config->dir_dep)
        {
            char *dep_dir_path = path_join(input_file, config->dir_dep);
            dep_root           = absolutize_path(dep_dir_path);
            free(dep_dir_path);
        }

        input_file = entry_path;

        free(src_dir_path);
        config_dnit(config);
        free(config);
    }

    // Determine output file
    if (!output_file && is_project && project_root && target_binary)
    {
        // Build to out/<binary path> (binary is relative to out dir in mach.toml)
        // e.g. if binary = "linux/bin/mach", we build to "<project_root>/out/linux/bin/mach"
        char out_path[1024];
        snprintf(out_path, sizeof(out_path), "%s/out/%s", project_root, target_binary);
        output_file = strdup(out_path);

        // Create output directory if it doesn't exist
        char *out_dir  = strdup(output_file);
        char *last_sep = strrchr(out_dir, '/');
        if (last_sep)
        {
            *last_sep = '\0';
            // Create directory recursively
            char mkdir_cmd[1536];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s 2>/dev/null", out_dir);
            system(mkdir_cmd);
        }
        free(out_dir);
    }
    else if (!output_file)
    {
        output_file = "output";
    }

    // read source file
    FILE *f = fopen(input_file, "r");
    if (!f)
    {
        fprintf(stderr, "error: could not open file '%s'\n", input_file);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = malloc(size + 1);
    if (!source)
    {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return 1;
    }

    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    // lex
    Lexer lexer;
    lexer_init(&lexer, source);

    // parse
    Parser parser;
    parser_init(&parser, &lexer);

    AstNode *ast = parser_parse_program(&parser);
    if (!ast || parser.had_error)
    {
        fprintf(stderr, "error: parsing failed\n");
        parser_error_list_print(&parser.errors, &lexer, input_file);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // determine module path
    char *module_path = NULL;

    // check for -m flag
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc)
        {
            module_path = strdup(argv[++i]);
            break;
        }
    }

    // if not specified, try to derive from project
    if (!module_path)
    {
        char *project_root = find_project_root(input_file);
        if (project_root && strcmp(project_root, ".") != 0)
        {
            char   *config_path = path_join(project_root, "mach.toml");
            Config *config      = config_load(config_path);
            free(config_path);

            if (config)
            {
                // calculate relative path from src_dir
                char *src_dir   = path_join(project_root, config->dir_src ? config->dir_src : "src");
                char *abs_input = absolutize_path(input_file);

                if (strncmp(abs_input, src_dir, strlen(src_dir)) == 0)
                {
                    // file is inside src_dir
                    char *rel_path = abs_input + strlen(src_dir);
                    if (is_sep(*rel_path))
                    {
                        rel_path++; // skip leading separator
                    }

                    // construct module path: project_id + . + rel_path (with / -> .)
                    // remove extension .mach
                    char *rel_no_ext = strdup(rel_path);
                    char *dot        = strrchr(rel_no_ext, '.');
                    if (dot)
                    {
                        *dot = '\0';
                    }

                    // replace separators with dots
                    for (char *p = rel_no_ext; *p; p++)
                    {
                        if (is_sep(*p))
                        {
                            *p = '.';
                        }
                    }

                    size_t len  = strlen(config->id) + 1 + strlen(rel_no_ext) + 1;
                    module_path = malloc(len);
                    snprintf(module_path, len, "%s.%s", config->id, rel_no_ext);

                    free(rel_no_ext);
                }

                free(abs_input);
                free(src_dir);
                config_dnit(config);
                free(config);
            }
        }
        free(project_root);
    }

    // 3. Default to "main"
    if (!module_path)
    {
        module_path = strdup("main");
    }

    // semantic analysis
    Sema *sema = sema_create(module_path);
    free(module_path); // sema makes a copy
    if (!sema)
    {
        fprintf(stderr, "error: failed to create semantic analyzer\n");
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        if (project_id)
        {
            free(project_id);
        }
        if (src_root)
        {
            free(src_root);
        }
        if (dep_root)
        {
            free(dep_root);
        }
        return 1;
    }

    // set module resolution roots if available
    if (project_id && src_root)
    {
        sema_set_module_roots(sema, project_id, src_root, dep_root);
    }
    free(project_id);
    free(src_root);
    free(dep_root);

    // set file context for error reporting
    sema_set_file_context(sema, input_file, source);

    if (sema_analyze(sema, ast) < 0)
    {
        sema_print_errors(sema);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // lower to MIR - first the main module
    MIRModule *mir = mir_lower_module(ast, sema_get_root_table(sema));
    if (!mir)
    {
        fprintf(stderr, "error: lowering to MIR failed\n");
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // lower all imported modules
    AstNode *loaded_asts[64];
    int      loaded_count = sema_get_loaded_modules(sema, loaded_asts, 64);
    for (int i = 0; i < loaded_count; i++)
    {
        MIRModule *imported_mir = mir_lower_module(loaded_asts[i], sema_get_root_table(sema));
        if (imported_mir)
        {
            // merge imported module into main module
            mir_module_merge(mir, imported_mir);
            mir_module_destroy(imported_mir);
        }
    }

    // generate object file
    char obj_file[512];
    snprintf(obj_file, sizeof(obj_file), "%s.o", output_file);

    MIRTarget target = mir_target_native();
    if (mir_emit_object(mir, &target, obj_file) < 0)
    {
        fprintf(stderr, "error: failed to emit object file\n");
        mir_module_destroy(mir);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // link into executable
    char link_cmd[1024];
    snprintf(link_cmd, sizeof(link_cmd), "ld -o %s %s 2>&1", output_file, obj_file);

    int link_result = system(link_cmd);
    if (link_result != 0)
    {
        fprintf(stderr, "error: linking failed\n");
        mir_module_destroy(mir);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // Silent on success - no output

    // cleanup
    mir_module_destroy(mir);
    sema_destroy(sema);
    parser_dnit(&parser);
    lexer_dnit(&lexer);
    free(source);

    return 0;
}
