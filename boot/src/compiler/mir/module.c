#include "compiler/mir/module.h"
#include <stdlib.h>
#include <string.h>

MIRModule *mir_module_create(const char *name)
{
    MIRModule *module = malloc(sizeof(MIRModule));
    if (!module)
    {
        return NULL;
    }

    module->name           = name ? strdup(name) : NULL;
    module->functions      = NULL;
    module->globals        = NULL;
    module->function_count = 0;
    module->global_count   = 0;

    return module;
}

void mir_module_destroy(MIRModule *module)
{
    if (!module)
    {
        return;
    }

    if (module->name)
    {
        free(module->name);
    }

    // free all functions
    MIRFunction *func = module->functions;
    while (func)
    {
        MIRFunction *next = func->next;
        mir_function_destroy(func);
        func = next;
    }

    // free all globals
    MIRGlobal *global = module->globals;
    while (global)
    {
        MIRGlobal *next = global->next;
        mir_global_destroy(global);
        global = next;
    }

    free(module);
}

void mir_module_add_function(MIRModule *module, MIRFunction *func)
{
    if (!module || !func)
    {
        return;
    }

    func->next        = module->functions;
    module->functions = func;
    module->function_count++;
}

void mir_module_add_global(MIRModule *module, MIRGlobal *global)
{
    if (!module || !global)
    {
        return;
    }

    global->next    = module->globals;
    module->globals = global;
    module->global_count++;
}

MIRFunction *mir_module_get_function(MIRModule *module, const char *name)
{
    if (!module || !name)
    {
        return NULL;
    }

    for (MIRFunction *func = module->functions; func; func = func->next)
    {
        if (func->name && strcmp(func->name, name) == 0)
        {
            return func;
        }
    }

    return NULL;
}

MIRGlobal *mir_module_get_global(MIRModule *module, const char *name)
{
    if (!module || !name)
    {
        return NULL;
    }

    for (MIRGlobal *global = module->globals; global; global = global->next)
    {
        if (global->name && strcmp(global->name, name) == 0)
        {
            return global;
        }
    }

    return NULL;
}

void mir_module_merge(MIRModule *dst, MIRModule *src)
{
    if (!dst || !src)
    {
        return;
    }

    // transfer functions from src to dst (avoiding duplicates)
    MIRFunction *func = src->functions;
    while (func)
    {
        MIRFunction *next = func->next;

        // check if function already exists in dst
        if (!mir_module_get_function(dst, func->name))
        {
            // detach from src and add to dst
            func->next     = dst->functions;
            dst->functions = func;
            dst->function_count++;
        }

        func = next;
    }
    src->functions      = NULL;
    src->function_count = 0;

    // transfer globals from src to dst (avoiding duplicates)
    MIRGlobal *global = src->globals;
    while (global)
    {
        MIRGlobal *next = global->next;

        // check if global already exists in dst
        if (!mir_module_get_global(dst, global->name))
        {
            // detach from src and add to dst
            global->next = dst->globals;
            dst->globals = global;
            dst->global_count++;
        }

        global = next;
    }
    src->globals      = NULL;
    src->global_count = 0;
}
