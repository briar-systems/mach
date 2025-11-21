#ifndef CMD_INIT_H
#define CMD_INIT_H

#include <stdio.h>

void cmd_help_general(FILE *stream);
void cmd_init_help(FILE *stream);
int  cmd_init_handle(int argc, char **argv);

#endif
