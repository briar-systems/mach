#include "mir.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Module Management
// ============================================================================

MirModule *mir_module_create(const char *name)
{
    MirModule *module = malloc(sizeof(MirModule));
    if (!module)
        return NULL;

    module->name         = name ? strdup(name) : NULL;
    module->data         = NULL;
    module->last_data    = NULL;
    module->blocks       = NULL;
    module->last_block   = NULL;
    module->next_vreg_id = 0;
    module->next_label_id = 0;

    return module;
}

void mir_module_destroy(MirModule *module)
{
    if (!module)
        return;

    // free data declarations
    MirData *data = module->data;
    while (data)
    {
        MirData *next = data->next;
        if (data->name)
            free((void *)data->name);
        if (data->kind == MIR_DATA_VAL && data->init_value.string_value)
            free((void *)data->init_value.string_value);
        if (data->kind == MIR_DATA_VAR && data->init_value.array_int.values)
            free(data->init_value.array_int.values);
        if (data->kind == MIR_DATA_VAR && data->init_value.array_float.values)
            free(data->init_value.array_float.values);
        free(data);
        data = next;
    }

    // free basic blocks
    MirBasicBlock *block = module->blocks;
    while (block)
    {
        MirBasicBlock *next = block->next;
        mir_block_destroy(block);
        block = next;
    }

    if (module->name)
        free((void *)module->name);
    free(module);
}

MirBasicBlock *mir_module_add_block(MirModule *module, const char *label, bool is_exported)
{
    if (!module)
        return NULL;

    MirBasicBlock *block = mir_block_create(label, is_exported);
    if (!block)
        return NULL;

    // append to end
    if (module->last_block)
    {
        module->last_block->next = block;
        module->last_block       = block;
    }
    else
    {
        module->blocks     = block;
        module->last_block = block;
    }

    return block;
}

MirData *mir_module_add_data(MirModule *module, MirDataKind kind, const char *name, Type *type)
{
    if (!module || !name)
        return NULL;

    MirData *data = malloc(sizeof(MirData));
    if (!data)
        return NULL;

    memset(data, 0, sizeof(MirData));
    data->kind = kind;
    data->name = strdup(name);
    data->type = type;
    data->next = NULL;

    // append to end
    if (module->last_data)
    {
        module->last_data->next = data;
        module->last_data       = data;
    }
    else
    {
        module->data      = data;
        module->last_data = data;
    }

    return data;
}

uint32_t mir_module_alloc_vreg(MirModule *module)
{
    if (!module)
        return 0;
    return module->next_vreg_id++;
}

const char *mir_module_gen_label(MirModule *module, const char *prefix)
{
    if (!module)
        return NULL;

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s%u", prefix ? prefix : ".L", module->next_label_id++);
    return strdup(buffer);
}

// ============================================================================
// Basic Block Management
// ============================================================================

MirBasicBlock *mir_block_create(const char *label, bool is_exported)
{
    MirBasicBlock *block = malloc(sizeof(MirBasicBlock));
    if (!block)
        return NULL;

    block->label            = label ? strdup(label) : NULL;
    block->is_exported      = is_exported;
    block->instructions     = NULL;
    block->last_instruction = NULL;
    block->instruction_count = 0;
    block->next             = NULL;

    return block;
}

void mir_block_destroy(MirBasicBlock *block)
{
    if (!block)
        return;

    // free instructions
    MirInstruction *inst = block->instructions;
    while (inst)
    {
        MirInstruction *next = inst->next;
        mir_inst_destroy(inst);
        inst = next;
    }

    if (block->label)
        free((void *)block->label);
    free(block);
}

void mir_block_add_instruction(MirBasicBlock *block, MirInstruction *inst)
{
    if (!block || !inst)
        return;

    // append to end
    if (block->last_instruction)
    {
        block->last_instruction->next = inst;
        block->last_instruction       = inst;
    }
    else
    {
        block->instructions     = inst;
        block->last_instruction = inst;
    }

    block->instruction_count++;
}

// ============================================================================
// Instruction Builders
// ============================================================================

MirInstruction *mir_inst_create_0op(MirOpcode opcode)
{
    MirInstruction *inst = malloc(sizeof(MirInstruction));
    if (!inst)
        return NULL;

    memset(inst, 0, sizeof(MirInstruction));
    inst->opcode        = opcode;
    inst->type          = NULL;
    inst->type2         = NULL;
    inst->operand_count = 0;
    inst->next          = NULL;

    return inst;
}

MirInstruction *mir_inst_create_1op(MirOpcode opcode, Type *type, MirOperand dst)
{
    MirInstruction *inst = malloc(sizeof(MirInstruction));
    if (!inst)
        return NULL;

    memset(inst, 0, sizeof(MirInstruction));
    inst->opcode        = opcode;
    inst->type          = type;
    inst->type2         = NULL;
    inst->operands[0]   = dst;
    inst->operand_count = 1;
    inst->next          = NULL;

    return inst;
}

