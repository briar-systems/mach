#include "type.h"
#include "ast.h"
#include "symbol.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// builtin types storage
static Type *g_builtin_types[TYPE_PTR + 1] = {0};
static Type *g_error_type                  = NULL;

typedef struct PointerCacheEntry
{
    Type                     *base;
    Type                     *pointer;
    struct PointerCacheEntry *next;
} PointerCacheEntry;

static PointerCacheEntry *g_pointer_cache = NULL;

static size_t type_align_to(size_t value, size_t alignment)
{
    if (alignment <= 1)
    {
        return value;
    }

    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}

static void type_free_field_list(Symbol *fields)
{
    while (fields)
    {
        Symbol *next = fields->next;
        fields->next = NULL;
        symbol_destroy(fields);
        fields = next;
    }
}

// type info table for builtin types
static struct
{
    TypeKind    kind;
    size_t      size;
    size_t      alignment;
    const char *name;
} type_info_table[] = {
    {TYPE_U8, 1, 1, "u8"},
    {TYPE_U16, 2, 2, "u16"},
    {TYPE_U32, 4, 4, "u32"},
    {TYPE_U64, 8, 8, "u64"},
    {TYPE_I8, 1, 1, "i8"},
    {TYPE_I16, 2, 2, "i16"},
    {TYPE_I32, 4, 4, "i32"},
    {TYPE_I64, 8, 8, "i64"},
    {TYPE_F16, 2, 2, "f16"},
    {TYPE_F32, 4, 4, "f32"},
    {TYPE_F64, 8, 8, "f64"},
    {TYPE_PTR, 8, 8, "ptr"},
};

void type_system_init(void)
{
    // create builtin types
    for (size_t i = 0; i < sizeof(type_info_table) / sizeof(type_info_table[0]); i++)
    {
        Type *type           = malloc(sizeof(Type));
        type->kind           = type_info_table[i].kind;
        type->size           = type_info_table[i].size;
        type->alignment      = type_info_table[i].alignment;
        type->name           = strdup(type_info_table[i].name);
        type->generic_origin = NULL;
        type->type_args      = NULL;
        type->type_arg_count = 0;

        g_builtin_types[type->kind] = type;
    }

    if (!g_error_type)
    {
        g_error_type                 = malloc(sizeof(Type));
        g_error_type->kind           = TYPE_ERROR;
        g_error_type->size           = 0;
        g_error_type->alignment      = 1;
        g_error_type->name           = strdup("<error>");
        g_error_type->generic_origin = NULL;
        g_error_type->type_args      = NULL;
        g_error_type->type_arg_count = 0;
    }
}

void type_system_dnit(void)
{
    for (size_t i = 0; i < sizeof(g_builtin_types) / sizeof(g_builtin_types[0]); i++)
    {
        if (g_builtin_types[i])
        {
            free(g_builtin_types[i]->name);
            free(g_builtin_types[i]);
            g_builtin_types[i] = NULL;
        }
    }

    PointerCacheEntry *entry = g_pointer_cache;
    while (entry)
    {
        PointerCacheEntry *next = entry->next;
        free(entry->pointer);
        free(entry);
        entry = next;
    }
    g_pointer_cache = NULL;

    if (g_error_type)
    {
        free(g_error_type->name);
        free(g_error_type);
        g_error_type = NULL;
    }
}

// builtin type accessors
Type *type_u8(void)
{
    return g_builtin_types[TYPE_U8];
}
Type *type_u16(void)
{
    return g_builtin_types[TYPE_U16];
}
Type *type_u32(void)
{
    return g_builtin_types[TYPE_U32];
}
Type *type_u64(void)
{
    return g_builtin_types[TYPE_U64];
}
Type *type_i8(void)
{
    return g_builtin_types[TYPE_I8];
}
Type *type_i16(void)
{
    return g_builtin_types[TYPE_I16];
}
Type *type_i32(void)
{
    return g_builtin_types[TYPE_I32];
}
Type *type_i64(void)
{
    return g_builtin_types[TYPE_I64];
}
Type *type_f16(void)
{
    return g_builtin_types[TYPE_F16];
}
Type *type_f32(void)
{
    return g_builtin_types[TYPE_F32];
}
Type *type_f64(void)
{
    return g_builtin_types[TYPE_F64];
}
Type *type_ptr(void)
{
    return g_builtin_types[TYPE_PTR];
}

Type *type_error(void)
{
    return g_error_type;
}

