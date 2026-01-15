#ifndef MASM_ISA_X86_64_H
#define MASM_ISA_X86_64_H

#include "compiler/masm/operand.h"
#include "compiler/masm/instruction.h"

// x86_64-specific opcodes live in the target-specific range
typedef enum MasmX86Opcode
{
    MASM_OP_X86_SYSCALL = MASM_OP_TARGET_SPECIFIC_START,

    // sse moves between xmm regs (encoded as masm_operand_register(xmm_index, 16))
    // and r/m32|r/m64 using MOVD/MOVQ encodings.
    MASM_OP_X86_MOVQ,
} MasmX86Opcode;

// x86_64 registers
typedef enum MasmX86Reg
{
    MASM_X86_RAX,
    MASM_X86_RCX,
    MASM_X86_RDX,
    MASM_X86_RBX,
    MASM_X86_RSP,
    MASM_X86_RBP,
    MASM_X86_RSI,
    MASM_X86_RDI,
    MASM_X86_R8,
    MASM_X86_R9,
    MASM_X86_R10,
    MASM_X86_R11,
    MASM_X86_R12,
    MASM_X86_R13,
    MASM_X86_R14,
    MASM_X86_R15,
    MASM_X86_REG_COUNT
} MasmX86Reg;

// register accessors
MasmOperand masm_x86_reg(MasmX86Reg reg, uint8_t size);

// instruction encoding (placeholder)
int masm_x86_encode(MasmInstruction inst, uint8_t *buffer, size_t size);

#endif // MASM_ISA_X86_64_H
