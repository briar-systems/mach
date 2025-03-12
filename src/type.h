#ifndef TYPE_H
#define TYPE_H

#include "target.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Type Type;

typedef enum
{
    TYPE_ERROR, // internal use, not an error type

    TYPE_VOID,

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

    TYPE_PTR,
    TYPE_ARR,
    TYPE_STR,
    TYPE_UNI,
    TYPE_FUN,
} TypeKind;

struct Type
{
    TypeKind kind;

    union
    {
        struct
        {
            char *message;
        } err;

        struct
        {
            Type *target;
        } ptr;

        struct
        {
            Type *element_type;
            int len;
        } arr;

        struct
        {
            char *name;
            Type **field_types;
            char **field_names;
            size_t *field_offsets;
            int field_count;
        } str;

        struct
        {
            char *name;
            Type **field_types;
            char **field_names;
            int field_count;
        } uni;

        struct
        {
            struct Type *return_type;
            struct Type **param_types;
            char **param_names;
            int param_count;
        } fun;
    };
};

typedef struct TypeVisited TypeVisited;

bool type_init(Type *type, TypeKind kind);
void type_free(Type *type);

Type *type_make_error(const char *message);
Type *type_make_ptr(Type *target);
Type *type_make_array(Type *element_type, int len);
Type *type_make_struct(const char *name, Type **field_types, char **field_names, int field_count);
Type *type_make_function(Type *return_type, Type **param_types, char **param_names, int param_count);

size_t type_size(Type *type, Target target);
size_t type_align(Type *type, Target target);

void type_compute_offsets(Type *type, Target target);

bool type_equals(Type *a, Type *b);

const char *type_kind_to_string(TypeKind kind);

bool type_visited_init(TypeVisited *tv);
void type_visited_free(TypeVisited *tv);
bool type_visited_contains(TypeVisited *tv, Type *type);
void type_visited_add(TypeVisited *tv, Type *type);

char *type_to_string(Type *type);
int type_to_string_internal(Type *type, char *buffer, size_t buffer_size, TypeVisited *visited);

#endif // TYPE_H
