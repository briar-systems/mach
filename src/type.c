#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TypeVisited
{
    Type **types;
    int len;
    int cap;
};

bool type_init(Type *type, TypeKind kind)
{
    if (!type)
    {
        return false;
    }

    type->kind = kind;

    return true;
}

void type_free(Type *type)
{
    if (!type)
        return;

    switch (type->kind)
    {
    case TYPE_ERROR:
        free(type->err.message);
        break;
    case TYPE_PTR:
        type_free(type->ptr.target);
        type->ptr.target = NULL;
        break;
    case TYPE_ARR:
        type_free(type->arr.element_type);
        type->arr.element_type = NULL;
        break;
    case TYPE_STR:
        free(type->str.name);
        type->str.name = NULL;

        for (int i = 0; i < type->str.field_count; i++)
        {
            type_free(type->str.field_types[i]);
            type->str.field_types[i] = NULL;

            if (type->str.field_names)
            {
                free(type->str.field_names[i]);
                type->str.field_names[i] = NULL;
            }
        }

        free(type->str.field_types);
        type->str.field_types = NULL;

        free(type->str.field_names);
        type->str.field_names = NULL;

        free(type->str.field_offsets);
        type->str.field_offsets = NULL;
        break;
    case TYPE_UNI:
        free(type->uni.name);
        type->uni.name = NULL;

        for (int i = 0; i < type->uni.field_count; i++)
        {
            type_free(type->uni.field_types[i]);
            type->uni.field_types[i] = NULL;

            if (type->uni.field_names)
            {
                free(type->uni.field_names[i]);
                type->uni.field_names[i] = NULL;
            }
        }

        free(type->uni.field_types);
        type->uni.field_types = NULL;

        free(type->uni.field_names);
        type->uni.field_names = NULL;
        break;
    case TYPE_FUN:
        free(type->fun.name);
        type->fun.name = NULL;

        type_free(type->fun.return_type);
        type->fun.return_type = NULL;

        for (int i = 0; i < type->fun.param_count; i++)
        {
            type_free(type->fun.param_types[i]);
            type->fun.param_types[i] = NULL;
            
            if (type->fun.param_names)
            {
                free(type->fun.param_names[i]);
                type->fun.param_names[i] = NULL;
            }
        }

        free(type->fun.param_types);
        type->fun.param_types = NULL;

        free(type->fun.param_names);
        type->fun.param_names = NULL;
        break;
    default:
        break;
    }

    free(type);
}

Type *type_make(TypeKind kind)
{
    Type *type = calloc(sizeof(Type), 1);
    if (!type_init(type, kind))
    {
        fprintf(stderr, "failed to initialize type\n");
        free(type);
        return NULL;
    }

    return type;
}

size_t primitive_size(TypeKind kind, Target target)
{
    TargetInfo info = target_info(target);

    switch (kind)
    {
    case TYPE_VOID:
        return 0;
    case TYPE_U8:
    case TYPE_I8:
        return 1;
    case TYPE_U16:
    case TYPE_I16:
        return 2;
    case TYPE_U32:
    case TYPE_I32:
    case TYPE_F32:
        return 4;
    case TYPE_U64:
    case TYPE_I64:
    case TYPE_F64:
        return 8;
    case TYPE_PTR:
        return info.size;
    default:
        return 0;
    }
}

size_t primitive_align(TypeKind kind, Target target)
{
    TargetInfo info = target_info(target);

    switch (kind)
    {
    case TYPE_VOID:
        return 1;
    case TYPE_U8:
    case TYPE_I8:
        return 1;
    case TYPE_U16:
    case TYPE_I16:
        return 2;
    case TYPE_U32:
    case TYPE_I32:
    case TYPE_F32:
        return 4;
    case TYPE_U64:
    case TYPE_I64:
    case TYPE_F64:
        return 8;
    case TYPE_PTR:
        return info.alignment;
    default:
        return 1;
    }
}

size_t type_size_internal(Type *type, Target target, TypeVisited *visited);
size_t type_align_internal(Type *type, Target target, TypeVisited *visited);

