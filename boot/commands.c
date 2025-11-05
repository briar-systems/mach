#include "commands.h"
#include "compilation.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mach_print_usage(const char *program_name)
{
    fprintf(stderr, "usage: %s build <path|file> [options]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "  <path>               build project from directory (requires mach.toml)\n");
    fprintf(stderr, "  <file.mach>          compile single file (no mach.toml required)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "project options:\n");
    fprintf(stderr, "  --target <name>      select target from mach.toml (required for projects)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "compile options:\n");
    fprintf(stderr, "  -o <file>            output file (executable or object)\n");
    fprintf(stderr, "  -O<level>            optimization level (0-3, overrides config)\n");
    fprintf(stderr, "  --no-link            compile only, don't link (produces object file)\n");
    fprintf(stderr, "  --emit-asm[=<file>]  emit target assembly (overrides config)\n");
    fprintf(stderr, "  --emit-ir[=<file>]   emit LLVM IR (overrides config)\n");
    fprintf(stderr, "  --emit-ast[=<file>]  emit parsed AST for debugging (overrides config)\n");
    fprintf(stderr, "  --obj-dir=<dir>      override object file directory\n");
    fprintf(stderr, "  --no-pie             disable position independent executable (overrides config)\n");
    fprintf(stderr, "  --link <obj>         link with additional object file\n");
    fprintf(stderr, "  -g, --debug          include debug info\n");
    fprintf(stderr, "  --no-debug           disable debug info\n");
    fprintf(stderr, "  -I <dir>             add module search directory\n");
    fprintf(stderr, "  -M n=dir             map module prefix 'n' to base directory 'dir'\n");
}

int mach_cmd_build(int argc, char **argv)
{
    if (argc < 3)
    {
        mach_print_usage(argv[0]);
        return 1;
    }

    const char *input_arg = argv[2];

    // Determine build mode: project or single-file
    int is_project_build = fs_is_directory(input_arg);
    int is_single_file   = fs_is_mach_file(input_arg);

    if (!is_project_build && !is_single_file)
    {
        fprintf(stderr, "error: '%s' is not a directory or .mach file\n", input_arg);
        return 1;
    }

    // Initialize structures
    char          *project_root = NULL;
    ProjectConfig *config       = NULL;
    BuildOptions   opts;
    build_options_init(&opts);

    const char   *target_name = NULL;
    TargetConfig *target      = NULL;

    // === PROJECT BUILD MODE ===
    if (is_project_build)
    {
        // Step 1: Load project configuration
        project_root = realpath(input_arg, NULL);
        if (!project_root)
        {
            fprintf(stderr, "error: could not resolve path '%s'\n", input_arg);
            build_options_dnit(&opts);
            return 1;
        }

        config = config_load_from_dir(project_root);
        if (!config)
        {
            fprintf(stderr, "error: could not load mach.toml from '%s'\n", project_root);
            if (project_root)
                free(project_root);
            build_options_dnit(&opts);
            return 1;
        }

        // Validate config
        if (!config_validate(config))
        {
            fprintf(stderr, "error: invalid configuration in mach.toml\n");
            if (config)
            {
                config_dnit(config);
            }

            if (project_root)
                free(project_root);
            build_options_dnit(&opts);
            return 1;
        }

        // Step 2: Parse --target flag (required for project builds)
        for (int i = 3; i < argc; i++)
        {
            if (strcmp(argv[i], "--target") == 0 || strncmp(argv[i], "--target=", 9) == 0)
            {
                if (argv[i][8] == '=')
                {
                    target_name = argv[i] + 9;
                }
                else if (i + 1 < argc)
                {
                    target_name = argv[++i];
                }
                else
                {
                    fprintf(stderr, "error: --target requires a target name\n");
                    build_options_dnit(&opts);
                    if (config)
                    {
                        config_dnit(config);
                    }

                    if (project_root)
                        free(project_root);
                    return 1;
                }
                break;
            }
        }

        if (!target_name)
        {
            // Try to use default target from config
            target = config_get_default_target(config);
            if (!target)
            {
                fprintf(stderr, "error: --target is required for project builds\n");
                fprintf(stderr, "available targets:");
                for (int i = 0; i < config->target_count; i++)
                {
                    fprintf(stderr, " %s", config->targets[i]->name);
                }
                fprintf(stderr, "\n");
                build_options_dnit(&opts);
                if (config)
                {
                    config_dnit(config);
                }

                if (project_root)
                    free(project_root);
                return 1;
            }
            target_name = target->name;
        }
        else if (strcmp(target_name, "all") == 0)
        {
            // build all targets
            bool all_success = true;

            for (int t = 0; t < config->target_count; t++)
            {
                TargetConfig *current_target      = config->targets[t];
                const char   *current_target_name = current_target->name;

                // create separate build options for this target
                BuildOptions target_opts;
                build_options_init(&target_opts);

                // Resolve entrypoint for this target
                char *entrypoint_path = config_resolve_target_entrypoint(config, project_root, current_target_name);
                if (!entrypoint_path)
                {
                    fprintf(stderr, "error: could not resolve entrypoint for target '%s'\n", current_target_name);
                    build_options_dnit(&target_opts);
                    all_success = false;
                    continue;
                }

                target_opts.input_file  = entrypoint_path;
                target_opts.opt_level   = current_target->opt_level;
                target_opts.emit_ast    = current_target->emit_ast;
                target_opts.emit_ir     = current_target->emit_ir;
                target_opts.emit_asm    = current_target->emit_asm;
                target_opts.no_pie      = current_target->no_pie;
                target_opts.debug_info  = 1;
                target_opts.target_name = current_target_name;

                // Set up target-based output directories
                target_opts.obj_dir = config_resolve_obj_dir(config, project_root, current_target_name);

                // Register project source directory as module alias (use id for module prefix)
                char *src_dir = config_resolve_src_dir(config, project_root);
                if (src_dir)
                {
                    build_options_add_alias(&target_opts, config->id, src_dir);
                    free(src_dir);
                }

                // Copy CLI-provided options
                for (int i = 0; i < opts.include_paths.count; i++)
                    build_options_add_include(&target_opts, opts.include_paths.items[i]);
                for (int i = 0; i < opts.link_objects.count; i++)
                    build_options_add_link_object(&target_opts, opts.link_objects.items[i]);
                for (int i = 0; i < opts.aliases.count; i++)
                    build_options_add_alias(&target_opts, opts.aliases.names[i], opts.aliases.dirs[i]);

                // Run compilation for this target
                CompilationContext ctx;
                if (!compilation_context_init(&ctx, &target_opts))
                {
                    fprintf(stderr, "error: failed to initialize compilation context for target '%s'\n", current_target_name);
                    build_options_dnit(&target_opts);
                    all_success = false;
                    continue;
                }

                bool success = compilation_run(&ctx);
                if (!success)
                {
                    fprintf(stderr, "error: build failed for target '%s'\n", current_target_name);
                    all_success = false;
                }

                compilation_context_dnit(&ctx);
                build_options_dnit(&target_opts);
            }

            // Clean up and return
            build_options_dnit(&opts);
            if (config)
            {
                config_dnit(config);
            }
            if (project_root)
                free(project_root);

            return all_success ? 0 : 1;
        }
        else
        {
            target = config_get_target(config, target_name);
            if (!target)
            {
                fprintf(stderr, "error: target '%s' not found in mach.toml\n", target_name);
                fprintf(stderr, "available targets:");
                for (int i = 0; i < config->target_count; i++)
                {
                    fprintf(stderr, " %s", config->targets[i]->name);
                }
                fprintf(stderr, "\n");
                build_options_dnit(&opts);
                if (config)
                {
                    config_dnit(config);
                }

                if (project_root)
                    free(project_root);
                return 1;
            }
        }

        // Step 3: Resolve entrypoint from target
        char *entrypoint_path = config_resolve_target_entrypoint(config, project_root, target_name);
        if (!entrypoint_path)
        {
            fprintf(stderr, "error: could not resolve entrypoint for target '%s'\n", target_name);
            build_options_dnit(&opts);
            if (config)
            {
                config_dnit(config);
            }

            if (project_root)
                free(project_root);
            return 1;
        }

        opts.input_file = entrypoint_path;

        // Step 4: Apply config defaults from target
        opts.opt_level   = target->opt_level;
        opts.emit_ast    = target->emit_ast;
        opts.emit_ir     = target->emit_ir;
        opts.emit_asm    = target->emit_asm;
        opts.no_pie      = target->no_pie;
        opts.debug_info  = 1;           // default to debug info
        opts.target_name = target_name; // store target name for directory structure

        // Step 4.5: Register project source directory as module alias
        // this ensures entrypoint and project files are namespaced under the project id
        char *src_dir = config_resolve_src_dir(config, project_root);
        if (src_dir)
        {
            build_options_add_alias(&opts, config->id, src_dir);
            free(src_dir);
        }

        // Step 5: Set up target-based output directories if not overridden
        if (!opts.obj_dir)
        {
            opts.obj_dir = config_resolve_obj_dir(config, project_root, target_name);
        }
    }
    // === SINGLE-FILE BUILD MODE ===
    else
    {
        // Single file mode - try to find project root but don't require it
        opts.input_file = input_arg;

        project_root = fs_find_project_root(input_arg);
        if (project_root)
        {
            config = config_load_from_dir(project_root);
            // Config is optional for single-file builds
            if (config && config_validate(config))
            {
                // Can optionally use config defaults, but CLI overrides
                fprintf(stderr, "note: found mach.toml, using as reference\n");
            }
            else if (config)
            {
                // Invalid config - ignore it
                if (config)
                {
                    config_dnit(config);
                }

                config = NULL;
            }
        }

        // Apply CLI defaults for single-file mode
        opts.opt_level  = 2;
        opts.debug_info = 1;
        opts.emit_ast   = 0;
        opts.emit_ir    = 0;
        opts.emit_asm   = 0;
    }

    // === PARSE CLI ARGUMENTS (both modes) ===
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "--target") == 0 || strncmp(argv[i], "--target=", 9) == 0)
        {
            // already handled
            if (argv[i][8] != '=')
                i++; // skip the argument
            continue;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
                opts.output_file = argv[++i];
            else
            {
                fprintf(stderr, "error: -o requires a filename\n");
                build_options_dnit(&opts);
                if (config)
                {
                    if (config)
                    {
                        config_dnit(config);
                    }
                }
                if (project_root)
                    if (project_root)
                        free(project_root);
                return 1;
            }
        }
        else if (strncmp(argv[i], "-O", 2) == 0)
        {
            opts.opt_level = atoi(argv[i] + 2);
            if (opts.opt_level < 0 || opts.opt_level > 3)
            {
                fprintf(stderr, "error: invalid optimization level\n");
                build_options_dnit(&opts);
                if (config)
                {
                    if (config)
                    {
                        config_dnit(config);
                    }
                }
                if (project_root)
                    if (project_root)
                        free(project_root);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--no-link") == 0)
        {
            opts.link_exe = 0;
        }
        else if (strncmp(argv[i], "--emit-ast", 10) == 0)
        {
            opts.emit_ast = 1;
            if (argv[i][10] == '=' && argv[i][11] != '\0')
                opts.emit_ast_path = argv[i] + 11;
        }
        else if (strncmp(argv[i], "--emit-ir", 9) == 0)
        {
            opts.emit_ir = 1;
            if (argv[i][9] == '=' && argv[i][10] != '\0')
                opts.emit_ir_path = argv[i] + 10;
        }
        else if (strncmp(argv[i], "--emit-asm", 10) == 0)
        {
            opts.emit_asm = 1;
            if (argv[i][10] == '=' && argv[i][11] != '\0')
                opts.emit_asm_path = argv[i] + 11;
        }
        else if (strncmp(argv[i], "--obj-dir=", 10) == 0)
        {
            opts.obj_dir = argv[i] + 10;
        }
        else if (strcmp(argv[i], "--no-pie") == 0)
        {
            opts.no_pie = 1;
        }
        else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--debug") == 0)
        {
            opts.debug_info = 1;
        }
        else if (strcmp(argv[i], "--no-debug") == 0)
        {
            opts.debug_info = 0;
        }
        else if (strcmp(argv[i], "--link") == 0)
        {
            if (i + 1 < argc)
            {
                build_options_add_link_object(&opts, argv[++i]);
            }
            else
            {
                fprintf(stderr, "error: --link requires an object file\n");
                build_options_dnit(&opts);
                if (config)
                {
                    config_dnit(config);
                }

                if (project_root)
                    free(project_root);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-I") == 0)
        {
            if (i + 1 < argc)
                build_options_add_include(&opts, argv[++i]);
            else
            {
                fprintf(stderr, "error: -I requires a directory\n");
                build_options_dnit(&opts);
                if (config)
                {
                    config_dnit(config);
                }

                if (project_root)
                    free(project_root);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-M") == 0)
        {
            if (i + 1 < argc)
            {
                char *a  = argv[++i];
                char *eq = strchr(a, '=');
                if (!eq)
                {
                    fprintf(stderr, "error: -M expects name=dir\n");
                    build_options_dnit(&opts);
                    if (config)
                    {
                        config_dnit(config);
                    }

                    if (project_root)
                        free(project_root);
                    return 1;
                }
                *eq = '\0';
                build_options_add_alias(&opts, a, eq + 1);
            }
            else
            {
                fprintf(stderr, "error: -M requires name=dir\n");
                build_options_dnit(&opts);
                if (config)
                {
                    config_dnit(config);
                }

                if (project_root)
                    free(project_root);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            build_options_dnit(&opts);
            if (config)
            {
                config_dnit(config);
            }

            if (project_root)
                free(project_root);
            return 1;
        }
    }

    // Step 5: Run compilation
    CompilationContext ctx;
    if (!compilation_context_init(&ctx, &opts))
    {
        build_options_dnit(&opts);
        if (config)
        {
            if (config)
            {
                config_dnit(config);
            }
        }
        if (project_root)
            if (project_root)
                free(project_root);
        return 1;
    }

    bool success = compilation_run(&ctx);

    compilation_context_dnit(&ctx);
    build_options_dnit(&opts);
    if (config)
    {
        if (config)
        {
            config_dnit(config);
        }
    }
    if (project_root)
        if (project_root)
            free(project_root);

    return success ? 0 : 1;
}
