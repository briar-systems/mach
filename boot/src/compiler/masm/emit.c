#include "compiler/masm/emit.h"
#include "compiler/masm/isa/spec.h"
#include "compiler/masm/of/spec.h"
#include "compiler/masm/opt/masm/peephole.h"
#include "compiler/masm/opt/x86_64/peephole.h"

int masm_emit_object(Masm *masm, const char *filename)
{
    // 1. run IR-level optimizations (pre-isel)
    masm_opt_ir_run(masm);

    // 2. run instruction selection (IR -> target opcodes)
    const MasmISASpec *isa = masm_isa_spec_select(masm->target);
    if (isa && isa->isel)
    {
        isa->isel(masm);
    }

    // 3. run target-specific optimizations (post-isel)
    masm_opt_x86_run(masm);

    // 4. write object file
    const MasmOFSpec *of_spec = masm_of_spec_select(masm->target.of);
    if (!of_spec || !of_spec->write_object)
    {
        return -1;
    }
    return of_spec->write_object(masm, filename);
}