Type *type_lookup_builtin(const char *name)
{
    if (!name)
        return NULL;

    for (size_t i = 0; i < sizeof(type_info_table) / sizeof(type_info_table[0]); i++)
    {
        if (strcmp(name, type_info_table[i].name) == 0)
            return g_builtin_types[type_info_table[i].kind];
    }

    return NULL;
}

Type *type_pointer_create(Type *base)
{
    for (PointerCacheEntry *entry = g_pointer_cache; entry; entry = entry->next)
    {
        if (entry->base == base)
        {
            return entry->pointer;
        }
    }

    Type *type           = malloc(sizeof(Type));
    type->generic_origin = NULL;
    type->type_args      = NULL;
    type->type_arg_count = 0;
    type->kind           = TYPE_POINTER;
    type->size           = 8; // 64-bit pointers
    type->alignment      = 8;
    type->name           = NULL;
    type->pointer.base   = base;

    PointerCacheEntry *entry = malloc(sizeof(PointerCacheEntry));
    entry->base              = base;
    entry->pointer           = type;
    entry->next              = g_pointer_cache;
    g_pointer_cache          = entry;

    return type;
}

Type *type_array_create(Type *elem_type)
{
    Type *type            = malloc(sizeof(Type));
    type->kind            = TYPE_ARRAY;
    type->size            = 16; // fat pointer: {void *data, u64 len}
    type->alignment       = 8;
    type->name            = NULL;
    type->generic_origin  = NULL;
    type->type_args       = NULL;
    type->type_arg_count  = 0;
    type->array.elem_type = elem_type;
    type->array.size      = 0;
    type->array.is_slice  = true; // slice/fat pointer []T
    return type;
}

Type *type_fixed_array_create(Type *elem_type, size_t size)
{
    Type *type            = malloc(sizeof(Type));
    type->kind            = TYPE_ARRAY;
    type->size            = elem_type->size * size;
    type->alignment       = elem_type->alignment;
    type->name            = NULL;
    type->generic_origin  = NULL;
    type->type_args       = NULL;
    type->type_arg_count  = 0;
    type->array.elem_type = elem_type;
    type->array.size      = size;
    type->array.is_slice  = false; // fixed-size [N]T
    return type;
}

Type *type_struct_create(const char *name)
{
    Type *type                  = malloc(sizeof(Type));
    type->kind                  = TYPE_STRUCT;
    type->size                  = 0; // calculated later
    type->alignment             = 1; // calculated later
    type->name                  = name ? strdup(name) : NULL;
    type->generic_origin        = NULL;
    type->type_args             = NULL;
    type->type_arg_count        = 0;
    type->composite.fields      = NULL;
    type->composite.field_count = 0;
    return type;
}

Type *type_union_create(const char *name)
{
    Type *type                  = malloc(sizeof(Type));
    type->kind                  = TYPE_UNION;
    type->size                  = 0; // calculated later
    type->alignment             = 1; // calculated later
    type->name                  = name ? strdup(name) : NULL;
    type->generic_origin        = NULL;
    type->type_args             = NULL;
    type->type_arg_count        = 0;
    type->composite.fields      = NULL;
    type->composite.field_count = 0;
    return type;
}

Type *type_function_create(Type *return_type, Type **param_types, size_t param_count, bool is_variadic)
{
    Type *type                 = malloc(sizeof(Type));
    type->kind                 = TYPE_FUNCTION;
    type->size                 = 8; // function pointers are 8 bytes
    type->alignment            = 8;
    type->name                 = NULL;
    type->generic_origin       = NULL;
    type->type_args            = NULL;
    type->type_arg_count       = 0;
    type->function.return_type = return_type;
    type->function.param_count = param_count;
    type->function.is_variadic = is_variadic;

    if (param_count > 0)
    {
        type->function.param_types = malloc(sizeof(Type *) * param_count);
        memcpy(type->function.param_types, param_types, sizeof(Type *) * param_count);
    }
    else
    {
        type->function.param_types = NULL;
    }

    return type;
}

Type *type_alias_create(const char *name, Type *target)
{
    Type *type           = malloc(sizeof(Type));
    type->kind           = TYPE_ALIAS;
    type->size           = target ? target->size : 0;
    type->alignment      = target ? target->alignment : 0;
    type->name           = strdup(name);
    type->generic_origin = NULL;
    type->type_args      = NULL;
    type->type_arg_count = 0;
    type->alias.target   = target;
    return type;
}

