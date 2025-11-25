#ifndef MIR_GLOBAL_H
#define MIR_GLOBAL_H

#include "compiler/type.h"
#include <stdbool.h>
#include <stdint.h>

// mir global data kinds
typedef enum MIRGlobalKind
{
    MIR_GLOBAL_VAL,    // val (read-only)
    MIR_GLOBAL_VAR,    // var (mutable, initialized)
    MIR_GLOBAL_UNINIT, // var (uninitialized, bss)
} MIRGlobalKind;

// mir global data
typedef struct MIRGlobal
{
    char          *name;
    Type          *type;
    MIRGlobalKind  kind;
    bool           is_exported;
    
    // initializer data
    union
    {
        int64_t     int_value;
        double      float_value;
        const char *string_value;
        void       *array_data;
    } init;
    
    struct MIRGlobal *next; // linked list within module
} MIRGlobal;

// global management
MIRGlobal *mir_global_create(const char *name, Type *type, MIRGlobalKind kind, bool is_exported);
void       mir_global_destroy(MIRGlobal *global);
void       mir_global_set_int_init(MIRGlobal *global, int64_t value);
void       mir_global_set_float_init(MIRGlobal *global, double value);
void       mir_global_set_string_init(MIRGlobal *global, const char *value);

#endif // MIR_GLOBAL_H
