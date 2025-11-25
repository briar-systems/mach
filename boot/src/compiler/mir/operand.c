#include "compiler/mir/operand.h"

MIROperand mir_operand_none()
{
    MIROperand op;
    op.kind = MIR_OPERAND_NONE;
    return op;
}

MIROperand mir_operand_value(uint32_t value_id)
{
    MIROperand op;
    op.kind = MIR_OPERAND_VALUE;
    op.value_id = value_id;
    return op;
}

MIROperand mir_operand_imm_int(int64_t value)
{
    MIROperand op;
    op.kind = MIR_OPERAND_IMM_INT;
    op.imm_int = value;
    return op;
}

MIROperand mir_operand_imm_flt(double value)
{
    MIROperand op;
    op.kind = MIR_OPERAND_IMM_FLT;
    op.imm_flt = value;
    return op;
}

MIROperand mir_operand_global(const char *name)
{
    MIROperand op;
    op.kind = MIR_OPERAND_GLOBAL;
    op.global_name = name;
    return op;
}

MIROperand mir_operand_block(uint32_t block_id)
{
    MIROperand op;
    op.kind = MIR_OPERAND_BLOCK;
    op.block_id = block_id;
    return op;
}
