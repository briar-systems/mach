#include "compiler/masm/ir.h"

const char *masm_ir_name(MasmIrOpcode op)
{
    switch (op)
    {
        // Data Movement
        case MASM_IR_MOV: return "mov";
        case MASM_IR_LOAD: return "load";
        case MASM_IR_STORE: return "store";
        case MASM_IR_LEA: return "lea";
        case MASM_IR_ZEXT: return "zext";
        case MASM_IR_SEXT: return "sext";

        // Integer Arithmetic
        case MASM_IR_ADD: return "add";
        case MASM_IR_SUB: return "sub";
        case MASM_IR_MUL: return "mul";
        case MASM_IR_DIV: return "div";
        case MASM_IR_DIVU: return "divu";
        case MASM_IR_REM: return "rem";
        case MASM_IR_REMU: return "remu";
        case MASM_IR_NEG: return "neg";

        // Bitwise Operations
        case MASM_IR_AND: return "and";
        case MASM_IR_OR: return "or";
        case MASM_IR_XOR: return "xor";
        case MASM_IR_NOT: return "not";
        case MASM_IR_SHL: return "shl";
        case MASM_IR_SHR: return "shr";
        case MASM_IR_SAR: return "sar";

        // Comparisons (Set-if)
        case MASM_IR_CMP: return "cmp";
        case MASM_IR_SEQ: return "seq";
        case MASM_IR_SNE: return "sne";
        case MASM_IR_SLT: return "slt";
        case MASM_IR_SLTU: return "sltu";
        case MASM_IR_SLE: return "sle";
        case MASM_IR_SLEU: return "sleu";
        case MASM_IR_SGT: return "sgt";
        case MASM_IR_SGTU: return "sgtu";
        case MASM_IR_SGE: return "sge";
        case MASM_IR_SGEU: return "sgeu";

        // Floating Point
        case MASM_IR_FADD: return "fadd";
        case MASM_IR_FSUB: return "fsub";
        case MASM_IR_FMUL: return "fmul";
        case MASM_IR_FDIV: return "fdiv";
        case MASM_IR_FCMP: return "fcmp";
        case MASM_IR_FCONV: return "fconv";

        // Control Flow
        case MASM_IR_JMP: return "jmp";
        case MASM_IR_BEQ: return "beq";
        case MASM_IR_BNE: return "bne";
        case MASM_IR_BLT: return "blt";
        case MASM_IR_BLTU: return "bltu";
        case MASM_IR_BGE: return "bge";
        case MASM_IR_BGEU: return "bgeu";
        case MASM_IR_CALL: return "call";
        case MASM_IR_RET: return "ret";

        // System
        case MASM_IR_SYSCALL: return "syscall";

        // Pseudo-Ops
        case MASM_IR_LABEL: return "label";
        case MASM_IR_DATA: return "data";
        case MASM_IR_STACK_FRAME: return "stack_frame";

        default: return "unknown";
    }
}
