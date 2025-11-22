#include "mir.h"
#include "mir_print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// forward declarations
static void print_operand(FILE *out, MirOperand *op);
static void print_instruction(FILE *out, MirInstruction *inst);
static void print_data(FILE *out, MirData *data);
static const char *opcode_to_string(MirOpcode opcode);

// ============================================================================
// MIR Module Printing
// ============================================================================

void mir_print_module(FILE *out, MirModule *module)
{
    if (!out || !module)
        return;

    // print data declarations
    MirData *data = module->data;
    while (data)
    {
        print_data(out, data);
        data = data->next;
    }

    if (module->data)
        fprintf(out, "\n");

    // print basic blocks
    MirBasicBlock *block = module->blocks;
    while (block)
    {
        mir_print_block(out, block);
        if (block->next)
            fprintf(out, "\n");
        block = block->next;
    }
}

void mir_print_block(FILE *out, MirBasicBlock *block)
{
    if (!out || !block)
        return;

    // print label
    if (block->is_exported)
        fprintf(out, "pub ");
    if (block->label)
        fprintf(out, "%s:\n", block->label);

    // print instructions
    MirInstruction *inst = block->instructions;
    while (inst)
    {
        fprintf(out, "    ");
        print_instruction(out, inst);
        fprintf(out, "\n");
        inst = inst->next;
    }
}

// ============================================================================
// Data Declaration Printing
// ============================================================================

static void print_data(FILE *out, MirData *data)
{
    if (!out || !data)
        return;

    switch (data->kind)
    {
    case MIR_DATA_VAL:
        fprintf(out, "val %s: %s = ", data->name, data->type ? type_to_string(data->type) : "?");
        if (data->init_value.string_value)
            fprintf(out, "\"%s\"", data->init_value.string_value);
        else
            fprintf(out, "%ld", data->init_value.int_value);
        fprintf(out, ";\n");
        break;

    case MIR_DATA_VAR:
        fprintf(out, "var %s: %s = ", data->name, data->type ? type_to_string(data->type) : "?");
        if (data->init_value.array_int.count > 0)
        {
            fprintf(out, "[");
            for (size_t i = 0; i < data->init_value.array_int.count; i++)
            {
                if (i > 0)
                    fprintf(out, ", ");
                fprintf(out, "%ld", data->init_value.array_int.values[i]);
            }
            fprintf(out, "]");
        }
        else
        {
            fprintf(out, "%ld", data->init_value.int_value);
        }
        fprintf(out, ";\n");
        break;

    case MIR_DATA_VAR_UNINIT:
        fprintf(out, "var %s: %s;\n", data->name, data->type ? type_to_string(data->type) : "?");
        break;

    case MIR_DATA_EXT:
        fprintf(out, "ext %s: %s;\n", data->name, data->type ? type_to_string(data->type) : "?");
        break;

    case MIR_DATA_EXT_LABEL:
        fprintf(out, "ext %s;\n", data->name);
        break;
    }
}

// ============================================================================
// Instruction Printing
// ============================================================================

static void print_instruction(FILE *out, MirInstruction *inst)
{
    if (!out || !inst)
        return;

    const char *op_name = opcode_to_string(inst->opcode);

    // special case: no-operand instructions
    if (inst->operand_count == 0)
    {
        fprintf(out, "%s;", op_name);
        return;
    }

    // print opcode with type suffix
    fprintf(out, "%s", op_name);
    if (inst->type)
    {
        char *type_str = type_to_string(inst->type);
        fprintf(out, ".%s", type_str);
        free(type_str);
    }
    if (inst->type2)
    {
        char *type_str = type_to_string(inst->type2);
        fprintf(out, ".%s", type_str);
        free(type_str);
    }

    // print operands
    for (size_t i = 0; i < inst->operand_count; i++)
    {
        if (i == 0)
            fprintf(out, " ");
        else
            fprintf(out, ", ");
        print_operand(out, &inst->operands[i]);
    }

    fprintf(out, ";");
}

static void print_operand(FILE *out, MirOperand *op)
{
    if (!out || !op)
        return;

    switch (op->kind)
    {
    case MIR_OPERAND_NONE:
        break;

    case MIR_OPERAND_VREG:
        fprintf(out, "%%%u", op->vreg_id);
        break;

    case MIR_OPERAND_PREG:
        fprintf(out, "%s", mir_preg_to_name(op->preg));
        break;

    case MIR_OPERAND_IMM:
        fprintf(out, "%ld", op->immediate);
        break;

    case MIR_OPERAND_LABEL:
        fprintf(out, "%s", op->label ? op->label : "<null>");
        break;

    case MIR_OPERAND_MEMORY:
        if (op->memory.offset == 0)
            fprintf(out, "[%%%u]", op->memory.base_vreg);
        else if (op->memory.offset > 0)
            fprintf(out, "[%%%u+%d]", op->memory.base_vreg, op->memory.offset);
        else
            fprintf(out, "[%%%u%d]", op->memory.base_vreg, op->memory.offset);
        break;
    }
}

// ============================================================================
// Opcode to String
// ============================================================================

static const char *opcode_to_string(MirOpcode opcode)
{
    switch (opcode)
    {
    case MIR_OP_MOV:          return "mov";
    case MIR_OP_LOAD:         return "load";
    case MIR_OP_STORE:        return "store";
    case MIR_OP_LEA:          return "lea";
    case MIR_OP_ADD:          return "add";
    case MIR_OP_SUB:          return "sub";
    case MIR_OP_MUL:          return "mul";
    case MIR_OP_DIV:          return "div";
    case MIR_OP_MOD:          return "mod";
    case MIR_OP_NEG:          return "neg";
    case MIR_OP_AND:          return "and";
    case MIR_OP_OR:           return "or";
    case MIR_OP_XOR:          return "xor";
    case MIR_OP_NOT:          return "not";
    case MIR_OP_SHL:          return "shl";
    case MIR_OP_SHR:          return "shr";
    case MIR_OP_SAR:          return "sar";
    case MIR_OP_CMP_EQ:       return "cmp.eq";
    case MIR_OP_CMP_NE:       return "cmp.ne";
    case MIR_OP_CMP_LT:       return "cmp.lt";
    case MIR_OP_CMP_LE:       return "cmp.le";
    case MIR_OP_CMP_GT:       return "cmp.gt";
    case MIR_OP_CMP_GE:       return "cmp.ge";
    case MIR_OP_ZEXT:         return "zext";
    case MIR_OP_SEXT:         return "sext";
    case MIR_OP_TRUNC:        return "trunc";
    case MIR_OP_BITCAST:      return "bitcast";
    case MIR_OP_JMP:          return "jmp";
    case MIR_OP_BR:           return "br";
    case MIR_OP_CALL:         return "call";
    case MIR_OP_RET:          return "ret";
    case MIR_OP_UNREACHABLE:  return "unreachable";
    case MIR_OP_ALLOCA:       return "alloca";
    case MIR_OP_ARCH_SYSCALL: return "x86.syscall";
    case MIR_OP_ARCH_SVC:     return "arm.svc";
    case MIR_OP_ARCH_ECALL:   return "riscv.ecall";
    case MIR_OP_ARCH_HLT:     return "x86.hlt";
    default:                  return "<unknown>";
    }
}
