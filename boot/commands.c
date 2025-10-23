#include "commands.h"
#include "compilation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mach_print_usage(const char *program_name)
{
    fprintf(stderr, "usage: %s build <file> [options]\n", program_name);
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -o <file>          output file (executable or object)\n");
    fprintf(stderr, "  -O<level>          optimization level (0-3, default: 2)\n");
    fprintf(stderr, "  --no-link          compile only, don't link (produces object file)\n");
    fprintf(stderr, "  --emit-asm[=<file>]  emit target assembly\n");
    fprintf(stderr, "  --emit-ir[=<file>]   emit LLVM IR\n");
    fprintf(stderr, "  --emit-ast[=<file>]  emit parsed AST for debugging\n");
    fprintf(stderr, "  --obj-dir=<dir>    directory for intermediate object files\n");
    fprintf(stderr, "  --dep-dir=<dir>    directory for dependency object files\n");
    fprintf(stderr, "  --no-pie           disable position independent executable\n");
    fprintf(stderr, "  --link <obj>       link with additional object file\n");
    fprintf(stderr, "  -g, --debug        include debug info (default)\n");
    fprintf(stderr, "  --no-debug         disable debug info\n");
    fprintf(stderr, "  -I <dir>           add module search directory\n");
    fprintf(stderr, "  -M n=dir           map module prefix 'n' to base directory 'dir'\n");
    fprintf(stderr, "  --std <path>       path to standard library (can also use MACH_STD env var)\n");
}

int mach_cmd_build(int argc, char **argv)
{
    if (argc < 3)
    {
        mach_print_usage(argv[0]);
        return 1;
    }

    BuildOptions opts;
    build_options_init(&opts);
    opts.input_file = argv[2];

    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
                opts.output_file = argv[++i];
            else
            {
                fprintf(stderr, "error: -o requires a filename\n");
                build_options_dnit(&opts);
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
                return 1;
            }
        }
        else if (strcmp(argv[i], "--emit-obj") == 0)
        {
            opts.emit_obj = 1;
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
        else if (strncmp(argv[i], "--dep-dir=", 10) == 0)
        {
            opts.dep_dir = argv[i] + 10;
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
                    return 1;
                }
                *eq = '\0';
                build_options_add_alias(&opts, a, eq + 1);
            }
            else
            {
                fprintf(stderr, "error: -M requires name=dir\n");
                build_options_dnit(&opts);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--std") == 0)
        {
            if (i + 1 < argc)
            {
                config_set_std_path(argv[++i]);
            }
            else
            {
                fprintf(stderr, "error: --std requires a path\n");
                build_options_dnit(&opts);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            build_options_dnit(&opts);
            return 1;
        }
    }

    CompilationContext ctx;
    if (!compilation_context_init(&ctx, &opts))
    {
        build_options_dnit(&opts);
        return 1;
    }

    bool success = compilation_run(&ctx);

    compilation_context_dnit(&ctx);
    build_options_dnit(&opts);

    return success ? 0 : 1;
}
