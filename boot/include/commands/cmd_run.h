#ifndef CMD_RUN_H
#define CMD_RUN_H

#include <stdio.h>

void cmd_run_help(FILE *stream);
int  cmd_run_handle(int argc, char **argv);

#endif
