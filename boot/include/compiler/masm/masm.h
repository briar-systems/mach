#ifndef MASM_H
#define MASM_H

#include "compiler/masm/target.h"
#include "compiler/masm/section.h"
#include "compiler/masm/symbol.h"
#include <stddef.h>

typedef struct Masm
{
    MasmTarget target;
    
    // sections
    MasmSection **sections;
    size_t        section_count;
    size_t        section_capacity;
    
    // symbols
    MasmSymbol **symbols;
    size_t       symbol_count;
    size_t       symbol_capacity;
} Masm;

Masm        *masm_create(MasmTarget target);
void         masm_destroy(Masm *masm);

// section management
MasmSection *masm_get_section(Masm *masm, const char *name);
MasmSection *masm_get_or_create_section(Masm *masm, const char *name, MasmSectionKind kind);

// symbol management
MasmSymbol  *masm_get_symbol(Masm *masm, const char *name);
void         masm_add_symbol(Masm *masm, MasmSymbol *symbol);

// module management
void         masm_merge(Masm *dest, Masm *src);

#endif // MASM_H
