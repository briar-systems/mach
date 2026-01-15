#ifndef MASM_OF_SPEC_H
#define MASM_OF_SPEC_H

#include "compiler/masm/masm.h"

typedef struct MasmOFSpec
{
    const char *name;
    int (*write_object)(Masm *masm, const char *filename);
} MasmOFSpec;

const MasmOFSpec *masm_of_spec_select(MasmTargetOF of);

#endif // MASM_OF_SPEC_H
