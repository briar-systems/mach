#include "compiler/masm/backend.h"
#include "compiler/masm/ir.h"
#include "compiler/masm/isa/x86_64.h"
#include "compiler/masm/section.h"
#include <stdio.h>
#include <stdlib.h>



// -----------------------------------------------------------------------------
// Translation: IR -> x86-64 Machine Code
// -----------------------------------------------------------------------------

static void translate_mov(MasmSection *section, MasmInstruction *inst)
{
    uint8_t buffer[15];
    MasmInstruction legacy_inst = *inst;
    legacy_inst.opcode = MASM_OP_MOV;

    int len = masm_x86_encode(legacy_inst, buffer, sizeof(buffer));
    if (len > 0)
    {
        masm_section_append_data(section, buffer, len);
    }
}

static void translate_ret(MasmSection *section, MasmInstruction *inst)
{
    uint8_t buffer[15];
    MasmInstruction legacy_inst = *inst;
    legacy_inst.opcode = MASM_OP_RET;

    int len = masm_x86_encode(legacy_inst, buffer, sizeof(buffer));
    if (len > 0)
    {
        masm_section_append_data(section, buffer, len);
    }
}

static void translate_inst(MasmSection *section, MasmInstruction *inst)
{
    switch (inst->opcode)
    {
        case MASM_IR_MOV:
            translate_mov(section, inst);
            break;
            
        case MASM_IR_RET:
            translate_ret(section, inst);
            break;

        case MASM_IR_SYSCALL:
        {
            uint8_t buffer[15];
            MasmInstruction legacy_inst = *inst;
            legacy_inst.opcode = MASM_OP_X86_SYSCALL;
            int len = masm_x86_encode(legacy_inst, buffer, sizeof(buffer));
            if (len > 0)
            {
                masm_section_append_data(section, buffer, len);
            }
            break;
        }
            
        // TODO: Implement other opcodes
        
        default:
            // For now, warn about unimplemented opcodes
            // fprintf(stderr, "warning: unimplemented x86-64 backend opcode: %s\n", masm_ir_name(inst->opcode));
            break;
    }
}

static void x86_64_codegen(Masm *masm)
{
    // Iterate over all text sections and generate code
    for (size_t i = 0; i < masm->section_count; ++i)
    {
        MasmSection *section = masm->sections[i];
        
        // We only generate code for executable sections (text)
        // Data/BSS are populated directly by the frontend usually, 
        // or by pseudo-ops like MASM_IR_DATA
        if (section->kind == MASM_SECTION_TEXT)
        {
            for (size_t j = 0; j < section->inst_count; ++j)
            {
                translate_inst(section, &section->instructions[j]);
            }
        }
    }
}

const MasmBackend masm_backend_x86_64 = {
    .name = "x86_64",
    .codegen = x86_64_codegen
};