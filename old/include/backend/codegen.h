#ifndef CODEGEN_H
#define CODEGEN_H

#include "codebuf.h"

typedef enum SectionKind
{
    SECTION_TEXT,
    SECTION_RODATA,
    SECTION_DATA,
    SECTION_COUNT
} SectionKind;

typedef struct Label
{
    char       *name;
    SectionKind section;
    size_t      offset;
} Label;

typedef struct LabelList
{
    Label *items;
    size_t      count;
    size_t      capacity;
} LabelList;

void label_list_init(LabelList *list);
void label_list_dnit(LabelList *list);
bool label_list_add(LabelList *list, const char *name, SectionKind section, size_t offset);

// relocation kinds (expand per architecture as needed)
typedef enum RelocKind
{
    RELOC_X86_64_PC32,
    RELOC_ABSOLUTE64,
} RelocKind;

typedef struct Reloc
{
    SectionKind section;
    size_t      offset;
    RelocKind   kind;
    char       *symbol;
    int64_t     addend;
} Reloc;

typedef struct RelocList
{
    Reloc *items;
    size_t count;
    size_t capacity;
} RelocList;

void reloc_list_init(RelocList *list);
void reloc_list_dnit(RelocList *list);
bool reloc_list_add(RelocList *list, SectionKind section, size_t offset, RelocKind type, const char *symbol, int64_t addend);

typedef struct Section
{
    SectionKind kind;
    CodeBuffer  buffer;
} Section;

typedef struct CodegenResult
{
    Section   text;
    Section   rodata;
    Section   data;
    LabelList labels;
    RelocList relocs;
} CodegenResult;

void codegen_result_init(CodegenResult *result);
void codegen_result_dnit(CodegenResult *result);

#endif
