#include "compiler/mir/inst.h"
#include <stdlib.h>
#include <string.h>

MIRInst *mir_inst_create(MIROp op, MIRType *type)
{
    MIRInst *inst = malloc(sizeof(MIRInst));
    if (!inst)
    {
        return NULL;
    }

    inst->op = op;
    inst->type = type;
    inst->result = NULL;
    inst->operands = NULL;
    inst->operand_count = 0;
    inst->operand_capacity = 0;
    inst->next = NULL;

    return inst;
}

void mir_inst_destroy(MIRInst *inst)
{
    if (!inst)
    {
        return;
    }

    if (inst->operands)
    {
        free(inst->operands);
    }

    free(inst);
}

void mir_inst_add_operand(MIRInst *inst, MIROperand operand)
{
    if (!inst)
    {
        return;
    }

    if (inst->operand_count >= inst->operand_capacity)
    {
        size_t new_capacity = inst->operand_capacity == 0 ? 4 : inst->operand_capacity * 2;
        MIROperand *new_operands = realloc(inst->operands, new_capacity * sizeof(MIROperand));
        if (!new_operands)
        {
            return;
        }
        inst->operands = new_operands;
        inst->operand_capacity = new_capacity;
    }

    inst->operands[inst->operand_count++] = operand;
}

void mir_inst_set_result(MIRInst *inst, MIRValue *result)
{
    if (!inst)
    {
        return;
    }

    inst->result = result;
    if (result)
    {
        result->def_inst = inst;
    }
}

MIRInst *mir_inst_const(MIRType *type, int64_t value)
{
    MIRInst *inst = mir_inst_create(MIR_OP_CONST, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, mir_operand_imm_int(value));
    return inst;
}

MIRInst *mir_inst_const_float(MIRType *type, double value)
{
    MIRInst *inst = mir_inst_create(MIR_OP_CONST, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, mir_operand_imm_flt(value));
    return inst;
}

MIRInst *mir_inst_binary(MIROp op, MIRType *type, MIROperand left, MIROperand right)
{
    MIRInst *inst = mir_inst_create(op, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, left);
    mir_inst_add_operand(inst, right);
    return inst;
}

MIRInst *mir_inst_unary(MIROp op, MIRType *type, MIROperand operand)
{
    MIRInst *inst = mir_inst_create(op, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, operand);
    return inst;
}

MIRInst *mir_inst_load(MIRType *type, MIROperand ptr)
{
    MIRInst *inst = mir_inst_create(MIR_OP_LOAD, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, ptr);
    return inst;
}

MIRInst *mir_inst_store(MIRType *type, MIROperand ptr, MIROperand value)
{
    MIRInst *inst = mir_inst_create(MIR_OP_STORE, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, ptr);
    mir_inst_add_operand(inst, value);
    return inst;
}

MIRInst *mir_inst_gep(MIRType *type, MIROperand ptr, MIROperand offset)
{
    MIRInst *inst = mir_inst_create(MIR_OP_GEP, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, ptr);
    mir_inst_add_operand(inst, offset);
    return inst;
}

MIRInst *mir_inst_call(MIRType *return_type, const char *func_name, MIROperand *args, size_t arg_count)
{
    MIRInst *inst = mir_inst_create(MIR_OP_CALL, return_type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, mir_operand_global(func_name));

    for (size_t i = 0; i < arg_count; i++)
    {
        mir_inst_add_operand(inst, args[i]);
    }

    return inst;
}

MIRInst *mir_inst_ret(MIRType *type, MIROperand value)
{
    MIRInst *inst = mir_inst_create(MIR_OP_RET, type);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, value);
    return inst;
}

MIRInst *mir_inst_ret_void()
{
    return mir_inst_create(MIR_OP_RET, NULL);
}

MIRInst *mir_inst_br(uint32_t target_block)
{
    MIRInst *inst = mir_inst_create(MIR_OP_BR, NULL);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, mir_operand_block(target_block));
    return inst;
}

MIRInst *mir_inst_brcond(MIROperand cond, uint32_t true_block, uint32_t false_block)
{
    MIRInst *inst = mir_inst_create(MIR_OP_BRCOND, NULL);
    if (!inst)
    {
        return NULL;
    }

    mir_inst_add_operand(inst, cond);
    mir_inst_add_operand(inst, mir_operand_block(true_block));
    mir_inst_add_operand(inst, mir_operand_block(false_block));
    return inst;
}
