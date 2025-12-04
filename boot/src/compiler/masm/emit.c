#include "compiler/masm/emit.h"
#include "compiler/masm/of/elf.h"
#include "compiler/masm/opt/masm_opt.h"

int masm_emit_object(Masm *masm, const char *filename)
{
    // run optimizations before emitting
    masm_opt_run(masm);

    return masm_elf_write(masm, filename);
}
