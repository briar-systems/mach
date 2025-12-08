#include "commands/cmd_run.h"
#include "config.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

// helper: execute process at path with args, return exit code
// platform-specific implementation
static int process_execute(const char *path, char **args)
{
#ifdef _WIN32
    // windows implementation using _spawnv
    int exit_code = _spawnv(_P_WAIT, path, (const char *const *)args);
    if (exit_code == -1)
    {
        fprintf(stderr, "error: failed to execute '%s'\n", path);
        return 1;
    }
    return exit_code;
#else
    // unix implementation (linux, darwin) using fork/execv
    pid_t pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "error: failed to fork process\n");
        return 1;
    }

    if (pid == 0)
    {
        // child process
        execv(path, args);
        // if execv returns, it failed
        fprintf(stderr, "error: failed to execute '%s'\n", path);
        exit(1);
    }
    else
    {
        // parent process
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            fprintf(stderr, "error: failed to wait for process\n");
            return 1;
        }

        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            // process was terminated by a signal
            return 128 + WTERMSIG(status);
        }
        return 1;
    }
#endif
}

void cmd_run_help(FILE *stream)
{
    fprintf(stream, "usage: mach run [--target <name>] [args...]\n");
    fprintf(stream, "\n");
    fprintf(stream, "run the built executable for a Mach project in the current directory\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target to run (default: default project target)\n");
    fprintf(stream, "  [args...]            arguments to pass to the executable\n");
}

// execute a Mach project executable
// looks for mach.toml in the current directory to determine target and output path
// passes any additional arguments to the executable
// returns exit code of the executed program
int cmd_run_handle(int argc, char **argv)
{
    const char *target_name  = NULL;
    const char *project_path = NULL;
    int         arg_start    = argc; // will compute

    // parse flags/positional args: first non-flag becomes project path (optional)
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--target") == 0)
        {
            if (i + 1 < argc)
            {
                target_name = argv[++i];
                continue;
            }
            else
            {
                fprintf(stderr, "error: --target requires a target name\n");
                return 1;
            }
        }
        else if (strncmp(argv[i], "--target=", 9) == 0)
        {
            target_name = argv[i] + 9;
            continue;
        }

        // first non-flag positional is project path
        if (!project_path)
        {
            project_path = argv[i];
            continue;
        }

        // the rest are executable arguments
        arg_start = i;
        break;
    }

    if (!project_path)
    {
        project_path = ".";
    }

    if (arg_start == argc)
    {
        arg_start = argc; // no args to pass
    }

    // find project root from provided path
    char *project_root = find_project_root(project_path);
    if (!project_root)
    {
        fprintf(stderr, "error: failed to find project root\n");
        return 1;
    }

    // build config path
    char *config_path = path_join(project_root, "mach.toml");
    if (!config_path)
    {
        fprintf(stderr, "error: failed to build config path\n");
        free(project_root);
        return 1;
    }

    Config *config = config_load(config_path);
    free(config_path);
    if (!config)
    {
        fprintf(stderr, "error: failed to load mach.toml from current directory\n");
        free(project_root);
        return 1;
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
        target = config_get_target(config, config->target);
        if (!target)
        {
            fprintf(stderr, "error: default target '%s' not found in mach.toml\n", config->target);
            config_dnit(config);
            free(project_root);
            return 1;
        }
    }

    if (target == NULL)
    {
        fprintf(stderr, "error: unable to find valid target\n");
        config_dnit(config);
        free(project_root);
        return 1;
    }

    // construct binary path
    char *binary_path = NULL;

    // note: binary_path is a mandatory field in ConfigTarget
    char *out_path = path_join(project_root, config->dir_out);
    binary_path = path_join(out_path, target->binary);
    free(out_path);

    // prepare arguments for execv
    int    exec_argc = argc - arg_start + 1;
    char **exec_argv = malloc(sizeof(char *) * (exec_argc + 1));
    exec_argv[0]     = binary_path;
    for (int i = 1; i < exec_argc; i++)
    {
        exec_argv[i] = argv[arg_start + i - 1];
    }
    exec_argv[exec_argc] = NULL;
    int exit_code        = process_execute(binary_path, exec_argv);

    free(exec_argv);
    free(binary_path);
    config_dnit(config);
    free(project_root);

    return exit_code;
}
