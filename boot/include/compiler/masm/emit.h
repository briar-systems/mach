#ifndef MASM_EMIT_H
#define MASM_EMIT_H

#include "compiler/masm/masm.h"

// emit masm to object file
int masm_emit_object(Masm *masm, const char *filename);

#endif // MASM_EMIT_H
