#include <stdio.h>
#include <string.h>

#include "commands/cmd_build.h"
#include "commands/cmd_dep.h"
#include "commands/cmd_help.h"
#include "commands/cmd_init.h"
#include "commands/cmd_run.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cmd_help_general(stderr);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "init") == 0)
    {
        return cmd_init_handle(argc, argv);
    }
    else if (strcmp(command, "build") == 0)
    {
        return cmd_build_handle(argc, argv);
    }
    else if (strcmp(command, "run") == 0)
    {
        return cmd_run_handle(argc, argv);
    }
    else if (strcmp(command, "dep") == 0)
    {
        return cmd_dep_handle(argc, argv);
    }
    else if (strcmp(command, "help") == 0)
    {
        return cmd_help_handle(argc, argv);
    }
    else
    {
        fprintf(stderr, "error: unknown command '%s'\n", command);
        cmd_help_general(stderr);
        return 1;
    }
}
