#include "compiler/type.h"
#include <stdlib.h>
#include <string.h>

// primitive type singletons
static Type primitive_types[11] = {0};
static bool types_initialized = false;

static void init_primitive_types()
{
    if (types_initialized)
    {
        return;
    }

    // initialize primitive types
    primitive_types[TYPE_U8]  = (Type){.kind = TYPE_U8,  .size = 1, .alignment = 1};
    primitive_types[TYPE_U16] = (Type){.kind = TYPE_U16, .size = 2, .alignment = 2};
    primitive_types[TYPE_U32] = (Type){.kind = TYPE_U32, .size = 4, .alignment = 4};
    primitive_types[TYPE_U64] = (Type){.kind = TYPE_U64, .size = 8, .alignment = 8};
    primitive_types[TYPE_I8]  = (Type){.kind = TYPE_I8,  .size = 1, .alignment = 1};
    primitive_types[TYPE_I16] = (Type){.kind = TYPE_I16, .size = 2, .alignment = 2};
    primitive_types[TYPE_I32] = (Type){.kind = TYPE_I32, .size = 4, .alignment = 4};
    primitive_types[TYPE_I64] = (Type){.kind = TYPE_I64, .size = 8, .alignment = 8};
    primitive_types[TYPE_F32] = (Type){.kind = TYPE_F32, .size = 4, .alignment = 4};
    primitive_types[TYPE_F64] = (Type){.kind = TYPE_F64, .size = 8, .alignment = 8};
    primitive_types[TYPE_PTR] = (Type){.kind = TYPE_PTR, .size = 8, .alignment = 8};

    types_initialized = true;
}

Type *type_get_primitive(TypeKind kind)
{
    init_primitive_types();

    if (kind >= TYPE_U8 && kind <= TYPE_PTR)
    {
        return &primitive_types[kind];
    }

    return NULL;
}

Type *type_create_pointer(Type *base, bool is_const)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind = TYPE_POINTER;
    type->size = 8;
    type->alignment = 8;
    type->pointer.base = base;
    type->pointer.is_const = is_const;

    return type;
}

Type *type_create_array(Type *elem_type, size_t count)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind = TYPE_ARRAY;
    type->size = elem_type ? elem_type->size * count : 0;
    type->alignment = elem_type ? elem_type->alignment : 1;
    type->array.elem_type = elem_type;
    type->array.count = count;

    return type;
}

Type *type_create_function(Type *return_type, Type **param_types, int param_count)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind = TYPE_FUNCTION;
    type->size = 0; // functions don't have a size
    type->alignment = 0;
    type->function.return_type = return_type;
    type->function.param_types = param_types;
    type->function.param_count = param_count;

    return type;
}

Type *type_create_struct(const char *name, TypeField *fields, int field_count)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind = TYPE_STRUCT;
    type->structure.name = name ? strdup(name) : NULL;
    type->structure.fields = fields;
    type->structure.field_count = field_count;

    // calculate size and alignment
    size_t size = 0;
    size_t alignment = 1;

    for (int i = 0; i < field_count; i++)
    {
        Type *field_type = fields[i].type;
        size_t field_size = field_type->size;
        size_t field_align = field_type->alignment;

        // update struct alignment
        if (field_align > alignment)
        {
            alignment = field_align;
        }

        // add padding
        if (size % field_align != 0)
        {
            size += field_align - (size % field_align);
        }

        // set field offset
        fields[i].offset = size;
        size += field_size;
    }

    // align total size
    if (size % alignment != 0)
    {
        size += alignment - (size % alignment);
    }

    type->size = size;
    type->alignment = alignment;

    return type;
}

bool type_equals(Type *a, Type *b)
{
    if (!a || !b)
    {
        return a == b;
    }

    if (a->kind != b->kind)
    {
        return false;
    }

    switch (a->kind)
    {
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64:
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_F32:
    case TYPE_F64:
    case TYPE_PTR:
        return true; // primitives are equal if kinds match

    case TYPE_POINTER:
        return type_equals(a->pointer.base, b->pointer.base) &&
               a->pointer.is_const == b->pointer.is_const;

    case TYPE_ARRAY:
        return a->array.count == b->array.count &&
               type_equals(a->array.elem_type, b->array.elem_type);

    case TYPE_FUNCTION:
        if (!type_equals(a->function.return_type, b->function.return_type))
        {
            return false;
        }
        if (a->function.param_count != b->function.param_count)
        {
            return false;
        }
        for (int i = 0; i < a->function.param_count; i++)
        {
            if (!type_equals(a->function.param_types[i], b->function.param_types[i]))
            {
                return false;
            }
        }
        return true;

    case TYPE_STRUCT:
        // Nominal typing for structs: equal if they have the same name
        if (a->structure.name && b->structure.name)
        {
            return strcmp(a->structure.name, b->structure.name) == 0;
        }
        // If anonymous, check structure
        if (a->structure.field_count != b->structure.field_count)
        {
            return false;
        }
        for (int i = 0; i < a->structure.field_count; i++)
        {
            if (!type_equals(a->structure.fields[i].type, b->structure.fields[i].type))
            {
                return false;
            }
        }
        return true;

    default:
        return false;
    }
}

bool type_is_integer(Type *t)
{
    if (!t)
    {
        return false;
    }

    return t->kind >= TYPE_U8 && t->kind <= TYPE_I64;
}

bool type_is_float(Type *t)
{
    if (!t)
    {
        return false;
    }

    return t->kind == TYPE_F32 || t->kind == TYPE_F64;
}

bool type_is_numeric(Type *t)
{
    return type_is_integer(t) || type_is_float(t);
}
