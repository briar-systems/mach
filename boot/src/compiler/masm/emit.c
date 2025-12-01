#include "compiler/masm/emit.h"
#include <stdio.h>

int masm_emit_object(Masm *masm, const char *filename)
{
    (void)masm;
    // TODO: implement object file emission (ELF, etc.)
    // for now, just print to stdout that we would emit
    printf("Emitting MASM to %s\n", filename);
    
    // create an empty file to satisfy build process
    FILE *f = fopen(filename, "w");
    if (f)
    {
        fclose(f);
        return 0;
    }
    
    return -1;
}
