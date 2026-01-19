#include "compiler/masm/ir.h"
#include <stdlib.h>
#include <string.h>

// instruction builders

MasmInstruction masm_inst_create(MasmOpcodeKind kind, uint32_t opcode, MasmOperand *operands, uint8_t count)
{
    MasmInstruction inst;
    inst.kind = kind;
    inst.opcode = opcode;
    inst.operand_count = count;

    if (count > 0)
    {
        inst.operands = malloc(sizeof(MasmOperand) * count);
        memcpy(inst.operands, operands, sizeof(MasmOperand) * count);

        // deep copy strings to avoid ownership issues
        for (int i = 0; i < count; i++)
        {
            if (inst.operands[i].kind == MASM_OPERAND_LABEL && inst.operands[i].label)
            {
                inst.operands[i].label = strdup(inst.operands[i].label);
            }
            else if (inst.operands[i].kind == MASM_OPERAND_SYMBOL && inst.operands[i].symbol)
            {
                inst.operands[i].symbol = strdup(inst.operands[i].symbol);
            }
        }
    }
    else
    {
        inst.operands = NULL;
    }

    return inst;
}

// ir instruction builders (default to MASM_OPCODE_IR)

MasmInstruction masm_inst_0(uint32_t opcode)
{
    return masm_inst_create(MASM_OPCODE_IR, opcode, NULL, 0);
}

MasmInstruction masm_inst_1(uint32_t opcode, MasmOperand op1)
{
    return masm_inst_create(MASM_OPCODE_IR, opcode, &op1, 1);
}

MasmInstruction masm_inst_2(uint32_t opcode, MasmOperand op1, MasmOperand op2)
{
    MasmOperand ops[] = {op1, op2};
    return masm_inst_create(MASM_OPCODE_IR, opcode, ops, 2);
}

MasmInstruction masm_inst_3(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3)
{
    MasmOperand ops[] = {op1, op2, op3};
    return masm_inst_create(MASM_OPCODE_IR, opcode, ops, 3);
}

MasmInstruction masm_inst_4(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4)
{
    MasmOperand ops[] = {op1, op2, op3, op4};
    return masm_inst_create(MASM_OPCODE_IR, opcode, ops, 4);
}

// target-specific instruction builders

MasmInstruction masm_inst_target_create(MasmOpcodeKind kind, uint32_t opcode, MasmOperand *operands, uint8_t count)
{
    return masm_inst_create(kind, opcode, operands, count);
}

MasmInstruction masm_inst_target_0(MasmOpcodeKind kind, uint32_t opcode)
{
    return masm_inst_create(kind, opcode, NULL, 0);
}

MasmInstruction masm_inst_target_1(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1)
{
    return masm_inst_create(kind, opcode, &op1, 1);
}

MasmInstruction masm_inst_target_2(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2)
{
    MasmOperand ops[] = {op1, op2};
    return masm_inst_create(kind, opcode, ops, 2);
}

MasmInstruction masm_inst_target_3(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3)
{
    MasmOperand ops[] = {op1, op2, op3};
    return masm_inst_create(kind, opcode, ops, 3);
}

MasmInstruction masm_inst_target_4(MasmOpcodeKind kind, uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4)
{
    MasmOperand ops[] = {op1, op2, op3, op4};
    return masm_inst_create(kind, opcode, ops, 4);
}

void masm_inst_destroy(MasmInstruction inst)
{
    if (inst.operands)
    {
        for (int i = 0; i < inst.operand_count; i++)
        {
            if (inst.operands[i].kind == MASM_OPERAND_LABEL && inst.operands[i].label)
            {
                free((void *)inst.operands[i].label);
            }
            else if (inst.operands[i].kind == MASM_OPERAND_SYMBOL && inst.operands[i].symbol)
            {
                free((void *)inst.operands[i].symbol);
            }
        }
        free(inst.operands);
    }
}

// ir opcode names

const char *masm_ir_name(MasmIrOpcode op)
{
    switch (op)
    {
    // data movement
    case MASM_IR_MOV:
        return "mov";
    case MASM_IR_LOAD:
        return "load";
    case MASM_IR_STORE:
        return "store";
    case MASM_IR_LEA:
        return "lea";
    case MASM_IR_ZEXT:
        return "zext";
    case MASM_IR_SEXT:
        return "sext";

    // integer arithmetic
    case MASM_IR_ADD:
        return "add";
    case MASM_IR_SUB:
        return "sub";
    case MASM_IR_MUL:
        return "mul";
    case MASM_IR_DIV:
        return "div";
    case MASM_IR_DIVU:
        return "divu";
    case MASM_IR_REM:
        return "rem";
    case MASM_IR_REMU:
        return "remu";
    case MASM_IR_NEG:
        return "neg";

    // bitwise operations
    case MASM_IR_AND:
        return "and";
    case MASM_IR_OR:
        return "or";
    case MASM_IR_XOR:
        return "xor";
    case MASM_IR_NOT:
        return "not";
    case MASM_IR_SHL:
        return "shl";
    case MASM_IR_SHR:
        return "shr";
    case MASM_IR_SAR:
        return "sar";

    // comparisons (set-if)
    case MASM_IR_SEQ:
        return "seq";
    case MASM_IR_SNE:
        return "sne";
    case MASM_IR_SLT:
        return "slt";
    case MASM_IR_SLTU:
        return "sltu";
    case MASM_IR_SLE:
        return "sle";
    case MASM_IR_SLEU:
        return "sleu";
    case MASM_IR_SGT:
        return "sgt";
    case MASM_IR_SGTU:
        return "sgtu";
    case MASM_IR_SGE:
        return "sge";
    case MASM_IR_SGEU:
        return "sgeu";

    // floating point
    case MASM_IR_FADD:
        return "fadd";
    case MASM_IR_FSUB:
        return "fsub";
    case MASM_IR_FMUL:
        return "fmul";
    case MASM_IR_FDIV:
        return "fdiv";
    case MASM_IR_FCMP:
        return "fcmp";
    case MASM_IR_FCONV:
        return "fconv";

    // control flow
    case MASM_IR_JMP:
        return "jmp";
    case MASM_IR_BEQ:
        return "beq";
    case MASM_IR_BNE:
        return "bne";
    case MASM_IR_BLT:
        return "blt";
    case MASM_IR_BLTU:
        return "bltu";
    case MASM_IR_BGE:
        return "bge";
    case MASM_IR_BGEU:
        return "bgeu";
    case MASM_IR_CALL:
        return "call";
    case MASM_IR_RET:
        return "ret";

    // system
    case MASM_IR_SYSCALL:
        return "syscall";

    // pseudo-ops
    case MASM_IR_LABEL:
        return "label";
    case MASM_IR_DATA:
        return "data";
    case MASM_IR_STACK_FRAME:
        return "stack_frame";

    default:
        return "unknown";
    }
}