bool type_equals(Type *a, Type *b)
{
    if (a == b)
        return true;
    if (!a || !b)
        return false;

    // resolve aliases
    while (a->kind == TYPE_ALIAS)
        a = a->alias.target;
    while (b->kind == TYPE_ALIAS)
        b = b->alias.target;

    if (a->kind == TYPE_ERROR || b->kind == TYPE_ERROR)
        return false;

    if (a->kind != b->kind)
        return false;

    switch (a->kind)
    {
    case TYPE_POINTER:
        return type_equals(a->pointer.base, b->pointer.base);

    case TYPE_ARRAY:
        return type_equals(a->array.elem_type, b->array.elem_type);

    case TYPE_FUNCTION:
        if (a->function.param_count != b->function.param_count)
            return false;
        if (a->function.is_variadic != b->function.is_variadic)
            return false;
        if (!type_equals(a->function.return_type, b->function.return_type))
            return false;

        for (size_t i = 0; i < a->function.param_count; i++)
        {
            if (!type_equals(a->function.param_types[i], b->function.param_types[i]))
            {
                return false;
            }
        }
        return true;

    case TYPE_STRUCT:
    case TYPE_UNION:
        // for named types, compare by name
        if (a->name && b->name)
        {
            return strcmp(a->name, b->name) == 0;
        }
        // for anonymous types, they're only equal if they're the same object
        return a == b;

    default:
        return true; // builtin types with same kind are equal
    }
}

bool type_is_numeric(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    return type_is_integer(type) || type_is_float(type);
}

bool type_is_integer(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    return type->kind >= TYPE_U8 && type->kind <= TYPE_I64;
}

bool type_is_float(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    return type->kind >= TYPE_F16 && type->kind <= TYPE_F64;
}

bool type_is_signed(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    return (type->kind >= TYPE_I8 && type->kind <= TYPE_I64) || (type->kind >= TYPE_F16 && type->kind <= TYPE_F64);
}

bool type_is_pointer_like(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    // slices ([]T) are fat pointers and must not participate in pointer-only coercions
    return type->kind == TYPE_PTR || type->kind == TYPE_POINTER || type->kind == TYPE_FUNCTION;
}

bool type_is_truthy(Type *type)
{
    if (!type)
        return false;
    while (type->kind == TYPE_ALIAS)
        type = type->alias.target;

    // only raw u8 values (0 or 1 at runtime) are valid boolean conditions
    return type->kind == TYPE_U8;
}

bool type_is_error(Type *type)
{
    if (!type)
        return false;

    while (type->kind == TYPE_ALIAS && type->alias.target)
        type = type->alias.target;

    return type->kind == TYPE_ERROR;
}

bool type_can_cast_to(Type *from, Type *to)
{
    if (!from || !to)
        return false;
    while (from->kind == TYPE_ALIAS)
        from = from->alias.target;
    while (to->kind == TYPE_ALIAS)
        to = to->alias.target;

    if (from->kind == TYPE_ERROR || to->kind == TYPE_ERROR)
        return false;

    // array to pointer decay is not supported via cast - use ?array[0] instead
    if (from->kind == TYPE_ARRAY && (to->kind == TYPE_PTR || to->kind == TYPE_POINTER))
    {
        return false;
    }

    // same type
    if (type_equals(from, to))
        return true;

    // same size casting (reinterpretation)
    if (from->size == to->size)
        return true;

    // numeric conversions
    if (type_is_numeric(from) && type_is_numeric(to))
        return true;

    // pointer conversions
    if (type_is_pointer_like(from) && type_is_pointer_like(to))
        return true;

    // integer to pointer conversions (common in C)
    if (type_is_integer(from) && type_is_pointer_like(to))
        return true;

    // pointer to integer conversions
    if (type_is_pointer_like(from) && type_is_integer(to))
        return true;

    return false;
}

bool type_can_assign_to(Type *from, Type *to)
{
    if (!from || !to)
        return false;
    while (from->kind == TYPE_ALIAS)
        from = from->alias.target;
    while (to->kind == TYPE_ALIAS)
        to = to->alias.target;

    if (from->kind == TYPE_ERROR || to->kind == TYPE_ERROR)
        return false;

    // same type
    if (type_equals(from, to))
        return true;

    // numeric conversions (more restrictive than casting)
    if (type_is_numeric(from) && type_is_numeric(to))
    {
        bool from_is_float = type_is_float(from);
        bool to_is_float   = type_is_float(to);

        // no implicit conversions between int and float
        if (from_is_float != to_is_float)
            return false;

        // allow integer narrowing/widening; still guard floating-point to wider only
        if (!from_is_float)
            return true;

        return from->size <= to->size;
    }

    // pointer conversions (same pointer types, but not functions)
    if (type_is_pointer_like(from) && type_is_pointer_like(to) && from->kind != TYPE_FUNCTION && to->kind != TYPE_FUNCTION)
        return true;

    // array to pointer decay
    if (from->kind == TYPE_ARRAY && to->kind == TYPE_POINTER)
    {
        return type_equals(from->array.elem_type, to->pointer.base);
    }

    // NO implicit pointer ↔ integer conversions for assignments
    return false;
}

