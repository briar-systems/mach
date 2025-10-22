#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stddef.h>

// forward declarations
typedef struct Symbol      Symbol;
typedef struct AstNode     AstNode;
typedef struct SymbolTable SymbolTable;

typedef enum TypeKind
{
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_F16,
    TYPE_F32,
    TYPE_F64,
    TYPE_PTR,     // generic pointer
    TYPE_POINTER, // typed pointer
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_FUNCTION,
    TYPE_ALIAS, // type alias from 'def'
    TYPE_ERROR, // placeholder for failed type resolution
} TypeKind;

typedef struct Type
{
    TypeKind kind;
    size_t   size;      // size in bytes
    size_t   alignment; // alignment requirement
    char    *name;      // for named types (structs, unions, aliases)

    // generic specialization info
    Symbol       *generic_origin; // the generic symbol this was instantiated from
    struct Type **type_args;      // type arguments used for specialization
    size_t        type_arg_count; // number of type arguments

    union
    {
        // TYPE_POINTER
        struct
        {
            struct Type *base;
        } pointer;

        // TYPE_ARRAY
        struct
        {
            struct Type *elem_type;
            size_t       size;     // 0 for slices/fat pointers, >0 for fixed-size arrays
            bool         is_slice; // true for []T (fat pointer), false for [N]T (fixed-size)
        } array;

        // TYPE_STRUCT, TYPE_UNION
        struct
        {
            Symbol *fields;
            size_t  field_count;
        } composite;

        // TYPE_FUNCTION
        struct
        {
            struct Type  *return_type;
            struct Type **param_types;
            size_t        param_count;
            bool          is_variadic;
        } function;

        // TYPE_ALIAS
        struct
        {
            struct Type *target;
        } alias;
    };
} Type;

// type system initialization
void type_system_init(void);
void type_system_dnit(void);

// builtin type accessors
Type *type_u8(void);
Type *type_u16(void);
Type *type_u32(void);
Type *type_u64(void);
Type *type_i8(void);
Type *type_i16(void);
Type *type_i32(void);
Type *type_i64(void);
Type *type_f16(void);
Type *type_f32(void);
Type *type_f64(void);
Type *type_ptr(void);
Type *type_error(void);

// type constructors
Type *type_pointer_create(Type *base);
Type *type_array_create(Type *elem_type);                    // creates slice/fat pointer []T
Type *type_fixed_array_create(Type *elem_type, size_t size); // creates fixed-size [N]T
Type *type_struct_create(const char *name);
Type *type_union_create(const char *name);
Type *type_function_create(Type *return_type, Type **param_types, size_t param_count, bool is_variadic);
Type *type_alias_create(const char *name, Type *target);

// type operations
bool   type_equals(Type *a, Type *b);
bool   type_is_numeric(Type *type);
bool   type_is_integer(Type *type);
bool   type_is_float(Type *type);
bool   type_is_signed(Type *type);
bool   type_is_pointer_like(Type *type);
bool   type_is_truthy(Type *type); // true when type is the mach boolean (u8)
bool   type_is_error(Type *type);
bool   type_can_cast_to(Type *from, Type *to);
bool   type_can_assign_to(Type *from, Type *to);
size_t type_sizeof(Type *type);
size_t type_alignof(Type *type);

// builtin lookup
Type *type_lookup_builtin(const char *name);

// type resolution from AST
Type *type_resolve(AstNode *type_node, SymbolTable *symbol_table);
Type *type_resolve_alias(Type *type); // resolve alias to underlying type

// debug
void  type_print(Type *type);
char *type_to_string(Type *type);

#endif // TYPE_H
