#include "compiler/masm/section.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


MasmSection *masm_section_create(MasmSectionKind kind, const char *name)
{
    MasmSection *section = malloc(sizeof(MasmSection));
    if (!section) return NULL;
    
    memset(section, 0, sizeof(MasmSection));
    section->kind = kind;
    section->name = strdup(name);
    
    return section;
}

void masm_section_destroy(MasmSection *section)
{
    if (!section) return;
    
    if (section->name) free(section->name);
    
    if (section->instructions)
    {
        for (size_t i = 0; i < section->inst_count; i++)
        {
            masm_inst_destroy(section->instructions[i]);
        }
        free(section->instructions);
    }
    
    if (section->data) free(section->data);
    
    if (section->data_relocs)
    {
        for (size_t i = 0; i < section->data_reloc_count; i++)
        {
            if (section->data_relocs[i].symbol_name)
            {
                free((void *)section->data_relocs[i].symbol_name);
            }
        }
        free(section->data_relocs);
    }
    
    free(section);
}

void masm_section_append_inst(MasmSection *section, MasmInstruction inst)
{
    if (section->inst_count >= section->inst_capacity)
    {
        size_t new_capacity = section->inst_capacity == 0 ? 64 : section->inst_capacity * 2;
        section->instructions = realloc(section->instructions, sizeof(MasmInstruction) * new_capacity);
        section->inst_capacity = new_capacity;
    }
    
    section->instructions[section->inst_count++] = inst;
}

void masm_section_append_data(MasmSection *section, const void *data, size_t size)
{
    if (section->data_size + size > section->data_capacity)
    {
        size_t new_capacity = section->data_capacity == 0 ? 64 : section->data_capacity * 2;
        while (new_capacity < section->data_size + size) new_capacity *= 2;
        
        section->data = realloc(section->data, new_capacity);
        section->data_capacity = new_capacity;
    }
    
    memcpy(section->data + section->data_size, data, size);
    section->data_size += size;
}

void masm_section_append_zero(MasmSection *section, size_t size)
{
    if (section->data_size + size > section->data_capacity)
    {
        size_t new_capacity = section->data_capacity == 0 ? 64 : section->data_capacity * 2;
        while (new_capacity < section->data_size + size) new_capacity *= 2;
        
        section->data = realloc(section->data, new_capacity);
        section->data_capacity = new_capacity;
    }
    
    memset(section->data + section->data_size, 0, size);
    section->data_size += size;
}

void masm_section_append_reloc(MasmSection *section, size_t offset, const char *symbol_name, int64_t addend)
{
    if (section->data_reloc_count >= section->data_reloc_capacity)
    {
        size_t new_capacity = section->data_reloc_capacity == 0 ? 16 : section->data_reloc_capacity * 2;
        section->data_relocs = realloc(section->data_relocs, sizeof(MasmDataReloc) * new_capacity);
        section->data_reloc_capacity = new_capacity;
    }
    
    section->data_relocs[section->data_reloc_count].offset = offset;
    section->data_relocs[section->data_reloc_count].symbol_name = strdup(symbol_name);
    section->data_relocs[section->data_reloc_count].addend = addend;
    section->data_reloc_count++;
}
