#include "compiler/masm/emit.h"
#include "compiler/masm/of/elf.h"
#include <stdio.h>

int masm_emit_object(Masm *masm, const char *filename)
{
    return masm_elf_write(masm, filename);
}
