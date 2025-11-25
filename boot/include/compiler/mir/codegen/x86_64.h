#ifndef MIR_CODEGEN_X86_64_H
#define MIR_CODEGEN_X86_64_H

#include "compiler/mir/function.h"
#include "compiler/mir/isa/x86_64.h"
#include <stddef.h>
#include <stdint.h>

// x86_64 code generation
// encodes mir instructions to x86_64 machine code

// code buffer for building instructions
typedef struct
{
    uint8_t *data;
    size_t   size;
    size_t   capacity;
} X86_64_CodeBuffer;

// register allocation map (virtual reg id -> physical reg)
typedef struct
{
    X86_64_Reg *map;
    size_t      count;
    size_t      capacity;
} X86_64_RegMap;

// code generation context
typedef struct
{
    X86_64_CodeBuffer code;
    X86_64_RegMap     reg_map;
} X86_64_CodegenContext;

// codegen operations
X86_64_CodegenContext *x86_64_codegen_create();
void                   x86_64_codegen_destroy(X86_64_CodegenContext *ctx);

// register allocation (simple linear scan for now)
int x86_64_allocate_registers(X86_64_CodegenContext *ctx, MIRFunction *func);

// code emission
int x86_64_emit_function(X86_64_CodegenContext *ctx, MIRFunction *func);

// get generated code
uint8_t *x86_64_codegen_get_code(X86_64_CodegenContext *ctx, size_t *size);

#endif // MIR_CODEGEN_X86_64_H
