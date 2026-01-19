#include "compiler/masm/of/spec.h"
#include "compiler/masm/of/elf.h"

static const MasmOFSpec OF_ELF = {
    .name         = "elf",
    .write_object = masm_elf_write,
};

const MasmOFSpec *masm_of_spec_select(MasmTargetOF of)
{
    switch (of)
    {
    case MASM_OF_ELF:
        return &OF_ELF;
    default:
        return NULL;
    }
}