size_t type_size(Type *type, Target target)
{
    if (!type)
    {
        return 0;
    }

    TypeVisited *visited = calloc(sizeof(TypeVisited), 1);
    if (!visited)
    {
        fprintf(stderr, "failed to allocate memory for type visited set\n");
        return 0;
    }

    size_t result = type_size_internal(type, target, visited);
    type_visited_free(visited);

    return result;
}

size_t type_size_internal(Type *type, Target target, TypeVisited *visited)
{
    if (!type)
        return 0;

    // check for recursive types
    if (type_visited_contains(visited, type))
    {
        // recursive reference, return pointer size
        TargetInfo info = target_info(target);
        return info.size;
    }

    type_visited_add(visited, type);

    switch (type->kind)
    {
    case TYPE_VOID:
    case TYPE_U8:
    case TYPE_I8:
    case TYPE_U16:
    case TYPE_I16:
    case TYPE_U32:
    case TYPE_I32:
    case TYPE_F32:
    case TYPE_U64:
    case TYPE_I64:
    case TYPE_F64:
    case TYPE_PTR:
        return primitive_size(type->kind, target);

    case TYPE_ARR:
    {
        if (type->arr.len < 0)
        {
            TargetInfo info = target_info(target);
            return info.size + 8;
        }

        if (type->arr.len == 0)
        {
            return 0;
        }

        size_t elem_size = type_size_internal(type->arr.element_type, target, visited);
        size_t elem_align = type_align_internal(type->arr.element_type, target, visited);

        size_t total_size = elem_size * type->arr.len;
        size_t padding = (elem_align - (total_size % elem_align)) % elem_align;

        return total_size + padding;
    }

    case TYPE_STR:
    {
        if (type->str.field_count == 0)
        {
            return 1;
        }

        size_t total_size = 0;

        for (int i = 0; i < type->str.field_count; i++)
        {
            size_t field_align = type_align_internal(type->str.field_types[i], target, visited);
            total_size += (field_align - (total_size % field_align)) % field_align;

            if (type->str.field_offsets)
            {
                type->str.field_offsets[i] = total_size;
            }

            total_size += type_size_internal(type->str.field_types[i], target, visited);
        }

        size_t struct_align = type_align_internal(type, target, visited);
        size_t padding = (struct_align - (total_size % struct_align)) % struct_align;
        total_size += padding;

        return total_size;
    }

    case TYPE_UNI:
    {
        size_t max_size = 0;
        for (int i = 0; i < type->uni.field_count; i++)
        {
            size_t field_size = type_size_internal(type->uni.field_types[i], target, visited);
            if (field_size > max_size)
            {
                max_size = field_size;
            }
        }

        size_t union_align = type_align_internal(type, target, visited);
        size_t padding = (union_align - (max_size % union_align)) % union_align;

        return max_size + padding;
    }

    case TYPE_FUN:
        return primitive_size(TYPE_PTR, target);
    default:
        return 0;
    }
}

size_t type_align(Type *type, Target target)
{
    if (!type)
    {
        return 1;
    }

    TypeVisited *visited = calloc(sizeof(TypeVisited), 1);
    if (!visited)
    {
        fprintf(stderr, "failed to allocate memory for type visited set\n");
        return 1;
    }

    size_t result = type_align_internal(type, target, visited);
    type_visited_free(visited);

    return result;
}

size_t type_align_internal(Type *type, Target target, TypeVisited *visited)
{
    if (!type)
        return 1;

    // check for recursive types
    if (type_visited_contains(visited, type))
    {
        // recursive reference, return pointer alignment
        TargetInfo info = target_info(target);
        return info.alignment;
    }

    type_visited_add(visited, type);

    switch (type->kind)
    {
    case TYPE_VOID:
    case TYPE_U8:
    case TYPE_I8:
    case TYPE_U16:
    case TYPE_I16:
    case TYPE_U32:
    case TYPE_I32:
    case TYPE_F32:
    case TYPE_U64:
    case TYPE_I64:
    case TYPE_F64:
    case TYPE_PTR:
        return primitive_align(type->kind, target);

    case TYPE_ARR:
        if (type->arr.len < 0)
        {
            return primitive_align(TYPE_PTR, target);
        }
        else
        {
            return type_align_internal(type->arr.element_type, target, visited);
        }

    case TYPE_STR:
    {
        size_t max_align = 1;
        for (int i = 0; i < type->str.field_count; i++)
        {
            size_t field_align = type_align_internal(type->str.field_types[i], target, visited);
            if (field_align > max_align)
            {
                max_align = field_align;
            }
        }
        return max_align;
    }

    case TYPE_UNI:
    {
        size_t max_align = 1;
        for (int i = 0; i < type->uni.field_count; i++)
        {
            size_t field_align = type_align_internal(type->uni.field_types[i], target, visited);
            if (field_align > max_align)
            {
                max_align = field_align;
            }
        }
        return max_align;
    }

    case TYPE_FUN:
        return primitive_align(TYPE_PTR, target);

    default:
        return 1;
    }
}