size_t type_sizeof(Type *type)
{
    return type->size;
}

size_t type_alignof(Type *type)
{
    return type->alignment;
}

Type *type_resolve(AstNode *type_node, SymbolTable *symbol_table)
{
    if (!type_node)
        return NULL;

    switch (type_node->kind)
    {
    case AST_TYPE_NAME:
    {
        const char *name = type_node->type_name.name;

        // check builtin types
        Type *builtin = type_lookup_builtin(name);
        if (builtin)
            return builtin;

        // look up user-defined types in symbol table
        if (symbol_table)
        {
            Symbol *symbol = symbol_lookup(symbol_table, name);
            if (symbol && symbol->kind == SYMBOL_TYPE)
            {
                return symbol->type;
            }
        }

        return NULL;
    }

    case AST_TYPE_PTR:
    {
        Type *base = type_resolve(type_node->type_ptr.base, symbol_table);
        if (!base)
            return NULL;
        return type_pointer_create(base);
    }

    case AST_TYPE_ARRAY:
    {
        Type *elem_type = type_resolve(type_node->type_array.elem_type, symbol_table);
        if (!elem_type)
            return NULL;
        // all arrays are fat pointers
        return type_array_create(elem_type);
    }

    case AST_TYPE_FUN:
    {
        Type *return_type = NULL;
        if (type_node->type_fun.return_type)
        {
            return_type = type_resolve(type_node->type_fun.return_type, symbol_table);
            if (!return_type)
            {
                return NULL; // failed to resolve return type
            }
        }

        size_t param_count = type_node->type_fun.params ? type_node->type_fun.params->count : 0;
        Type **param_types = NULL;

        if (param_count > 0)
        {
            param_types = malloc(sizeof(Type *) * param_count);
            for (size_t i = 0; i < param_count; i++)
            {
                AstNode *param = type_node->type_fun.params->items[i];
                // for function types, params are direct type nodes, not param statements
                param_types[i] = type_resolve(param, symbol_table);
                if (!param_types[i])
                {
                    free(param_types);
                    return NULL;
                }
            }
        }

        bool  is_variadic = type_node->type_fun.is_variadic;
        Type *func_type   = type_function_create(return_type, param_types, param_count, is_variadic);
        free(param_types); // function_create makes its own copy
        return func_type;
    }

    case AST_TYPE_STR:
    {
        if (type_node->type_str.name)
        {
            if (!symbol_table)
            {
                return NULL;
            }

            Symbol *symbol = symbol_lookup(symbol_table, type_node->type_str.name);
            if (symbol && symbol->kind == SYMBOL_TYPE)
            {
                return symbol->type;
            }

            return NULL;
        }

        Type   *struct_type = type_struct_create(NULL);
        Symbol *head        = NULL;
        Symbol *tail        = NULL;
        size_t  offset      = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        if (type_node->type_str.fields)
        {
            for (int i = 0; i < type_node->type_str.fields->count; i++)
            {
                AstNode *field_node = type_node->type_str.fields->items[i];
                Type    *field_type = type_resolve(field_node->field_stmt.type, symbol_table);
                if (!field_type)
                {
                    type_free_field_list(head);
                    free(struct_type->name);
                    free(struct_type);
                    return NULL;
                }

                Symbol *field_symbol = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);

                size_t field_align         = type_alignof(field_type);
                offset                     = type_align_to(offset, field_align);
                field_symbol->field.offset = offset;

                if (!head)
                {
                    head = tail = field_symbol;
                }
                else
                {
                    tail->next = field_symbol;
                    tail       = field_symbol;
                }

                offset += type_sizeof(field_type);
                if (field_align > max_align)
                {
                    max_align = field_align;
                }
                field_count++;
            }
        }

        offset    = type_align_to(offset, max_align);
        max_align = max_align ? max_align : 1;

        struct_type->size                  = offset;
        struct_type->alignment             = max_align;
        struct_type->composite.fields      = head;
        struct_type->composite.field_count = field_count;

        type_node->type = struct_type;
        return struct_type;
    }

    case AST_TYPE_UNI:
    {
        if (type_node->type_uni.name)
        {
            if (!symbol_table)
            {
                return NULL;
            }

            Symbol *symbol = symbol_lookup(symbol_table, type_node->type_uni.name);
            if (symbol && symbol->kind == SYMBOL_TYPE)
            {
                return symbol->type;
            }

            return NULL;
        }

        Type   *union_type  = type_union_create(NULL);
        Symbol *head        = NULL;
        Symbol *tail        = NULL;
        size_t  max_size    = 0;
        size_t  max_align   = 1;
        size_t  field_count = 0;

        if (type_node->type_uni.fields)
        {
            for (int i = 0; i < type_node->type_uni.fields->count; i++)
            {
                AstNode *field_node = type_node->type_uni.fields->items[i];
                Type    *field_type = type_resolve(field_node->field_stmt.type, symbol_table);
                if (!field_type)
                {
                    type_free_field_list(head);
                    free(union_type->name);
                    free(union_type);
                    return NULL;
                }

                Symbol *field_symbol       = symbol_create(SYMBOL_FIELD, field_node->field_stmt.name, field_type, field_node);
                field_symbol->field.offset = 0;

                if (!head)
                {
                    head = tail = field_symbol;
                }
                else
                {
                    tail->next = field_symbol;
                    tail       = field_symbol;
                }

                size_t field_size  = type_sizeof(field_type);
                size_t field_align = type_alignof(field_type);
                if (field_size > max_size)
                {
                    max_size = field_size;
                }
                if (field_align > max_align)
                {
                    max_align = field_align;
                }
                field_count++;
            }
        }

        max_size  = type_align_to(max_size, max_align);
        max_align = max_align ? max_align : 1;

        union_type->size                  = max_size;
        union_type->alignment             = max_align;
        union_type->composite.fields      = head;
        union_type->composite.field_count = field_count;

        type_node->type = union_type;
        return union_type;
    }

    default:
        return NULL;
    }
}

