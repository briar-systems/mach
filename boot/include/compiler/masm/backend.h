#ifndef MASM_BACKEND_H
#define MASM_BACKEND_H

#include "compiler/masm/masm.h"

// The MasmBackend interface defines how portable MASM IR is translated
// into target-specific machine code.
typedef struct MasmBackend
{
    const char *name;

    // Run the backend code generation pipeline.
    // This transforms the high-level IR instructions in the Masm module
    // into encoded machine bytes stored in the section data buffers.
    //
    // Steps typically include:
    // 1. Instruction Selection (IR -> Target Opcodes)
    // 2. Register Allocation (Virtual -> Physical)
    // 3. Encoding (Target Instructions -> Bytes)
    void (*codegen)(Masm *masm);

} MasmBackend;

// Retrieve the backend for the specified ISA
const MasmBackend *masm_backend_get(MasmTargetISA isa);

#endif // MASM_BACKEND_H