void type_compute_offsets(Type *type, Target target)
{
    if (!type || type->kind != TYPE_STR || !type->str.field_count)
    {
        return;
    }

    if (!type->str.field_offsets)
    {
        type->str.field_offsets = calloc(type->str.field_count, sizeof(size_t));
        if (!type->str.field_offsets)
        {
            return;
        }
    }

    size_t offset = 0;

    for (int i = 0; i < type->str.field_count; i++)
    {
        size_t field_align = type_align(type->str.field_types[i], target);
        offset += (field_align - (offset % field_align)) % field_align;

        type->str.field_offsets[i] = offset;
        offset += type_size(type->str.field_types[i], target);
    }
}

bool type_equals(Type *a, Type *b)
{
    if (!a || !b || a->kind != b->kind)
    {
        return false;
    }

    switch (a->kind)
    {
    case TYPE_VOID:
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
        return true;

    case TYPE_PTR:
        return type_equals(a->ptr.target, b->ptr.target);

    case TYPE_ARR:
        return a->arr.len == b->arr.len && type_equals(a->arr.element_type, b->arr.element_type);

    case TYPE_STR:
        if (a->str.field_count != b->str.field_count || (a->str.name && b->str.name && strcmp(a->str.name, b->str.name) != 0))
        {
            return false;
        }

        for (int i = 0; i < a->str.field_count; i++)
        {
            if (!type_equals(a->str.field_types[i], b->str.field_types[i]) || (a->str.field_names[i] && b->str.field_names[i] && strcmp(a->str.field_names[i], b->str.field_names[i]) != 0))
            {
                return false;
            }
        }
        return true;

    case TYPE_UNI:
        if (a->uni.field_count != b->uni.field_count || (a->uni.name && b->uni.name && strcmp(a->uni.name, b->uni.name) != 0))
        {
            return false;
        }

        for (int i = 0; i < a->uni.field_count; i++)
        {
            if (!type_equals(a->uni.field_types[i], b->uni.field_types[i]) || (a->uni.field_names[i] && b->uni.field_names[i] && strcmp(a->uni.field_names[i], b->uni.field_names[i]) != 0))
            {
                return false;
            }
        }
        return true;

    case TYPE_FUN:
        if (!type_equals(a->fun.return_type, b->fun.return_type) || a->fun.param_count != b->fun.param_count)
        {
            return false;
        }

        for (int i = 0; i < a->fun.param_count; i++)
        {
            if (!type_equals(a->fun.param_types[i], b->fun.param_types[i]))
            {
                return false;
            }
        }
        return true;

    default:
        return false;
    }
}

const char *type_kind_to_string(TypeKind kind)
{
    switch (kind)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_U8:
        return "u8";
    case TYPE_U16:
        return "u16";
    case TYPE_U32:
        return "u32";
    case TYPE_U64:
        return "u64";
    case TYPE_I8:
        return "i8";
    case TYPE_I16:
        return "i16";
    case TYPE_I32:
        return "i32";
    case TYPE_I64:
        return "i64";
    case TYPE_F32:
        return "f32";
    case TYPE_F64:
        return "f64";
    case TYPE_PTR:
        return "ptr";
    case TYPE_ARR:
        return "arr";
    case TYPE_STR:
        return "struct";
    case TYPE_UNI:
        return "union";
    case TYPE_FUN:
        return "fun";
    default:
        return "<unknown>";
    }
}

