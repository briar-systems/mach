#include "compiler/masm/of/elf.h"
#include <stdio.h>

int masm_elf_write(Masm *masm, const char *filename)
{
    (void)masm;
    // TODO: implement ELF generation
    // for now, just create an empty file
    FILE *f = fopen(filename, "w");
    if (f)
    {
        fclose(f);
        return 0;
    }
    return -1;
}
