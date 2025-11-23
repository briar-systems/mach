#include "commands/cmd_build.h"

#include <stdio.h>

void cmd_build_help(FILE *stream)
{
    fprintf(stream, "usage: mach build <project|file> [options]\n");
    fprintf(stream, "\n");
    fprintf(stream, "build a Mach project from the specified directory or compile a single Mach source file\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target from mach.toml (required for projects)\n");
    fprintf(stream, "  -o <file>            output file (executable or object)\n");
    fprintf(stream, "  -I n=dir             map module prefix 'n' to base directory 'dir'\n");
}

int cmd_build_handle(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    fprintf(stderr, "error: build command not yet implemented\n");
    return 1;
}
