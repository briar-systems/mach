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

void masm_inst_destroy(MasmInstruction inst)
{
    if (inst.operands)
    {
        free(inst.operands);
    }
}
