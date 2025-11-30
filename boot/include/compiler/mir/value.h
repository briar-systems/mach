#ifndef MIR_VALUE_H
#define MIR_VALUE_H

#include "compiler/mir/type.h"
#include <stdint.h>

// forward declarations
typedef struct MIRInst MIRInst;

// mir ssa value
typedef struct MIRValue
{
    uint32_t id;       // unique id within function
    MIRType *type;     // mir type
    MIRInst *def_inst; // defining instruction (ssa property)
    char    *name;     // optional name (for readability)
} MIRValue;

// value management
MIRValue *mir_value_create(uint32_t id, MIRType *type, const char *name);
void      mir_value_destroy(MIRValue *value);

#endif // MIR_VALUE_H
