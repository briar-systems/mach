#include "compiler/mir/value.h"
#include <stdlib.h>
#include <string.h>

MIRValue *mir_value_create(uint32_t id, MIRType *type, const char *name)
{
    MIRValue *value = malloc(sizeof(MIRValue));
    if (!value)
    {
        return NULL;
    }

    value->id = id;
    value->type = type;
    value->def_inst = NULL;
    value->name = name ? strdup(name) : NULL;

    return value;
}

void mir_value_destroy(MIRValue *value)
{
    if (!value)
    {
        return;
    }

    if (value->name)
    {
        free(value->name);
    }

    free(value);
}
