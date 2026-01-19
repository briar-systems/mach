#include "compiler/masm/opt/x86_64/peephole.h"
#include "compiler/masm/ir.h"
#include "compiler/masm/isa/x86_64/x86_64.h"
#include <stdlib.h>
#include <string.h>

// check if two operands are the same register
static bool same_register(MasmOperand a, MasmOperand b)
{
    if (a.kind != MASM_OPERAND_REGISTER || b.kind != MASM_OPERAND_REGISTER)
    {
        return false;
    }
    return a.reg.id == b.reg.id && a.reg.size == b.reg.size;
}

static bool mem_uses_reg(MasmOperand mem_op, uint32_t reg_id)
{
    if (mem_op.kind != MASM_OPERAND_MEMORY)
    {
        return false;
    }
    return mem_op.mem.base.id == reg_id || mem_op.mem.index.id == reg_id;
}

// check if instruction is an x86 mov (register-to-register)
static bool is_x86_mov_rr(MasmInstruction *inst)
{
    return inst->kind == MASM_OPCODE_X86 && inst->opcode == MASM_OP_X86_MOV_RR;
}

// check if instruction is any x86 mov variant
static bool is_x86_mov(MasmInstruction *inst)
{
    if (inst->kind != MASM_OPCODE_X86) return false;
    return inst->opcode == MASM_OP_X86_MOV_RR ||
           inst->opcode == MASM_OP_X86_MOV_RM ||
           inst->opcode == MASM_OP_X86_MOV_MR ||
           inst->opcode == MASM_OP_X86_MOV_RI ||
           inst->opcode == MASM_OP_X86_MOV_MI;
}

// peephole optimization pass (x86_64-specific, runs post-isel)
static void masm_opt_peephole(MasmSection *section)
{
    if (!section || section->inst_count < 2)
    {
        return;
    }

    // mark instructions to remove (0 = keep, 1 = remove)
    int *remove = calloc(section->inst_count, sizeof(int));
    if (!remove)
    {
        return;
    }

    for (size_t i = 0; i < section->inst_count; i++)
    {
        MasmInstruction *inst = &section->instructions[i];

        // skip labels (IR pseudo-ops that survive isel)
        if (inst->kind == MASM_OPCODE_IR && inst->opcode == MASM_IR_LABEL)
        {
            continue;
        }

        // pattern 1: MOV_RR reg, reg (same register) -> remove
        if (is_x86_mov_rr(inst) && inst->operand_count == 2)
        {
            if (same_register(inst->operands[0], inst->operands[1]))
            {
                remove[i] = 1;
                continue;
            }
        }

        // pattern 2: MOV rax, X; MOV rax, Y -> remove first MOV (dead store)
        // only if no labels between them, X is not memory, and the next MOV does not
        // use the destination register as part of its source addressing (e.g.
        // `mov rax, label; mov rax, [rax]` is a load-from-address idiom and is not dead).
        if (is_x86_mov(inst) && inst->operand_count == 2 && i + 1 < section->inst_count)
        {
            MasmInstruction *next = &section->instructions[i + 1];
            if (is_x86_mov(next) && next->operand_count == 2)
            {
                // same destination register, source is not memory
                if (same_register(inst->operands[0], next->operands[0]) && inst->operands[1].kind != MASM_OPERAND_MEMORY)
                {
                    uint32_t dst_reg = inst->operands[0].reg.id;

                    // check if next instruction reads the register (memory or register source)
                    bool next_reads_reg = mem_uses_reg(next->operands[1], dst_reg);
                    if (!next_reads_reg && next->operands[1].kind == MASM_OPERAND_REGISTER && next->operands[1].reg.id == dst_reg)
                    {
                        next_reads_reg = true;
                    }

                    if (!next_reads_reg)
                    {
                        remove[i] = 1;
                        continue;
                    }
                }
            }
        }

        // pattern 3: PUSH_R reg; POP_R reg -> remove both (no-op)
        if (inst->kind == MASM_OPCODE_X86 && inst->opcode == MASM_OP_X86_PUSH_R && inst->operand_count == 1 && inst->operands[0].kind == MASM_OPERAND_REGISTER && i + 1 < section->inst_count)
        {
            MasmInstruction *next = &section->instructions[i + 1];
            if (next->kind == MASM_OPCODE_X86 && next->opcode == MASM_OP_X86_POP_R && next->operand_count == 1)
            {
                if (same_register(inst->operands[0], next->operands[0]))
                {
                    remove[i]     = 1;
                    remove[i + 1] = 1;
                    i++; // skip next
                    continue;
                }
            }
        }
    }

    // compact instructions, removing marked ones
    size_t write_idx = 0;
    for (size_t read_idx = 0; read_idx < section->inst_count; read_idx++)
    {
        if (!remove[read_idx])
        {
            if (write_idx != read_idx)
            {
                section->instructions[write_idx] = section->instructions[read_idx];
            }
            write_idx++;
        }
        else
        {
            // destroy the removed instruction
            masm_inst_destroy(section->instructions[read_idx]);
        }
    }
    section->inst_count = write_idx;

    free(remove);
}

void masm_opt_x86_run(Masm *masm)
{
    if (!masm)
    {
        return;
    }

    // run peephole on all sections
    for (size_t i = 0; i < masm->section_count; i++)
    {
        MasmSection *section = masm->sections[i];
        if (section->kind == MASM_SECTION_TEXT)
        {
            masm_opt_peephole(section);
        }
    }
}
