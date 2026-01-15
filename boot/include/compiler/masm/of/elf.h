#ifndef MASM_OF_ELF_H
#define MASM_OF_ELF_H

#include "compiler/masm/masm.h"

// elf file generation
int masm_elf_write(Masm *masm, const char *filename);

#endif // MASM_OF_ELF_H
