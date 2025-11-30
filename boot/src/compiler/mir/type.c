#include "compiler/mir/type.h"
#include <stdlib.h>
#include <string.h>

MIRType *mir_type_create(MIRTypeKind kind)
{
    MIRType *type = malloc(sizeof(MIRType));
    if (!type) return NULL;
    
    memset(type, 0, sizeof(MIRType));
    type->kind = kind;
    
    // set default sizes for primitives
    switch (kind)
    {
        case MIR_TYPE_I8:
        case MIR_TYPE_U8:
            type->size = 1;
            type->align = 1;
            break;
        case MIR_TYPE_I16:
        case MIR_TYPE_U16:
            type->size = 2;
            type->align = 2;
            break;
        case MIR_TYPE_I32:
        case MIR_TYPE_U32:
        case MIR_TYPE_F32:
            type->size = 4;
            type->align = 4;
            break;
        case MIR_TYPE_I64:
        case MIR_TYPE_U64:
        case MIR_TYPE_F64:
        case MIR_TYPE_PTR:
            type->size = 8;
            type->align = 8;
            break;
        default:
            break;
    }
    
    return type;
}

MIRType *mir_type_create_array(MIRType *elem_type, size_t count)
{
    MIRType *type = mir_type_create(MIR_TYPE_ARRAY);
    if (!type) return NULL;
    
    type->elem_type = elem_type;
    type->elem_count = count;
    type->size = elem_type->size * count;
    type->align = elem_type->align;
    
    return type;
}

MIRType *mir_type_create_record(MIRType **fields, size_t count)
{
    MIRType *type = mir_type_create(MIR_TYPE_RECORD);
    if (!type) return NULL;
    
    type->record.fields = malloc(sizeof(MIRType*) * count);
    type->record.offsets = malloc(sizeof(size_t) * count);
    type->record.count = count;
    
    size_t current_offset = 0;
    size_t max_align = 1;
    
    for (size_t i = 0; i < count; i++)
    {
        type->record.fields[i] = fields[i];
        
        // align offset
        size_t align = fields[i]->align;
        if (align > max_align) max_align = align;
        
        if (current_offset % align != 0)
        {
            current_offset += align - (current_offset % align);
        }
        
        type->record.offsets[i] = current_offset;
        current_offset += fields[i]->size;
    }
    
    // align record size
    if (current_offset % max_align != 0)
    {
        current_offset += max_align - (current_offset % max_align);
    }
    
    type->size = current_offset;
    type->align = max_align;
    
    return type;
}

MIRType *mir_type_create_function(MIRType *ret_type, MIRType **params, size_t param_count, bool is_variadic)
{
    MIRType *type = mir_type_create(MIR_TYPE_FUNCTION);
    if (!type) return NULL;
    
    type->function.ret_type = ret_type;
    type->function.params = malloc(sizeof(MIRType*) * param_count);
    memcpy(type->function.params, params, sizeof(MIRType*) * param_count);
    type->function.param_count = param_count;
    type->function.is_variadic = is_variadic;
    
    type->size = 8; // function pointer size
    type->align = 8;
    
    return type;
}

void mir_type_destroy(MIRType *type)
{
    if (!type) return;
    
    if (type->kind == MIR_TYPE_RECORD)
    {
        free(type->record.fields);
        free(type->record.offsets);
    }
    else if (type->kind == MIR_TYPE_FUNCTION)
    {
        free(type->function.params);
    }
    
    free(type);
}
