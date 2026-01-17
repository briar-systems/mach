#include "compiler/masm/instruction.h"
#include <stdlib.h>
#include <string.h>

MasmInstruction masm_inst_create(uint32_t opcode, MasmOperand *operands, uint8_t count)
{
    MasmInstruction inst;
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

MasmInstruction masm_inst_0(uint32_t opcode)
{
    return masm_inst_create(opcode, NULL, 0);
}

MasmInstruction masm_inst_1(uint32_t opcode, MasmOperand op1)
{
    return masm_inst_create(opcode, &op1, 1);
}

MasmInstruction masm_inst_2(uint32_t opcode, MasmOperand op1, MasmOperand op2)
{
    MasmOperand ops[] = {op1, op2};
    return masm_inst_create(opcode, ops, 2);
}

MasmInstruction masm_inst_3(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3)
{
    MasmOperand ops[] = {op1, op2, op3};
    return masm_inst_create(opcode, ops, 3);
}

MasmInstruction masm_inst_4(uint32_t opcode, MasmOperand op1, MasmOperand op2, MasmOperand op3, MasmOperand op4)
{
    MasmOperand ops[] = {op1, op2, op3, op4};
    return masm_inst_create(opcode, ops, 4);
}

void masm_inst_destroy(MasmInstruction inst)
{
    if (inst.operands)
    {
        for (int i = 0; i < inst.operand_count; i++)
        {
            if (inst.operands[i].kind == MASM_OPERAND_LABEL && inst.operands[i].label)
            {
                free((void*)inst.operands[i].label);
            }
            else if (inst.operands[i].kind == MASM_OPERAND_SYMBOL && inst.operands[i].symbol)
            {
                free((void*)inst.operands[i].symbol);
            }
        }
        free(inst.operands);
    }
}
