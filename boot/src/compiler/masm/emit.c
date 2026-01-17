#include "compiler/masm/emit.h"
#include "compiler/masm/of/spec.h"
#include "compiler/masm/opt/masm_opt.h"
#include "compiler/masm/backend.h"

int masm_emit_object(Masm *masm, const char *filename)
{
    // run backend code generation
    const MasmBackend *backend = masm_backend_get(masm->target.isa);
    if (backend && backend->codegen)
    {
        backend->codegen(masm);
    }

    // run optimizations before emitting
    masm_opt_run(masm);
    const MasmOFSpec *of_spec = masm_of_spec_select(masm->target.of);
    if (!of_spec || !of_spec->write_object)
    {
        return -1;
    }
    return of_spec->write_object(masm, filename);
}
