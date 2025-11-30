#include "compiler/mir/global.h"
#include <stdlib.h>
#include <string.h>

MIRGlobal *mir_global_create(const char *name, MIRType *type, MIRGlobalKind kind, bool is_exported)
{
    MIRGlobal *global = malloc(sizeof(MIRGlobal));
    if (!global)
    {
        return NULL;
    }

    global->name = name ? strdup(name) : NULL;
    global->type = type;
    global->kind = kind;
    global->init_kind = MIR_INIT_NONE;
    global->is_exported = is_exported;
    global->next = NULL;

    // initialize union to zero
    memset(&global->init, 0, sizeof(global->init));

    return global;
}

void mir_global_destroy(MIRGlobal *global)
{
    if (!global)
    {
        return;
    }

    if (global->name)
    {
        free(global->name);
    }

    // note: string_value and array_data are not owned by the global
    // they point to static data or will be freed by the caller

    free(global);
}

void mir_global_set_int_init(MIRGlobal *global, int64_t value)
{
    if (global)
    {
        global->init.int_value = value;
        global->init_kind = MIR_INIT_INT;
    }
}

void mir_global_set_float_init(MIRGlobal *global, double value)
{
    if (global)
    {
        global->init.float_value = value;
        global->init_kind = MIR_INIT_FLOAT;
    }
}

void mir_global_set_string_init(MIRGlobal *global, const char *value)
{
    if (global)
    {
        global->init.string_value = value;
        global->init_kind = MIR_INIT_STRING;
    }
}
