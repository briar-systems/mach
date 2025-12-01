#include "compiler/masm/operand.h"
#include <string.h>

MasmOperand masm_operand_none()
{
    MasmOperand op;
    memset(&op, 0, sizeof(MasmOperand));
    op.kind = MASM_OPERAND_NONE;
    return op;
}

MasmOperand masm_operand_register(uint32_t id, uint8_t size)
{
    MasmOperand op;
    memset(&op, 0, sizeof(MasmOperand));
    op.kind = MASM_OPERAND_REGISTER;
    op.reg.id = id;
    op.reg.size = size;
    return op;
}

MasmOperand masm_operand_imm(int64_t value)
{
    MasmOperand op;
    memset(&op, 0, sizeof(MasmOperand));
    op.kind = MASM_OPERAND_IMM;
    op.imm = value;
    return op;
}

MasmOperand masm_operand_memory(MasmRegister base, MasmRegister index, uint8_t scale, int64_t disp, uint8_t size)
{
    MasmOperand op;
    memset(&op, 0, sizeof(MasmOperand));
    op.kind = MASM_OPERAND_MEMORY;
    op.mem.base = base;
    op.mem.index = index;
    op.mem.scale = scale;
    op.mem.disp = disp;
    op.mem.size = size;
    return op;
}

MasmOperand masm_operand_symbol(const char *name)
{
    MasmOperand op;
    op.kind   = MASM_OPERAND_SYMBOL;
    op.symbol = name;
    return op;
}

MasmOperand masm_operand_label(const char *name)
{
    MasmOperand op;
    op.kind  = MASM_OPERAND_LABEL;
    op.label = name;
    return op;
}

// simplified memory operand for stack variables: [base + disp]
MasmOperand masm_operand_memory_simple(uint32_t base_reg, int32_t disp, uint8_t size)
{
    MasmOperand op;
    op.kind = MASM_OPERAND_MEMORY;
    op.mem.base.id = base_reg;
    op.mem.base.size = 8;
    op.mem.index.id = 0;
    op.mem.index.size = 0;
    op.mem.scale = 0;
    op.mem.disp = disp;
    op.mem.size = size;
    return op;
}
