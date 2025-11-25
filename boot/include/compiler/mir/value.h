#ifndef MIR_VALUE_H
#define MIR_VALUE_H

#include "compiler/type.h"
#include <stdint.h>

// forward declarations
typedef struct MIRInst MIRInst;

// mir ssa value
typedef struct MIRValue
{
    uint32_t id;       // unique id within function
    Type    *type;     // mach type
    MIRInst *def_inst; // defining instruction (ssa property)
    char    *name;     // optional name (for readability)
} MIRValue;

// value management
MIRValue *mir_value_create(uint32_t id, Type *type, const char *name);
void      mir_value_destroy(MIRValue *value);

#endif // MIR_VALUE_H
