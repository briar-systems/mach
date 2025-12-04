#include "compiler/masm/opt/masm_opt.h"
#include "compiler/masm/instruction.h"
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

// peephole optimization pass
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

        // skip labels
        if (inst->opcode == MASM_OP_LABEL)
        {
            continue;
        }

        // pattern 1: MOV reg, reg (same register) -> remove
        if (inst->opcode == MASM_OP_MOV && inst->operand_count == 2)
        {
            if (same_register(inst->operands[0], inst->operands[1]))
            {
                remove[i] = 1;
                continue;
            }
        }

        // pattern 2: MOV rax, X; MOV rax, Y -> remove first MOV (dead store)
        // only if no labels between them and X is not memory (could have side effects)
        if (inst->opcode == MASM_OP_MOV && inst->operand_count == 2 && i + 1 < section->inst_count)
        {
            MasmInstruction *next = &section->instructions[i + 1];
            if (next->opcode == MASM_OP_MOV && next->operand_count == 2)
            {
                // same destination register, source is not memory
                if (same_register(inst->operands[0], next->operands[0]) && inst->operands[1].kind != MASM_OPERAND_MEMORY)
                {
                    remove[i] = 1;
                    continue;
                }
            }
        }

        // pattern 3: PUSH reg; POP reg -> remove both (no-op)
        if (inst->opcode == MASM_OP_PUSH && inst->operand_count == 1 && inst->operands[0].kind == MASM_OPERAND_REGISTER && i + 1 < section->inst_count)
        {
            MasmInstruction *next = &section->instructions[i + 1];
            if (next->opcode == MASM_OP_POP && next->operand_count == 1)
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

void masm_opt_run(Masm *masm)
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
