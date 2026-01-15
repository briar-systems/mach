#ifndef MASM_SECTION_H
#define MASM_SECTION_H

#include "compiler/masm/instruction.h"
#include <stdint.h>
#include <stddef.h>

typedef enum MasmSectionKind
{
    MASM_SECTION_TEXT,
    MASM_SECTION_DATA,
    MASM_SECTION_BSS,
    MASM_SECTION_RODATA,
    MASM_SECTION_CUSTOM
} MasmSectionKind;

typedef struct MasmSection
{
    MasmSectionKind kind;
    char           *name;
    
    // for text sections
    MasmInstruction *instructions;
    size_t           inst_count;
    size_t           inst_capacity;
    
    // for data sections
    uint8_t        *data;
    size_t          data_size;
    size_t          data_capacity;
} MasmSection;

MasmSection *masm_section_create(MasmSectionKind kind, const char *name);
void         masm_section_destroy(MasmSection *section);

// text manipulation
void masm_section_append_inst(MasmSection *section, MasmInstruction inst);

// data manipulation
void masm_section_append_data(MasmSection *section, const void *data, size_t size);
void masm_section_append_zero(MasmSection *section, size_t size);

#endif // MASM_SECTION_H
