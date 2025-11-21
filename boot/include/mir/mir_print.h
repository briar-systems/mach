#ifndef MIR_PRINT_H
#define MIR_PRINT_H

#include "mir.h"
#include <stdio.h>

// print entire MIR module
void mir_print_module(FILE *out, MirModule *module);

// print single basic block
void mir_print_block(FILE *out, MirBasicBlock *block);

#endif // MIR_PRINT_H
