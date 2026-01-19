#ifndef MASM_IR_H
#define MASM_IR_H

#include "compiler/masm/operand.h"
#include <stdint.h>
#include <stddef.h>

// portable masm ir opcodes
// these represent the three-operand form (tof) instruction set
// and are platform-independent.
typedef enum MasmIrOpcode
{
    // data movement
    MASM_IR_MOV = 1,
    MASM_IR_LOAD,
    MASM_IR_STORE,
    MASM_IR_LEA,
    MASM_IR_ZEXT,
    MASM_IR_SEXT,

    // integer arithmetic
    MASM_IR_ADD,
    MASM_IR_SUB,
    MASM_IR_MUL,
    MASM_IR_DIV,
    MASM_IR_DIVU,
    MASM_IR_REM,
    MASM_IR_REMU,
    MASM_IR_NEG,

    // bitwise operations
    MASM_IR_AND,
    MASM_IR_OR,
    MASM_IR_XOR,
    MASM_IR_NOT,
    MASM_IR_SHL,
    MASM_IR_SHR,
    MASM_IR_SAR,

    // comparisons (set-if)
    // results are stored in destination register (1 or 0)
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

    // floating point
    MASM_IR_FADD,
    MASM_IR_FSUB,
    MASM_IR_FMUL,
    MASM_IR_FDIV,
    MASM_IR_FCMP,
    MASM_IR_FCONV,

    // control flow
    MASM_IR_JMP,
    MASM_IR_BEQ,
    MASM_IR_BNE,
    MASM_IR_BLT,
    MASM_IR_BLTU,
    MASM_IR_BGE,
    MASM_IR_BGEU,
    MASM_IR_CALL,
    MASM_IR_RET,

    // system
    MASM_IR_SYSCALL,

    // pseudo-ops
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

// opcode namespace discriminator
typedef enum MasmOpcodeKind
{
    MASM_OPCODE_IR = 0,   // portable IR opcodes (MasmIrOpcode)
    MASM_OPCODE_X86,      // x86_64 target opcodes (MasmX86Opcode)
    // future: MASM_OPCODE_ARM64, etc.
} MasmOpcodeKind;

// instruction structure
// used for both IR instructions and target-specific instructions
typedef struct MasmInstruction
{
    MasmOpcodeKind kind;          // which opcode namespace
    uint32_t       opcode;        // interpretation depends on kind
    MasmOperand   *operands;
    uint8_t        operand_count;
} MasmInstruction;

// instruction builders (default to IR opcode kind)
MasmInstruction masm_inst_create(MasmOpcodeKind kind, uint32_t opcode, MasmOperand *operands, uint8_t count);
MasmInstruction masm_inst_0(uint32_t opcode);
MasmInstruction masm_inst_1(uint32_t opcode, MasmOperand op1);
MasmInstruction masm_inst_2(uint32_t opcode, MasmOperand op1, MasmOperand op2);
MasmInstruction masm_inst_3(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3);
MasmInstruction masm_inst_4(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4);
void            masm_inst_destroy(MasmInstruction inst);

// target-specific instruction builders (for use by isel)
MasmInstruction masm_inst_target_create(MasmOpcodeKind kind, uint32_t opcode, MasmOperand *operands, uint8_t count);
MasmInstruction masm_inst_target_0(MasmOpcodeKind kind, uint32_t opcode);
MasmInstruction masm_inst_target_1(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1);
MasmInstruction masm_inst_target_2(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2);
MasmInstruction masm_inst_target_3(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3);
MasmInstruction masm_inst_target_4(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4);

// helper to get string representation of ir opcode
const char *masm_ir_name(MasmIrOpcode op);

#endif // MASM_IR_H
