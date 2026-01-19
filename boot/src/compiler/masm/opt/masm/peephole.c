#include "compiler/masm/opt/masm/peephole.h"
#include "compiler/masm/ir.h"
#include "compiler/masm/section.h"

// ir-level peephole optimization pass (runs pre-isel)
// currently a stub - patterns can be added as needed

static void masm_opt_ir_peephole(MasmSection *section)
{
    if (!section || section->inst_count < 2)
    {
        return;
    }

    // future IR-level optimizations go here
    // examples:
    // - MOV a, b; MOV b, a -> remove second MOV
    // - ADD a, 0 -> remove
    // - MUL a, 1 -> remove
    // - MUL a, 0 -> MOV a, 0
    // - consecutive stores to same location -> keep only last
}

void masm_opt_ir_run(Masm *masm)
{
    if (!masm)
    {
        return;
    }

    for (size_t i = 0; i < masm->section_count; i++)
    {
        MasmSection *section = masm->sections[i];
        if (section->kind == MASM_SECTION_TEXT)
        {
            masm_opt_ir_peephole(section);
        }
    }
}