bool type_visited_init(TypeVisited *tv)
{
    if (!tv)
    {
        return false;
    }

    tv->types = NULL;
    tv->len = 0;
    tv->cap = 0;

    return true;
}

void type_visited_free(TypeVisited *tv)
{
    if (!tv)
    {
        return;
    }

    free(tv->types);
    tv->types = NULL;

    free(tv);
}

bool type_visited_contains(TypeVisited *tv, Type *type)
{
    for (int i = 0; i < tv->len; i++)
    {
        if (tv->types[i] == type)
        {
            return true;
        }
    }

    return false;
}

void type_visited_add(TypeVisited *tv, Type *type)
{
    if (tv->len >= tv->cap)
    {
        int new_cap = tv->cap == 0 ? 8 : tv->cap * 2;
        Type **new_types = realloc(tv->types, new_cap * sizeof(Type *));
        if (!new_types)
            return;

        tv->types = new_types;
        tv->cap = new_cap;
    }

    tv->types[tv->len++] = type;
}

char *type_to_string(Type *type)
{
    if (!type)
    {
        return strdup("<null>");
    }

    char *buffer = malloc(1024);
    if (!buffer)
    {
        return strdup("<memory error>");
    }

    TypeVisited *visited = calloc(sizeof(TypeVisited), 1);
    if (!visited)
    {
        free(buffer);
        return strdup("<memory error>");
    }

    type_to_string_internal(type, buffer, 1024, visited);
    type_visited_free(visited);

    size_t len = strlen(buffer) + 1;
    char *result = realloc(buffer, len);
    return result ? result : buffer;
}

