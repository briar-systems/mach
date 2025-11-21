#include "reloc.h"
#include <stdlib.h>
#include <string.h>

static char *dup_string(const char *str)
{
    if (!str)
        return NULL;
    size_t len = strlen(str) + 1;
    char *copy = malloc(len);
    if (copy)
        memcpy(copy, str, len);
    return copy;
}

void backend_label_list_init(BackendLabelList *list)
{
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

void backend_label_list_destroy(BackendLabelList *list)
{
    if (!list)
        return;
    for (size_t i = 0; i < list->count; i++)
    {
        free(list->items[i].name);
    }
    free(list->items);
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

bool backend_label_list_add(BackendLabelList *list, const char *name, BackendSectionKind section, size_t offset)
{
    if (!list)
        return false;
    if (list->count == list->capacity)
    {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        BackendLabel *new_items = realloc(list->items, new_capacity * sizeof(BackendLabel));
        if (!new_items)
            return false;
        list->items    = new_items;
        list->capacity = new_capacity;
    }

    BackendLabel *label = &list->items[list->count++];
    label->name    = dup_string(name);
    label->section = section;
    label->offset  = offset;
    return label->name != NULL;
}

void backend_reloc_list_init(BackendRelocList *list)
{
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

void backend_reloc_list_destroy(BackendRelocList *list)
{
    if (!list)
        return;
    for (size_t i = 0; i < list->count; i++)
    {
        free(list->items[i].symbol);
    }
    free(list->items);
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

bool backend_reloc_list_add(BackendRelocList *list,
                            BackendSectionKind section,
                            size_t offset,
                            BackendRelocType type,
                            const char *symbol,
                            int64_t addend)
{
    if (!list)
        return false;
    if (list->count == list->capacity)
    {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        BackendReloc *new_items = realloc(list->items, new_capacity * sizeof(BackendReloc));
        if (!new_items)
            return false;
        list->items    = new_items;
        list->capacity = new_capacity;
    }

    BackendReloc *reloc = &list->items[list->count++];
    reloc->section = section;
    reloc->offset  = offset;
    reloc->type    = type;
    reloc->symbol  = dup_string(symbol);
    reloc->addend  = addend;
    return reloc->symbol != NULL;
}
