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

// relocation request
typedef struct X86_64_Relocation
{
    uint64_t offset;      // offset in code where relocation is needed
    char    *symbol_name; // symbol to relocate to
    int      type;        // relocation type (ELF R_X86_64_*)
    struct X86_64_Relocation *next;
} X86_64_Relocation;

// block offset map
typedef struct
{
    MIRBlock *block;
    size_t    offset;
} BlockOffset;

typedef struct
{
    BlockOffset *items;
    size_t       count;
    size_t       capacity;
} BlockOffsetMap;

// pending jump for backpatching
typedef struct PendingJump
{
    size_t    disp_offset; // offset of the 4-byte displacement field
    MIRBlock *target;      // target block
    struct PendingJump *next;
} PendingJump;

// code generation context
typedef struct
{
    X86_64_CodeBuffer    code;
    X86_64_RegMap        reg_map;
    X86_64_Relocation   *relocations; // linked list of relocations
    BlockOffsetMap       block_offsets;
    PendingJump         *pending_jumps;
    MIRFunction         *current_function; // current function being emitted
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

// get relocations
X86_64_Relocation *x86_64_codegen_get_relocations(X86_64_CodegenContext *ctx);

#endif // MIR_CODEGEN_X86_64_H
