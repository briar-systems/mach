#include "compiler/masm/backend.h"
#include <stddef.h>

// Forward declarations of backend implementations
extern const MasmBackend masm_backend_x86_64;

const MasmBackend *masm_backend_get(MasmTargetISA isa)
{
    switch (isa)
    {
        case MASM_ISA_X86_64:
            return &masm_backend_x86_64;
        default:
            return NULL;
    }
}