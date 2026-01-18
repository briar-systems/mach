#include "compiler/masm/emit.h"
#include "compiler/masm/isa/spec.h"
#include "compiler/masm/of/spec.h"
#include "compiler/masm/opt/peephole.h"

int masm_emit_object(Masm *masm, const char *filename)
{
    // run instruction selection
    const MasmISASpec *isa = masm_isa_spec_select(masm->target);
    if (isa && isa->isel)
    {
        isa->isel(masm);
    }

    // run peephole optimization
    masm_opt_run(masm);

    // write object file
    const MasmOFSpec *of_spec = masm_of_spec_select(masm->target.of);
    if (!of_spec || !of_spec->write_object)
    {
        return -1;
    }
    return of_spec->write_object(masm, filename);
}
