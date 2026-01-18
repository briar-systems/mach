#ifndef MASM_ISA_X86_64_H
#define MASM_ISA_X86_64_H

#include "compiler/masm/operand.h"
#include "compiler/masm/instruction.h"

// x86_64-specific opcodes live in the target-specific range
typedef enum MasmX86Opcode
{
    MASM_OP_X86_SYSCALL = MASM_OP_TARGET_SPECIFIC_START,

    // Data Movement
    MASM_OP_X86_MOV_RR,
    MASM_OP_X86_MOV_RM,
    MASM_OP_X86_MOV_MR,
    MASM_OP_X86_MOV_RI,
    MASM_OP_X86_MOV_MI,
    MASM_OP_X86_MOVSX_RR,
    MASM_OP_X86_MOVSX_RM,
    MASM_OP_X86_MOVZX_RR,
    MASM_OP_X86_MOVZX_RM,
    MASM_OP_X86_LEA,

    // Arithmetic
    MASM_OP_X86_ADD_RR, MASM_OP_X86_ADD_RM, MASM_OP_X86_ADD_MR, MASM_OP_X86_ADD_RI, MASM_OP_X86_ADD_MI,
    MASM_OP_X86_SUB_RR, MASM_OP_X86_SUB_RM, MASM_OP_X86_SUB_MR, MASM_OP_X86_SUB_RI, MASM_OP_X86_SUB_MI,
    MASM_OP_X86_AND_RR, MASM_OP_X86_AND_RM, MASM_OP_X86_AND_MR, MASM_OP_X86_AND_RI, MASM_OP_X86_AND_MI,
    MASM_OP_X86_OR_RR,  MASM_OP_X86_OR_RM,  MASM_OP_X86_OR_MR,  MASM_OP_X86_OR_RI,  MASM_OP_X86_OR_MI,
    MASM_OP_X86_XOR_RR, MASM_OP_X86_XOR_RM, MASM_OP_X86_XOR_MR, MASM_OP_X86_XOR_RI, MASM_OP_X86_XOR_MI,
    MASM_OP_X86_IMUL_RR, MASM_OP_X86_IMUL_RM, MASM_OP_X86_IMUL_RRI, MASM_OP_X86_IMUL_RMI,
    MASM_OP_X86_DIV, MASM_OP_X86_IDIV,

    // Shifts
    MASM_OP_X86_SHL_RI, MASM_OP_X86_SHL_RC,
    MASM_OP_X86_SHR_RI, MASM_OP_X86_SHR_RC,
    MASM_OP_X86_SAR_RI, MASM_OP_X86_SAR_RC,

    // Comparison
    MASM_OP_X86_CMP_RR, MASM_OP_X86_CMP_RM, MASM_OP_X86_CMP_MR, MASM_OP_X86_CMP_RI, MASM_OP_X86_CMP_MI,
    MASM_OP_X86_TEST_RR, MASM_OP_X86_TEST_RM, MASM_OP_X86_TEST_MR, MASM_OP_X86_TEST_RI, MASM_OP_X86_TEST_MI,

    // SetCC
    MASM_OP_X86_SETE, MASM_OP_X86_SETNE,
    MASM_OP_X86_SETL, MASM_OP_X86_SETG,
    MASM_OP_X86_SETLE, MASM_OP_X86_SETGE,
    MASM_OP_X86_SETB, MASM_OP_X86_SETA,
    MASM_OP_X86_SETBE, MASM_OP_X86_SETAE,

    // Stack
    MASM_OP_X86_PUSH_R, MASM_OP_X86_PUSH_M, MASM_OP_X86_PUSH_I,
    MASM_OP_X86_POP_R,  MASM_OP_X86_POP_M,

    // Control Flow
    MASM_OP_X86_JMP_REL, MASM_OP_X86_JMP_RM,
    MASM_OP_X86_JE, MASM_OP_X86_JNE,
    MASM_OP_X86_JL, MASM_OP_X86_JG,
    MASM_OP_X86_JLE, MASM_OP_X86_JGE,
    MASM_OP_X86_CALL_REL, MASM_OP_X86_CALL_RM,
    MASM_OP_X86_RET,

    // sse moves between xmm regs (encoded as masm_operand_register(xmm_index, 16))
    // and r/m32|r/m64 using MOVD/MOVQ encodings.
    MASM_OP_X86_MOVQ,

    // sse comparison: ucomisd xmm1, xmm2/m64
    // sets EFLAGS (ZF, PF, CF) based on unordered comparison
    MASM_OP_X86_UCOMISD,

    // sse arithmetic (scalar double-precision)
    MASM_OP_X86_ADDSD,  // addsd xmm1, xmm2/m64
    MASM_OP_X86_SUBSD,  // subsd xmm1, xmm2/m64
    MASM_OP_X86_MULSD,  // mulsd xmm1, xmm2/m64
    MASM_OP_X86_DIVSD,  // divsd xmm1, xmm2/m64

    // sse conversions
    MASM_OP_X86_CVTSI2SD,  // cvtsi2sd xmm, r/m32|r/m64
    MASM_OP_X86_CVTSI2SS,  // cvtsi2ss xmm, r/m32|r/m64
    MASM_OP_X86_CVTTSD2SI, // cvttsd2si r32|r64, xmm/m64
    MASM_OP_X86_CVTTSS2SI, // cvttss2si r32|r64, xmm/m32
    MASM_OP_X86_CVTSD2SS,  // cvtsd2ss xmm1, xmm2/m64
    MASM_OP_X86_CVTSS2SD,  // cvtss2sd xmm1, xmm2/m32

    // sign extension
    MASM_OP_X86_CBW, // AL -> AX
    MASM_OP_X86_CWD, // AX -> DX:AX
    MASM_OP_X86_CDQ, // EAX -> EDX:EAX
    MASM_OP_X86_CQO, // RAX -> RDX:RAX
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
