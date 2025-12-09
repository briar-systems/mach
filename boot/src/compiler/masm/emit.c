#include "compiler/masm/emit.h"
#include "compiler/masm/of/spec.h"
#include "compiler/masm/opt/masm_opt.h"

int masm_emit_object(Masm *masm, const char *filename)
{
    // run optimizations before emitting
    masm_opt_run(masm);
    const MasmOFSpec *of_spec = masm_of_spec_select(masm->target.of);
    if (!of_spec || !of_spec->write_object)
    {
        return -1;
    }
    return of_spec->write_object(masm, filename);
}