int type_to_string_internal(Type *type, char *buffer, size_t buffer_size, TypeVisited *visited)
{
    if (!type || buffer_size <= 1)
    {
        if (buffer_size > 0)
        {
            buffer[0] = '\0';
        }

        return 0;
    }

    if (type_visited_contains(visited, type))
    {
        return snprintf(buffer, buffer_size, "<recursive:%s>", type_kind_to_string(type->kind));
    }

    type_visited_add(visited, type);

    switch (type->kind)
    {
    case TYPE_VOID:
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
        return snprintf(buffer, buffer_size, "%s", type_kind_to_string(type->kind));

    case TYPE_PTR:
    {
        int written = snprintf(buffer, buffer_size, "@");
        if (written < 0 || (size_t)written >= buffer_size)
        {
            return written;
        }

        return written + type_to_string_internal(type->ptr.target, buffer + written, buffer_size - written, visited);
    }

    case TYPE_ARR:
    {
        int written = snprintf(buffer, buffer_size, "[");
        if (written < 0 || (size_t)written >= buffer_size)
        {
            return written;
        }

        if (type->arr.len >= 0)
        {
            int len_written = snprintf(buffer + written, buffer_size - written, "%d", type->arr.len);
            if (len_written < 0 || (size_t)(written + len_written) >= buffer_size)
            {
                return written + len_written;
            }
            written += len_written;
        }

        if (written < (int)buffer_size - 1)
        {
            buffer[written++] = ']';
            buffer[written] = '\0';
        }

        if ((size_t)written < buffer_size)
        {
            int elem_written = type_to_string_internal(type->arr.element_type, buffer + written, buffer_size - written, visited);

            if (elem_written < 0 || (size_t)(written + elem_written) >= buffer_size)
            {
                return written + elem_written;
            }
            written += elem_written;
        }

        return written;
    }

    case TYPE_STR:
    {
        int written;
        if (type->str.name)
        {
            written = snprintf(buffer, buffer_size, "struct %s {", type->str.name);
        }
        else
        {
            written = snprintf(buffer, buffer_size, "struct {");
        }

        if (written < 0 || (size_t)written >= buffer_size)
        {
            return written;
        }

        for (int i = 0; i < type->str.field_count; i++)
        {
            if (i > 0)
            {
                if ((size_t)written < buffer_size - 2)
                {
                    buffer[written++] = ',';
                    buffer[written++] = ' ';
                    buffer[written] = '\0';
                }
                else
                {
                    break;
                }
            }

            if (type->str.field_names && type->str.field_names[i])
            {
                int name_written = snprintf(buffer + written, buffer_size - written, "%s: ", type->str.field_names[i]);

                if (name_written < 0 || (size_t)(written + name_written) >= buffer_size)
                {
                    return written + name_written;
                }
                written += name_written;
            }

            int field_written = type_to_string_internal(type->str.field_types[i], buffer + written, buffer_size - written, visited);

            if (field_written < 0 || (size_t)(written + field_written) >= buffer_size)
            {
                return written + field_written;
            }
            written += field_written;
        }

        if ((size_t)written < buffer_size - 1)
        {
            buffer[written++] = '}';
            buffer[written] = '\0';
        }
        else
        {
            return written;
        }

        return written;
    }

    case TYPE_UNI:
    {
        int written;
        if (type->uni.name)
        {
            written = snprintf(buffer, buffer_size, "union %s {", type->uni.name);
        }
        else
        {
            written = snprintf(buffer, buffer_size, "union {");
        }

        if (written < 0 || (size_t)written >= buffer_size)
        {
            return written;
        }

        for (int i = 0; i < type->uni.field_count; i++)
        {
            if (i > 0)
            {
                if ((size_t)written < buffer_size - 2)
                {
                    buffer[written++] = ',';
                    buffer[written++] = ' ';
                    buffer[written] = '\0';
                }
                else
                {
                    break;
                }
            }

            if (type->uni.field_names && type->uni.field_names[i])
            {
                int name_written = snprintf(buffer + written, buffer_size - written, "%s: ", type->uni.field_names[i]);

                if (name_written < 0 || (size_t)(written + name_written) >= buffer_size)
                {
                    return written + name_written;
                }
                written += name_written;
            }

            int field_written = type_to_string_internal(type->uni.field_types[i], buffer + written, buffer_size - written, visited);

            if (field_written < 0 || (size_t)(written + field_written) >= buffer_size)
            {
                return written + field_written;
            }
            written += field_written;
        }

        if ((size_t)written < buffer_size - 1)
        {
            buffer[written++] = '}';
            buffer[written] = '\0';
        }
        else
        {
            return written;
        }

        return written;
    }

    case TYPE_FUN:
    {
        int written = snprintf(buffer, buffer_size, "fun(");
        if (written < 0 || (size_t)written >= buffer_size)
        {
            return written;
        }

        for (int i = 0; i < type->fun.param_count; i++)
        {
            if (i > 0)
            {
                if ((size_t)written < buffer_size - 2)
                {
                    buffer[written++] = ',';
                    buffer[written++] = ' ';
                    buffer[written] = '\0';
                }
                else
                {
                    break;
                }
            }

            if (type->fun.param_names && type->fun.param_names[i])
            {
                int name_written = snprintf(buffer + written, buffer_size - written, "%s: ", type->fun.param_names[i]);

                if (name_written < 0 || (size_t)(written + name_written) >= buffer_size)
                {
                    return written + name_written;
                }
                written += name_written;
            }

            int param_written = type_to_string_internal(type->fun.param_types[i], buffer + written, buffer_size - written, visited);

            if (param_written < 0 || (size_t)(written + param_written) >= buffer_size)
            {
                return written + param_written;
            }
            written += param_written;
        }

        if ((size_t)written < buffer_size - 2)
        {
            buffer[written++] = ')';
            buffer[written] = '\0';
        }
        else
        {
            return written;
        }

        if (type->fun.return_type)
        {
            int arrow_written = snprintf(buffer + written, buffer_size - written, " -> ");
            if (arrow_written < 0 || (size_t)(written + arrow_written) >= buffer_size)
            {
                return written + arrow_written;
            }
            written += arrow_written;

            int ret_written = type_to_string_internal(type->fun.return_type, buffer + written, buffer_size - written, visited);

            if (ret_written < 0 || (size_t)(written + ret_written) >= buffer_size)
            {
                return written + ret_written;
            }
            written += ret_written;
        }

        return written;
    }

    default:
        return snprintf(buffer, buffer_size, "<unknown:%d>", type->kind);
    }
}