Type *type_resolve_alias(Type *type)
{
    if (!type)
        return NULL;

    // resolve alias chain to final type
    while (type->kind == TYPE_ALIAS)
    {
        type = type->alias.target;
        if (!type)
            return NULL;
    }

    return type;
}

void type_print(Type *type)
{
    char *str = type_to_string(type);
    if (str)
    {
        printf("%s", str);
        free(str);
    }
}

char *type_to_string(Type *type)
{
    if (!type)
        return strdup("(null)");

    char *result = malloc(256);
    switch (type->kind)
    {
    case TYPE_U8:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_U64:
    case TYPE_I8:
    case TYPE_I16:
    case TYPE_I32:
    case TYPE_I64:
    case TYPE_F16:
    case TYPE_F32:
    case TYPE_F64:
    case TYPE_PTR:
        snprintf(result, 256, "%s", type->name);
        break;

    case TYPE_POINTER:
        snprintf(result, 256, "*%s", type_to_string(type->pointer.base));
        break;

    case TYPE_ARRAY:
        snprintf(result, 256, "[]%s", type_to_string(type->array.elem_type));
        break;

    case TYPE_STRUCT:
        snprintf(result, 256, "str %s", type->name ? type->name : "(anonymous)");
        break;

    case TYPE_UNION:
        snprintf(result, 256, "uni %s", type->name ? type->name : "(anonymous)");
        break;

    case TYPE_FUNCTION:
        snprintf(result, 256, "fun(");
        for (size_t i = 0; i < type->function.param_count; i++)
        {
            if (i > 0)
                strcat(result, ", ");
            char *param_str = type_to_string(type->function.param_types[i]);
            strcat(result, param_str);
            free(param_str);
        }
        if (type->function.is_variadic)
        {
            if (type->function.param_count > 0)
                strcat(result, ", ...");
            else
                strcat(result, "...");
        }
        strcat(result, ") ");
        if (type->function.return_type)
        {
            char *return_str = type_to_string(type->function.return_type);
            strcat(result, return_str);
            free(return_str);
        }
        break;

    case TYPE_ALIAS:
        snprintf(result, 256, "%s", type->name);
        break;

    case TYPE_ERROR:
        snprintf(result, 256, "%s", type->name ? type->name : "<error>");
        break;
    }

    return result;
}