MirInstruction *mir_inst_create_2op(MirOpcode opcode, Type *type, MirOperand dst, MirOperand src)
{
    MirInstruction *inst = malloc(sizeof(MirInstruction));
    if (!inst)
        return NULL;

    memset(inst, 0, sizeof(MirInstruction));
    inst->opcode        = opcode;
    inst->type          = type;
    inst->type2         = NULL;
    inst->operands[0]   = dst;
    inst->operands[1]   = src;
    inst->operand_count = 2;
    inst->next          = NULL;

    return inst;
}

MirInstruction *mir_inst_create_3op(MirOpcode opcode, Type *type, MirOperand dst, MirOperand src1, MirOperand src2)
{
    MirInstruction *inst = malloc(sizeof(MirInstruction));
    if (!inst)
        return NULL;

    memset(inst, 0, sizeof(MirInstruction));
    inst->opcode        = opcode;
    inst->type          = type;
    inst->type2         = NULL;
    inst->operands[0]   = dst;
    inst->operands[1]   = src1;
    inst->operands[2]   = src2;
    inst->operand_count = 3;
    inst->next          = NULL;

    return inst;
}

void mir_inst_destroy(MirInstruction *inst)
{
    if (!inst)
        return;

    // free label strings in operands
    for (size_t i = 0; i < inst->operand_count; i++)
    {
        if (inst->operands[i].kind == MIR_OPERAND_LABEL && inst->operands[i].label)
            free((void *)inst->operands[i].label);
    }

    free(inst);
}

// ============================================================================
// Operand Builders
// ============================================================================

MirOperand mir_operand_none(void)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind = MIR_OPERAND_NONE;
    return op;
}

MirOperand mir_operand_vreg(uint32_t id)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind     = MIR_OPERAND_VREG;
    op.vreg_id  = id;
    return op;
}

MirOperand mir_operand_preg(MirPhysicalReg preg)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind = MIR_OPERAND_PREG;
    op.preg = preg;
    return op;
}

MirOperand mir_operand_imm(int64_t value)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind      = MIR_OPERAND_IMM;
    op.immediate = value;
    return op;
}

MirOperand mir_operand_label(const char *label)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind  = MIR_OPERAND_LABEL;
    op.label = label ? strdup(label) : NULL;
    return op;
}

MirOperand mir_operand_memory(uint32_t base_vreg, int32_t offset)
{
    MirOperand op;
    memset(&op, 0, sizeof(MirOperand));
    op.kind              = MIR_OPERAND_MEMORY;
    op.memory.base_vreg  = base_vreg;
    op.memory.offset     = offset;
    return op;
}

// ============================================================================
// Physical Register Helpers
// ============================================================================

MirPhysicalReg mir_preg_from_name(const char *name)
{
    if (!name)
        return MIR_PREG_RAX;

    // x86_64 registers
    if (strcmp(name, "rax") == 0) return MIR_PREG_RAX;
    if (strcmp(name, "rbx") == 0) return MIR_PREG_RBX;
    if (strcmp(name, "rcx") == 0) return MIR_PREG_RCX;
    if (strcmp(name, "rdx") == 0) return MIR_PREG_RDX;
    if (strcmp(name, "rsi") == 0) return MIR_PREG_RSI;
    if (strcmp(name, "rdi") == 0) return MIR_PREG_RDI;
    if (strcmp(name, "rbp") == 0) return MIR_PREG_RBP;
    if (strcmp(name, "rsp") == 0) return MIR_PREG_RSP;
    if (strcmp(name, "r8") == 0)  return MIR_PREG_R8;
    if (strcmp(name, "r9") == 0)  return MIR_PREG_R9;
    if (strcmp(name, "r10") == 0) return MIR_PREG_R10;
    if (strcmp(name, "r11") == 0) return MIR_PREG_R11;
    if (strcmp(name, "r12") == 0) return MIR_PREG_R12;
    if (strcmp(name, "r13") == 0) return MIR_PREG_R13;
    if (strcmp(name, "r14") == 0) return MIR_PREG_R14;
    if (strcmp(name, "r15") == 0) return MIR_PREG_R15;

    return MIR_PREG_RAX; // default
}

const char *mir_preg_to_name(MirPhysicalReg preg)
{
    switch (preg)
    {
    case MIR_PREG_RAX: return "rax";
    case MIR_PREG_RBX: return "rbx";
    case MIR_PREG_RCX: return "rcx";
    case MIR_PREG_RDX: return "rdx";
    case MIR_PREG_RSI: return "rsi";
    case MIR_PREG_RDI: return "rdi";
    case MIR_PREG_RBP: return "rbp";
    case MIR_PREG_RSP: return "rsp";
    case MIR_PREG_R8:  return "r8";
    case MIR_PREG_R9:  return "r9";
    case MIR_PREG_R10: return "r10";
    case MIR_PREG_R11: return "r11";
    case MIR_PREG_R12: return "r12";
    case MIR_PREG_R13: return "r13";
    case MIR_PREG_R14: return "r14";
    case MIR_PREG_R15: return "r15";
    default:           return "rax";
    }
}
