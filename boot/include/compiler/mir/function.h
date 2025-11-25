#ifndef MIR_FUNCTION_H
#define MIR_FUNCTION_H

#include "compiler/mir/block.h"
#include "compiler/mir/value.h"
#include "compiler/type.h"
#include <stdbool.h>
#include <stddef.h>

// mir function (ssa function)
typedef struct MIRFunction
{
    char       *name;              // function name
    Type       *type;              // function type
    bool        is_exported;       // pub function
    MIRValue  **params;            // parameter values
    size_t      param_count;
    size_t      param_capacity;
    MIRBlock   *first_block;       // entry block
    MIRBlock   *last_block;        // last block (for O(1) append)
    size_t      block_count;
    uint32_t    next_value_id;     // value counter
    uint32_t    next_block_id;     // block counter
    size_t      frame_size;        // stack frame size in bytes
    struct MIRFunction *next;      // linked list within module
} MIRFunction;

// function management
MIRFunction *mir_function_create(const char *name, Type *type, bool is_exported);
void         mir_function_destroy(MIRFunction *func);

// block operations
MIRBlock *mir_function_add_block(MIRFunction *func, const char *label);
MIRBlock *mir_function_get_entry_block(MIRFunction *func);

// value operations
MIRValue *mir_function_alloc_value(MIRFunction *func, Type *type, const char *name);
MIRValue *mir_function_add_param(MIRFunction *func, Type *type, const char *name);

#endif // MIR_FUNCTION_H
