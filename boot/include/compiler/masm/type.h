#ifndef MASM_TYPE_H
#define MASM_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum MasmTypeKind
{
    MASM_TYPE_VOID,
    MASM_TYPE_I8,
    MASM_TYPE_U8,
    MASM_TYPE_I16,
    MASM_TYPE_U16,
    MASM_TYPE_I32,
    MASM_TYPE_U32,
    MASM_TYPE_I64,
    MASM_TYPE_U64,
    MASM_TYPE_F32,
    MASM_TYPE_F64,
    MASM_TYPE_PTR,
    MASM_TYPE_ARRAY,
    MASM_TYPE_RECORD,
    MASM_TYPE_FUNCTION
} MasmTypeKind;

typedef struct MasmType MasmType;

struct MasmType
{
    MasmTypeKind kind;
    size_t      size;
    size_t      align;
    
    // for arrays
    MasmType *elem_type;
    size_t   elem_count;
    
    // for records
    struct {
        MasmType **fields;
        size_t    count;
        size_t   *offsets;
    } record;
    
    // for functions
    struct {
        MasmType  *ret_type;
        MasmType **params;
        size_t    param_count;
        bool      is_variadic;
    } function;
};

MasmType *masm_type_create(MasmTypeKind kind);
MasmType *masm_type_create_array(MasmType *elem_type, size_t count);
MasmType *masm_type_create_record(MasmType **fields, size_t count);
MasmType *masm_type_create_function(MasmType *ret_type, MasmType **params, size_t param_count, bool is_variadic);
void      masm_type_destroy(MasmType *type);

#endif // MASM_TYPE_H
