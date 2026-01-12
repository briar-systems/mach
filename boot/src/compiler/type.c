#include "compiler/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// primitive type singletons
static Type primitive_types[11] = {0};
static bool types_initialized   = false;

static Type *builtin_va_list_type = NULL;

static void init_primitive_types()
{
    if (types_initialized)
    {
        return;
    }

    // initialize primitive types
    primitive_types[TYPE_U8]  = (Type){.kind = TYPE_U8, .size = 1, .alignment = 1};
    primitive_types[TYPE_U16] = (Type){.kind = TYPE_U16, .size = 2, .alignment = 2};
    primitive_types[TYPE_U32] = (Type){.kind = TYPE_U32, .size = 4, .alignment = 4};
    primitive_types[TYPE_U64] = (Type){.kind = TYPE_U64, .size = 8, .alignment = 8};
    primitive_types[TYPE_I8]  = (Type){.kind = TYPE_I8, .size = 1, .alignment = 1};
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

    type->kind             = TYPE_POINTER;
    type->size             = 8;
    type->alignment        = 8;
    type->pointer.base     = base;
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

    type->kind            = TYPE_ARRAY;
    type->size            = elem_type ? elem_type->size * count : 0;
    type->alignment       = (elem_type && elem_type->alignment) ? elem_type->alignment : 1;
    type->array.elem_type = elem_type;
    type->array.count     = count;

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
    // function values are pointers to code (at least for stage1: sysv64).
    // keep this layout-safe so functions can appear in records as callbacks.
    Type *ptr_t                = type_get_primitive(TYPE_PTR);
    type->size                 = ptr_t ? ptr_t->size : 8;
    type->alignment            = ptr_t ? ptr_t->alignment : 8;
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

    type->kind                        = TYPE_STRUCT;
    type->structure.name              = name ? strdup(name) : NULL;
    type->structure.fields            = fields;
    type->structure.field_count       = field_count;
    type->structure.methods           = NULL; // initialize methods table
    type->structure.generic_args      = NULL;
    type->structure.generic_arg_count = 0;

    // calculate size and alignment
    size_t size      = 0;
    size_t alignment = 1;

    for (int i = 0; i < field_count; i++)
    {
        Type  *field_type  = fields[i].type;
        size_t field_size  = field_type ? field_type->size : 0;
        size_t field_align = (field_type && field_type->alignment) ? field_type->alignment : 1;

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

    type->size      = size;
    type->alignment = alignment;

    return type;
}

Type *type_get_builtin_va_list(void)
{
    if (builtin_va_list_type)
    {
        return builtin_va_list_type;
    }

    // NOTE: keep this in sync with the SysV x86_64 va_list implementation in the backend.
    // note: field names are for diagnostics only.
    TypeField *fields = calloc(4, sizeof(TypeField));
    if (!fields)
    {
        return NULL;
    }

    fields[0].name = strdup("gp_offset");
    fields[0].type = type_get_primitive(TYPE_U32);

    fields[1].name = strdup("fp_offset");
    fields[1].type = type_get_primitive(TYPE_U32);

    fields[2].name = strdup("overflow_arg_area");
    fields[2].type = type_get_primitive(TYPE_PTR);

    fields[3].name = strdup("reg_save_area");
    fields[3].type = type_get_primitive(TYPE_PTR);

    // ignore strdup failures; type_create_struct will still compute layout.
    builtin_va_list_type = type_create_struct("va_list", fields, 4);
    return builtin_va_list_type;
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
        return type_equals(a->pointer.base, b->pointer.base) && a->pointer.is_const == b->pointer.is_const;

    case TYPE_ARRAY:
        return a->array.count == b->array.count && type_equals(a->array.elem_type, b->array.elem_type);

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
        // nominal typing for structs: equal if they share the same name
        if (a->structure.name && b->structure.name)
        {
            return strcmp(a->structure.name, b->structure.name) == 0;
        }
        // if anonymous, compare structure
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

    case TYPE_UNION:
        // nominal typing for unions
        if (a->union_type.name && b->union_type.name)
        {
            return strcmp(a->union_type.name, b->union_type.name) == 0;
        }
        if (a->union_type.field_count != b->union_type.field_count)
        {
            return false;
        }
        for (int i = 0; i < a->union_type.field_count; i++)
        {
            if (!type_equals(a->union_type.fields[i].type, b->union_type.fields[i].type))
            {
                return false;
            }
        }
        return true;

    case TYPE_GENERIC_PARAM:
        // generic params are equal if they share the same name
        return strcmp(a->generic_param.name, b->generic_param.name) == 0;

    default:
        return false;
    }
}

bool type_can_assign_to(Type *from, Type *to)
{
    if (!from || !to)
    {
        return from == to;
    }

    // exact type match
    if (type_equals(from, to))
    {
        return true;
    }

    // *T -> &T: mutable pointer can be assigned to readonly pointer
    if (from->kind == TYPE_POINTER && to->kind == TYPE_POINTER)
    {
        // check base types match
        if (type_equals(from->pointer.base, to->pointer.base))
        {
            // *T -> &T is allowed (mutable to readonly)
            if (!from->pointer.is_const && to->pointer.is_const)
            {
                return true;
            }
        }
    }

    // untyped pointer TYPE_PTR can be assigned to any typed pointer
    if (from->kind == TYPE_PTR && to->kind == TYPE_POINTER)
    {
        return true;
    }

    // typed pointer can be assigned to TYPE_PTR
    if (from->kind == TYPE_POINTER && to->kind == TYPE_PTR)
    {
        return true;
    }

    return false;
}

