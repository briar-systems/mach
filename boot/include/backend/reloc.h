#ifndef BACKEND_RELOC_H
#define BACKEND_RELOC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum BackendSectionKind
{
    BACKEND_SECTION_TEXT,
    BACKEND_SECTION_RODATA,
    BACKEND_SECTION_DATA,
    BACKEND_SECTION_COUNT
} BackendSectionKind;

typedef struct BackendLabel
{
    char               *name;
    BackendSectionKind  section;
    size_t              offset;
} BackendLabel;

typedef struct BackendLabelList
{
    BackendLabel *items;
    size_t        count;
    size_t        capacity;
} BackendLabelList;

void backend_label_list_init(BackendLabelList *list);
void backend_label_list_destroy(BackendLabelList *list);
bool backend_label_list_add(BackendLabelList *list, const char *name, BackendSectionKind section, size_t offset);

// relocation types (expand per architecture as needed)
typedef enum BackendRelocType
{
    BACKEND_RELOC_X86_64_PC32,
    BACKEND_RELOC_ABSOLUTE64,
} BackendRelocType;

typedef struct BackendReloc
{
    BackendSectionKind section;
    size_t             offset;
    BackendRelocType   type;
    char              *symbol;
    int64_t            addend;
} BackendReloc;

typedef struct BackendRelocList
{
    BackendReloc *items;
    size_t        count;
    size_t        capacity;
} BackendRelocList;

void backend_reloc_list_init(BackendRelocList *list);
void backend_reloc_list_destroy(BackendRelocList *list);
bool backend_reloc_list_add(BackendRelocList *list,
                            BackendSectionKind section,
                            size_t offset,
                            BackendRelocType type,
                            const char *symbol,
                            int64_t addend);

#endif // BACKEND_RELOC_H
