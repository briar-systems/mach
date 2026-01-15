#include "commands/cmd_build.h"
#include "commands/cmd_dep.h"
#include "commands/cmd_help.h"
#include "commands/cmd_init.h"
#include "commands/cmd_run.h"

#include <stdio.h>
#include <string.h>

void cmd_help_general(FILE *stream)
{
    fprintf(stream, "usage: mach <command> [options]\n");
    fprintf(stream, "\n");
    fprintf(stream, "commands:\n");
    fprintf(stream, "  init  <name>         create a new Mach project\n");
    fprintf(stream, "  build <path|file>    build a project or single file\n");
    fprintf(stream, "  run   <path>         run a project executable\n");
    fprintf(stream, "  dep   <subcommand>   manage project dependencies\n");
    fprintf(stream, "\n");
    fprintf(stream, "use 'mach help <command>' for more information on a specific command.\n");
}

void cmd_help_help(FILE *stream)
{
    fprintf(stream, "usage: mach help <command>\n");
    fprintf(stream, "\n");
    fprintf(stream, "display help information for a specific command.\n");
}

int cmd_help_handle(int argc, char **argv)
{
    if (argc < 3)
    {
        cmd_help_general(stderr);
        return 1;
    }

    const char *command = argv[2];

    if (strcmp(command, "init") == 0)
    {
        cmd_init_help(stdout);
    }
    else if (strcmp(command, "build") == 0)
    {
        cmd_build_help(stdout);
    }
    else if (strcmp(command, "run") == 0)
    {
        cmd_run_help(stdout);
    }
    else if (strcmp(command, "dep") == 0)
    {
        cmd_dep_help(stdout);
    }
    else
    {
        fprintf(stderr, "error: unknown command '%s'\n", command);
        cmd_help_general(stderr);
        return 1;
    }

    return 0;
}
