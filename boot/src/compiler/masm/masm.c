#include "compiler/masm/masm.h"
#include "compiler/masm/instruction.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void masm_merge(Masm *dest, Masm *src)
{
    if (!dest || !src) return;

    // when we append section contents from `src` onto `dest`, any symbols with
    // concrete offsets into those sections must be adjusted by the base offset
    // in the destination section.
    typedef struct
    {
        const char *name;
        uint64_t    data_base;
    } SectionBase;

    SectionBase bases[64];
    size_t      base_count = 0;

    // Merge sections
    for (size_t i = 0; i < src->section_count; i++)
    {
        MasmSection *src_sec = src->sections[i];
        MasmSection *dest_sec = masm_get_or_create_section(dest, src_sec->name, src_sec->kind);

        // record the destination base *before* we append anything from src.
        // note: we only use this for data-backed sections; text symbol offsets
        // are resolved during encoding from label positions.
        if (base_count < (sizeof(bases) / sizeof(bases[0])))
        {
            bases[base_count].name      = src_sec->name;
            bases[base_count].data_base = (uint64_t)dest_sec->data_size;
            base_count++;
        }

        // Append instructions
        fprintf(stderr, "[masm_merge] merging section %s: src has %zu insts, dest has %zu insts before\n",
                src_sec->name, src_sec->inst_count, dest_sec->inst_count);
        for (size_t j = 0; j < src_sec->inst_count; j++)
        {
            MasmInstruction *src_inst = &src_sec->instructions[j];
            // Create a copy of the instruction (allocates new operand array)
            MasmInstruction new_inst = masm_inst_create(src_inst->opcode, src_inst->operands, src_inst->operand_count);
            if (src_inst->opcode == MASM_OP_MOV && src_inst->operand_count >= 2 && src_inst->operands[1].kind == MASM_OPERAND_LABEL)
            {
                fprintf(stderr, "[masm_merge] copying MOV with label %s at inst %zu, dest will be %zu\n", 
                        src_inst->operands[1].label, j, dest_sec->inst_count);
                fprintf(stderr, "[masm_merge] new_inst op0 kind=%d, op1 kind=%d\n", 
                        new_inst.operands[0].kind, new_inst.operands[1].kind);
            }
            masm_section_append_inst(dest_sec, new_inst);
        }
        fprintf(stderr, "[masm_merge] dest now has %zu insts after\n", dest_sec->inst_count);
        
        // Append data
        if (src_sec->data_size > 0)
        {
            masm_section_append_data(dest_sec, src_sec->data, src_sec->data_size);
        }
    }

    // Merge symbols
    for (size_t i = 0; i < src->symbol_count; i++)
    {
        MasmSymbol *src_sym = src->symbols[i];
        
        // Create new symbol (duplicates name)
        MasmSymbol *new_sym = masm_symbol_create(src_sym->name, src_sym->kind, src_sym->bind);
        
        // Copy other properties
        new_sym->offset = src_sym->offset;
        new_sym->size = src_sym->size;
        
        if (src_sym->section_name)
        {
            new_sym->section_name = strdup(src_sym->section_name);
        }

        // adjust concrete offsets for merged data sections
        if (new_sym->section_name && strcmp(new_sym->section_name, ".text") != 0)
        {
            for (size_t j = 0; j < base_count; j++)
            {
                if (strcmp(bases[j].name, new_sym->section_name) == 0)
                {
                    new_sym->offset += bases[j].data_base;
                    break;
                }
            }
        }
        
        masm_add_symbol(dest, new_sym);
    }
}
