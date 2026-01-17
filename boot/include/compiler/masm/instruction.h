#ifndef MASM_INSTRUCTION_H
#define MASM_INSTRUCTION_H

#include "compiler/masm/operand.h"
#include "compiler/masm/ir.h"
#include <stdint.h>
#include <stddef.h>

// generic opcodes (portable across supported targets). ISA/OS-specific opcodes
// must be defined within their respective backend headers and use values at or
// beyond MASM_OP_TARGET_SPECIFIC_START.
typedef enum MasmOpcode
{
    // data movement
    MASM_OP_MOV,
    MASM_OP_MOVSX,
    MASM_OP_MOVZX,
    MASM_OP_LEA,
    
    // arithmetic
    MASM_OP_ADD,
    MASM_OP_SUB,
    MASM_OP_MUL,
    MASM_OP_IMUL,
    MASM_OP_DIV,
    MASM_OP_IDIV,
    MASM_OP_AND,
    MASM_OP_OR,
    MASM_OP_XOR,
    MASM_OP_SHL,
    MASM_OP_SHR,
    MASM_OP_SAR,
    
    // comparison and logical
    MASM_OP_CMP,
    MASM_OP_TEST,
    MASM_OP_SETE,
    MASM_OP_SETNE,
    MASM_OP_SETL,
    MASM_OP_SETG,
    MASM_OP_SETLE,
    MASM_OP_SETGE,
    MASM_OP_SETB,
    MASM_OP_SETA,
    MASM_OP_SETBE,
    MASM_OP_SETAE,
    
    // control flow
    MASM_OP_JMP,
    MASM_OP_JE,
    MASM_OP_JNE,
    MASM_OP_JG,
    MASM_OP_JGE,
    MASM_OP_JL,
    MASM_OP_JLE,
    MASM_OP_CALL,
    MASM_OP_RET,
    
    // special (generic)
    MASM_OP_PUSH,
    MASM_OP_POP,
    
    // pseudo-ops (generic, non-ISA)
    MASM_OP_LABEL,

    // target specific start — ISA/OS-specific opcodes live at or beyond this value
    MASM_OP_TARGET_SPECIFIC_START = 1000
} MasmOpcode;

// instruction structure
typedef struct MasmInstruction
{
    uint32_t     opcode; // MasmOpcode or target-specific
    MasmOperand *operands;
    uint8_t      operand_count;
} MasmInstruction;

// instruction builders
MasmInstruction masm_inst_create(uint32_t opcode, MasmOperand *operands, uint8_t count);
MasmInstruction masm_inst_0(uint32_t opcode);
MasmInstruction masm_inst_1(uint32_t opcode, MasmOperand op1);
MasmInstruction masm_inst_2(uint32_t opcode, MasmOperand op1, MasmOperand op2);
MasmInstruction masm_inst_3(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3);
MasmInstruction masm_inst_4(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4);
void            masm_inst_destroy(MasmInstruction inst);

#endif // MASM_INSTRUCTION_H
