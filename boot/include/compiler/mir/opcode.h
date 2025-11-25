#ifndef MIR_OPCODE_H
#define MIR_OPCODE_H

#include <stdbool.h>

// mir operation codes - architecture-agnostic ssa instructions

typedef enum MIROp
{
    // constants
    MIR_OP_CONST,

    // data movement
    MIR_OP_MOV,
    MIR_OP_LOAD,
    MIR_OP_STORE,
    MIR_OP_ADDR, // get address of stack slot
    MIR_OP_PHI,

    // arithmetic
    MIR_OP_ADD,
    MIR_OP_SUB,
    MIR_OP_MUL,
    MIR_OP_DIV,
    MIR_OP_UDIV,
    MIR_OP_MOD,
    MIR_OP_UMOD,
    MIR_OP_NEG,

    // bitwise
    MIR_OP_AND,
    MIR_OP_OR,
    MIR_OP_XOR,
    MIR_OP_NOT,
    MIR_OP_SHL,
    MIR_OP_SHR,
    MIR_OP_SAR,

    // comparison (produce u8: 0 or 1)
    MIR_OP_EQ,
    MIR_OP_NE,
    MIR_OP_LT,
    MIR_OP_LE,
    MIR_OP_GT,
    MIR_OP_GE,
    MIR_OP_ULT,
    MIR_OP_ULE,
    MIR_OP_UGT,
    MIR_OP_UGE,

    // type conversions
    MIR_OP_ZEXT,
    MIR_OP_SEXT,
    MIR_OP_TRUNC,
    MIR_OP_CAST,

    // memory
    MIR_OP_GEP, // get element pointer (pointer arithmetic)

    // control flow (terminators)
    MIR_OP_RET,
    MIR_OP_BR,
    MIR_OP_BRCOND,
    MIR_OP_CALL,
    MIR_OP_UNREACHABLE,

    // platform intrinsics (generic, lowered by os/isa layers)
    MIR_OP_SYSCALL,

    MIR_OP_COUNT
} MIROp;

// opcode utilities
const char *mir_op_to_string(MIROp op);
bool        mir_op_is_terminator(MIROp op);
bool        mir_op_is_comparison(MIROp op);
bool        mir_op_is_binary(MIROp op);
bool        mir_op_is_unary(MIROp op);

#endif // MIR_OPCODE_H
