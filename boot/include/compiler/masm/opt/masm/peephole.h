#ifndef MASM_OPT_MASM_PEEPHOLE_H
#define MASM_OPT_MASM_PEEPHOLE_H

#include "compiler/masm/masm.h"

// IR-level peephole optimization pass (runs pre-isel)
void masm_opt_ir_run(Masm *masm);

#endif
