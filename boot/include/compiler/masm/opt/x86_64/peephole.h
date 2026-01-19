#ifndef MASM_OPT_X86_64_PEEPHOLE_H
#define MASM_OPT_X86_64_PEEPHOLE_H

#include "compiler/masm/masm.h"

// x86_64-specific peephole optimization pass (runs post-isel)
void masm_opt_x86_run(Masm *masm);

#endif
