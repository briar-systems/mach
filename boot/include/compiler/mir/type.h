#ifndef MIR_TYPE_H
#define MIR_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum MIRTypeKind
{
    MIR_TYPE_VOID,
    MIR_TYPE_I8,
    MIR_TYPE_U8,
    MIR_TYPE_I16,
    MIR_TYPE_U16,
    MIR_TYPE_I32,
    MIR_TYPE_U32,
    MIR_TYPE_I64,
    MIR_TYPE_U64,
    MIR_TYPE_F32,
    MIR_TYPE_F64,
    MIR_TYPE_PTR,
    MIR_TYPE_ARRAY,
    MIR_TYPE_STRUCT,
    MIR_TYPE_FUNCTION
} MIRTypeKind;

typedef struct MIRType MIRType;

struct MIRType
{
    MIRTypeKind kind;
    size_t      size;
    size_t      align;
    
    // for arrays
    MIRType *elem_type;
    size_t   elem_count;
    
    // for structs
    struct {
        MIRType **fields;
        size_t    count;
        size_t   *offsets;
    } structure;
    
    // for functions
    struct {
        MIRType  *ret_type;
        MIRType **params;
        size_t    param_count;
        bool      is_variadic;
    } function;
};

MIRType *mir_type_create(MIRTypeKind kind);
MIRType *mir_type_create_array(MIRType *elem_type, size_t count);
MIRType *mir_type_create_struct(MIRType **fields, size_t count);
MIRType *mir_type_create_function(MIRType *ret_type, MIRType **params, size_t param_count, bool is_variadic);
void     mir_type_destroy(MIRType *type);

#endif // MIR_TYPE_H
