#ifndef MASM_ISA_X86_64_ASM_H
#define MASM_ISA_X86_64_ASM_H

#include "compiler/masm/section.h"
#include <stdint.h>

// parse inline asm block and emit x86_64 instructions directly
void masm_x86_parse_inline_asm(struct MasmSection *section, const char *content, uint8_t ptr_size);

#endif // MASM_ISA_X86_64_ASM_H
