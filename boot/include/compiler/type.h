#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stddef.h>

// type kinds
typedef enum TypeKind
{
    // primitives
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_PTR,      // untyped pointer
    
    // compound types
    TYPE_POINTER,  // typed pointer
    TYPE_ARRAY,    // fixed-size array
    TYPE_FUNCTION, // function type
    TYPE_STRUCT,   // record
    TYPE_UNION,    // union
} TypeKind;

typedef struct Type Type;

typedef struct TypeField
{
    char *name;
    Type *type;
    size_t offset;
} TypeField;

struct Type
{
    TypeKind kind;
    size_t   size;      // size in bytes (0 if unknown)
    size_t   alignment; // alignment requirement
    
    union
    {
        // TYPE_POINTER
        struct
        {
            Type *base;
            bool  is_const; // for & (read-only) vs * (mutable)
        } pointer;
        
        // TYPE_ARRAY
        struct
        {
            Type  *elem_type;
            size_t count; // element count
        } array;
        
        // TYPE_FUNCTION
        struct
        {
            Type *return_type;
            Type **param_types;
            int    param_count;
        } function;

        // TYPE_STRUCT
        struct
        {
            char *name;
            TypeField *fields;
            int field_count;
        } structure;
    };
};

// get primitive type (cached/singleton)
Type *type_get_primitive(TypeKind kind);

// construct compound types
Type *type_create_pointer(Type *base, bool is_const);
Type *type_create_array(Type *elem_type, size_t count);
Type *type_create_function(Type *return_type, Type **param_types, int param_count);
Type *type_create_struct(const char *name, TypeField *fields, int field_count);

// type checking
bool type_equals(Type *a, Type *b);
bool type_is_integer(Type *t);
bool type_is_float(Type *t);
bool type_is_numeric(Type *t);

#endif // TYPE_H
