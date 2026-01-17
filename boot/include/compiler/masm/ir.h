#ifndef MASM_IR_H
#define MASM_IR_H

#include <stdint.h>

// Portable MASM IR Opcodes
// These opcodes represent the Three-Operand Form (TOF) instruction set
// and are platform-independent.
typedef enum MasmIrOpcode
{
    // Data Movement
    MASM_IR_MOV,
    MASM_IR_LOAD,
    MASM_IR_STORE,
    MASM_IR_LEA,

    // Integer Arithmetic
    MASM_IR_ADD,
    MASM_IR_SUB,
    MASM_IR_MUL,
    MASM_IR_DIV,
    MASM_IR_DIVU,
    MASM_IR_REM,
    MASM_IR_REMU,
    MASM_IR_NEG,

    // Bitwise Operations
    MASM_IR_AND,
    MASM_IR_OR,
    MASM_IR_XOR,
    MASM_IR_NOT,
    MASM_IR_SHL,
    MASM_IR_SHR,
    MASM_IR_SAR,

    // Comparisons (Set-if)
    // Results are stored in destination register (1 or 0)
    MASM_IR_SEQ,
    MASM_IR_SNE,
    MASM_IR_SLT,
    MASM_IR_SLTU,
    MASM_IR_SLE,
    MASM_IR_SLEU,
    MASM_IR_SGT,
    MASM_IR_SGTU,
    MASM_IR_SGE,
    MASM_IR_SGEU,

    // Floating Point
    MASM_IR_FADD,
    MASM_IR_FSUB,
    MASM_IR_FMUL,
    MASM_IR_FDIV,
    MASM_IR_FCMP,
    MASM_IR_FCONV,

    // Control Flow
    MASM_IR_JMP,
    MASM_IR_BEQ,
    MASM_IR_BNE,
    MASM_IR_BLT,
    MASM_IR_BLTU,
    MASM_IR_BGE,
    MASM_IR_BGEU,
    MASM_IR_CALL,
    MASM_IR_RET,

    // System
    MASM_IR_SYSCALL,

    // Pseudo-Ops
    MASM_IR_LABEL,
    MASM_IR_DATA,
    MASM_IR_STACK_FRAME,

    MASM_IR_COUNT
} MasmIrOpcode;

typedef enum MasmIrFcmpCond
{
    MASM_IR_FCMP_EQ,
    MASM_IR_FCMP_NE,
    MASM_IR_FCMP_LT,
    MASM_IR_FCMP_LE,
    MASM_IR_FCMP_GT,
    MASM_IR_FCMP_GE
} MasmIrFcmpCond;

// Helper to get string representation of IR opcode
const char *masm_ir_name(MasmIrOpcode op);

#endif // MASM_IR_H

