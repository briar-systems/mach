#include <stdio.h>
#include <string.h>

#include "commands/cmd_build.h"
#include "commands/cmd_dep.h"
#include "commands/cmd_help.h"
#include "commands/cmd_init.h"
#include "commands/cmd_run.h"

void cmd_help_general()
{
    fprintf(stderr, "usage: mach <command> [options]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "commands:\n");
    fprintf(stderr, "  init  <name>         create a new Mach project\n");
    fprintf(stderr, "  build <path|file>    build a project or single file\n");
    fprintf(stderr, "  run   <path>         run a project executable\n");
    fprintf(stderr, "  dep   <subcommand>   manage project dependencies\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "use 'mach help <command>' for more information on a specific command.\n");
}

void cmd_help_help()
{
    fprintf(stderr, "usage: mach help <command>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "display help information for a specific command.\n");
}

int cmd_help_handle(int argc, char **argv)
{
    if (argc < 3)
    {
        cmd_help_general();
        return 1;
    }

    const char *command = argv[2];

    if (strcmp(command, "init") == 0)
    {
        cmd_init_help();
    }
    else if (strcmp(command, "build") == 0)
    {
        cmd_build_help();
    }
    else if (strcmp(command, "run") == 0)
    {
        cmd_run_help();
    }
    else if (strcmp(command, "dep") == 0)
    {
        cmd_dep_help();
    }
    else
    {
        fprintf(stderr, "error: unknown command '%s'\n", command);
        cmd_help_general();
        return 1;
    }

    return 0;
}
