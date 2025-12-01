#include "compiler/masm/masm.h"
#include <stdlib.h>
#include <string.h>

Masm *masm_create(MasmTarget target)
{
    Masm *masm = malloc(sizeof(Masm));
    if (!masm) return NULL;
    
    memset(masm, 0, sizeof(Masm));
    masm->target = target;
    
    // create default sections
    masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);
    masm_get_or_create_section(masm, ".data", MASM_SECTION_DATA);
    masm_get_or_create_section(masm, ".bss", MASM_SECTION_BSS);
    masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);
    
    return masm;
}

void masm_destroy(Masm *masm)
{
    if (!masm) return;
    
    if (masm->sections)
    {
        for (size_t i = 0; i < masm->section_count; i++)
        {
            masm_section_destroy(masm->sections[i]);
        }
        free(masm->sections);
    }
    
    if (masm->symbols)
    {
        for (size_t i = 0; i < masm->symbol_count; i++)
        {
            masm_symbol_destroy(masm->symbols[i]);
        }
        free(masm->symbols);
    }
    
    free(masm);
}

MasmSection *masm_get_section(Masm *masm, const char *name)
{
    for (size_t i = 0; i < masm->section_count; i++)
    {
        if (strcmp(masm->sections[i]->name, name) == 0)
        {
            return masm->sections[i];
        }
    }
    return NULL;
}

MasmSection *masm_get_or_create_section(Masm *masm, const char *name, MasmSectionKind kind)
{
    MasmSection *section = masm_get_section(masm, name);
    if (section) return section;
    
    section = masm_section_create(kind, name);
    
    if (masm->section_count >= masm->section_capacity)
    {
        size_t new_capacity = masm->section_capacity == 0 ? 8 : masm->section_capacity * 2;
        masm->sections = realloc(masm->sections, sizeof(MasmSection*) * new_capacity);
        masm->section_capacity = new_capacity;
    }
    
    masm->sections[masm->section_count++] = section;
    return section;
}

MasmSymbol *masm_get_symbol(Masm *masm, const char *name)
{
    for (size_t i = 0; i < masm->symbol_count; i++)
    {
        if (strcmp(masm->symbols[i]->name, name) == 0)
        {
            return masm->symbols[i];
        }
    }
    return NULL;
}

void masm_add_symbol(Masm *masm, MasmSymbol *symbol)
{
    if (masm->symbol_count >= masm->symbol_capacity)
    {
        size_t new_capacity = masm->symbol_capacity == 0 ? 64 : masm->symbol_capacity * 2;
        masm->symbols = realloc(masm->symbols, sizeof(MasmSymbol*) * new_capacity);
        masm->symbol_capacity = new_capacity;
    }
    
    masm->symbols[masm->symbol_count++] = symbol;
}
