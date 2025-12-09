#ifndef MASM_OS_SPEC_H
#define MASM_OS_SPEC_H

#include "compiler/masm/target.h"

typedef struct MasmOSSpec
{
    const char *name;
} MasmOSSpec;

const MasmOSSpec *masm_os_spec_select(MasmTargetOS os);

#endif // MASM_OS_SPEC_H
