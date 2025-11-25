#include "compiler/mir/function.h"
#include <stdlib.h>
#include <string.h>

MIRFunction *mir_function_create(const char *name, Type *type, bool is_exported)
{
    MIRFunction *func = malloc(sizeof(MIRFunction));
    if (!func)
    {
        return NULL;
    }

    func->name = name ? strdup(name) : NULL;
    func->type = type;
    func->is_exported = is_exported;
    func->params = NULL;
    func->param_count = 0;
    func->param_capacity = 0;
    func->first_block = NULL;
    func->last_block = NULL;
    func->block_count = 0;
    func->next_value_id = 0;
    func->next_block_id = 0;
    func->frame_size = 0;
    func->next = NULL;

    return func;
}

void mir_function_destroy(MIRFunction *func)
{
    if (!func)
    {
        return;
    }

    // free name
    if (func->name)
    {
        free(func->name);
    }

    // free parameters
    if (func->params)
    {
        for (size_t i = 0; i < func->param_count; i++)
        {
            mir_value_destroy(func->params[i]);
        }
        free(func->params);
    }

    // free blocks
    MIRBlock *block = func->first_block;
    while (block)
    {
        MIRBlock *next = block->next;
        mir_block_destroy(block);
        block = next;
    }

    free(func);
}

MIRBlock *mir_function_add_block(MIRFunction *func, const char *label)
{
    if (!func)
    {
        return NULL;
    }

    MIRBlock *block = mir_block_create(func->next_block_id++, label);
    if (!block)
    {
        return NULL;
    }

    if (!func->first_block)
    {
        func->first_block = block;
        func->last_block = block;
    }
    else
    {
        func->last_block->next = block;
        func->last_block = block;
    }

    func->block_count++;
    return block;
}

MIRBlock *mir_function_get_entry_block(MIRFunction *func)
{
    return func ? func->first_block : NULL;
}

MIRValue *mir_function_alloc_value(MIRFunction *func, Type *type, const char *name)
{
    if (!func)
    {
        return NULL;
    }

    return mir_value_create(func->next_value_id++, type, name);
}

MIRValue *mir_function_add_param(MIRFunction *func, Type *type, const char *name)
{
    if (!func)
    {
        return NULL;
    }

    MIRValue *param = mir_function_alloc_value(func, type, name);
    if (!param)
    {
        return NULL;
    }

    if (func->param_count >= func->param_capacity)
    {
        size_t new_capacity = func->param_capacity == 0 ? 4 : func->param_capacity * 2;
        MIRValue **new_params = realloc(func->params, new_capacity * sizeof(MIRValue *));
        if (!new_params)
        {
            mir_value_destroy(param);
            return NULL;
        }
        func->params = new_params;
        func->param_capacity = new_capacity;
    }

    func->params[func->param_count++] = param;
    return param;
}
