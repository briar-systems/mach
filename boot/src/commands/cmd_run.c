#include "commands/cmd_run.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmd_run_help(FILE* stream)
{
    fprintf(stream, "usage: mach run <path> [--target <name>] [args...]\n");
    fprintf(stream, "\n");
    fprintf(stream, "run the built executable for a Mach project located at the specified path\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  <path>               project directory (default: current directory)\n");
    fprintf(stream, "  --target <name>      select target to run (default: default project target)\n");
    fprintf(stream, "  [args...]            arguments to pass to the executable\n");
}

// execute a Mach project executable
// looks for mach.toml in the specified directory to determine target and output path
// passes any additional arguments to the executable
// returns exit code of the executed program
int cmd_run_handle(int argc, char **argv)
{
    const char *project_path = ".";
    const char *target_name  = NULL;
    int arg_start            = 3; // index of first arg to pass to executable

    // parse arguments
    if (argc >= 3)
    {
        project_path = argv[2];
    }

    // build project config path from project_path + "/mach.toml"
    char *project_root = NULL;
#ifdef _WIN32
    char resolved[PATH_MAX];
    if (_fullpath(resolved, project_path, PATH_MAX))
        project_root = strdup(resolved);
#else
    project_root = realpath(project_path, NULL);
#endif

    Config *config = config_load(project_root);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml\n");
        free(project_root);
        return 1;
    }

    // parse --target flag
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "--target") == 0)
        {
            if (i + 1 < argc)
            {
                target_name = argv[++i];
            }
            else
            {
                fprintf(stderr, "error: --target requires a target name\n");
                config_dnit(config);
                free(project_root);
                return 1;
            }
            arg_start = i + 1;
        }
        else if (strncmp(argv[i], "--target=", 9) == 0)
        {
            target_name = argv[i] + 9;
            arg_start   = i + 1;
        }
    }

    ConfigTarget *target = NULL;
    if (target_name)
    {
        target = config_get_target(config, target_name);
        if (!target)
        {
            fprintf(stderr, "error: target '%s' not found in mach.toml\n", target_name);
            config_dnit(config);
            free(project_root);
            return 1;
        }
    }
    else
    {
        target = config_get_default_target(config);
        if (!target)
        {
            fprintf(stderr, "error: no default target set in mach.toml\n");
            config_dnit(config);
            free(project_root);
            return 1;
        }
    }
    target_name = target->name;
}

    // resolve executable path
    char *exe_path = config_resolve_final_output_path(config, project_root, target_name);
    if (!exe_path)
    {
        fprintf(stderr, "error: failed to resolve executable path for target '%s'\n", target_name);
        config_dnit(config);
        free(project_root);
        return 1;
    }

    // check if executable exists
