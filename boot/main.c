// small dispatcher for commands; implementation is in src/commands.c
#include "commands.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        mach_print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "help") == 0)
    {
        mach_print_usage(argv[0]);
        return 0;
    }
    else if (strcmp(command, "build") == 0)
    {
        return mach_cmd_build(argc, argv);
    }
    else
    {
        fprintf(stderr, "error: unknown command '%s'\n", command);
        mach_print_usage(argv[0]);
        return 1;
    }
}