Type *type_create_union(const char *name, TypeField *fields, int field_count)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind                         = TYPE_UNION;
    type->union_type.name              = name ? strdup(name) : NULL;
    type->union_type.fields            = fields;
    type->union_type.field_count       = field_count;
    type->union_type.methods           = NULL; // initialize methods table
    type->union_type.generic_args      = NULL;
    type->union_type.generic_arg_count = 0;

    // union size is the max of field sizes and alignment is the max of field alignments
    size_t size      = 0;
    size_t alignment = 1;

    for (int i = 0; i < field_count; i++)
    {
        Type *field_type = fields[i].type;
        if (field_type->size > size)
        {
            size = field_type->size;
        }
        if (field_type->alignment > alignment)
        {
            alignment = field_type->alignment;
        }
    }

    // align total size
    if (size % alignment != 0)
    {
        size += alignment - (size % alignment);
    }

    type->size      = size;
    type->alignment = alignment;

    return type;
}

Type *type_create_generic_param(const char *name)
{
    Type *type = malloc(sizeof(Type));
    if (!type)
    {
        return NULL;
    }

    type->kind               = TYPE_GENERIC_PARAM;
    type->size               = 0; // unknown size until instantiated
    type->alignment          = 0;
    type->generic_param.name = name ? strdup(name) : NULL;

    return type;
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

// mangle a type into Itanium-style encoding
// primitives: length-prefixed name (e.g., "3i64", "2u8")
// pointers: P<type> for mutable, K<type> for const/readonly
// arrays: A<count>_<elem_type>
// records/unions: length-prefixed name, with I...E for generic args
// returns number of chars written (not including null terminator)
int type_mangle(Type *type, char *buffer, size_t buffer_size)
{
    if (!type || !buffer || buffer_size == 0)
    {
        return 0;
    }

    int written = 0;

    switch (type->kind)
    {
    // primitives - use length-prefixed type names
    case TYPE_U8:
        written = snprintf(buffer, buffer_size, "2u8");
        break;
    case TYPE_U16:
        written = snprintf(buffer, buffer_size, "3u16");
        break;
    case TYPE_U32:
        written = snprintf(buffer, buffer_size, "3u32");
        break;
    case TYPE_U64:
        written = snprintf(buffer, buffer_size, "3u64");
        break;
    case TYPE_I8:
        written = snprintf(buffer, buffer_size, "2i8");
        break;
    case TYPE_I16:
        written = snprintf(buffer, buffer_size, "3i16");
        break;
    case TYPE_I32:
        written = snprintf(buffer, buffer_size, "3i32");
        break;
    case TYPE_I64:
        written = snprintf(buffer, buffer_size, "3i64");
        break;
    case TYPE_F32:
        written = snprintf(buffer, buffer_size, "3f32");
        break;
    case TYPE_F64:
        written = snprintf(buffer, buffer_size, "3f64");
        break;
    case TYPE_PTR:
        written = snprintf(buffer, buffer_size, "3ptr");
        break;

    case TYPE_POINTER:
    {
        // use 'P' for mutable pointers and 'K' for const/readonly pointers
        char prefix = type->pointer.is_const ? 'K' : 'P';
        written     = snprintf(buffer, buffer_size, "%c", prefix);
        if (written < (int)buffer_size && type->pointer.base)
        {
            written += type_mangle(type->pointer.base, buffer + written, buffer_size - written);
        }
        break;
    }

    case TYPE_ARRAY:
    {
        // arrays use the pattern 'A<count>_<elem_type>'
        written = snprintf(buffer, buffer_size, "A%zu_", type->array.count);
        if (written < (int)buffer_size && type->array.elem_type)
        {
            written += type_mangle(type->array.elem_type, buffer + written, buffer_size - written);
        }
        break;
    }

    case TYPE_STRUCT:
    {
        // length-prefixed struct name
        const char *name     = type->structure.name ? type->structure.name : "anon";
        size_t      name_len = strlen(name);
        written              = snprintf(buffer, buffer_size, "%zu%s", name_len, name);
        // note: generic args for structs would be appended by caller with I...E
        break;
    }

    case TYPE_UNION:
    {
        // length-prefixed union name
        const char *name     = type->union_type.name ? type->union_type.name : "anon";
        size_t      name_len = strlen(name);
        written              = snprintf(buffer, buffer_size, "%zu%s", name_len, name);
        break;
    }

    case TYPE_FUNCTION:
    {
        // functions use the pattern 'F<return_type><param_types>E'
        written = snprintf(buffer, buffer_size, "F");
        if (type->function.return_type && written < (int)buffer_size)
        {
            written += type_mangle(type->function.return_type, buffer + written, buffer_size - written);
        }
        else if (written < (int)buffer_size)
        {
            written += snprintf(buffer + written, buffer_size - written, "v"); // void
        }
        for (int i = 0; i < type->function.param_count && written < (int)buffer_size; i++)
        {
            if (type->function.param_types[i])
            {
                written += type_mangle(type->function.param_types[i], buffer + written, buffer_size - written);
            }
        }
        if (written < (int)buffer_size)
        {
            written += snprintf(buffer + written, buffer_size - written, "E");
        }
        break;
    }

    case TYPE_GENERIC_PARAM:
    {
        // should not appear in instantiated types, but handle gracefully
        const char *name     = type->generic_param.name ? type->generic_param.name : "T";
        size_t      name_len = strlen(name);
        written              = snprintf(buffer, buffer_size, "%zu%s", name_len, name);
        break;
    }

    default:
        written = snprintf(buffer, buffer_size, "?");
        break;
    }

    return written;
}
