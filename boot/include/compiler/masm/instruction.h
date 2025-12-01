#ifndef MASM_INSTRUCTION_H
#define MASM_INSTRUCTION_H

#include "compiler/masm/operand.h"
#include <stdint.h>
#include <stddef.h>

// generic opcodes
typedef enum MasmOpcode
{
    // data movement
    MASM_OP_MOV,
    MASM_OP_LEA,
    
    // arithmetic
    MASM_OP_ADD,
    MASM_OP_SUB,
    MASM_OP_MUL,
    MASM_OP_DIV,
    MASM_OP_AND,
    MASM_OP_OR,
    MASM_OP_XOR,
    MASM_OP_SHL,
    MASM_OP_SHR,
    
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
    
    // comparison
    MASM_OP_CMP,
    MASM_OP_TEST,
    
    // stack
    MASM_OP_PUSH,
    MASM_OP_POP,
    
    // system
    MASM_OP_SYSCALL,
    
    // pseudo-ops
    MASM_OP_LABEL,
    
    // target specific start
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
void            masm_inst_destroy(MasmInstruction inst);

#endif // MASM_INSTRUCTION_H
