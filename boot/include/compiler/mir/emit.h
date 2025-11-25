#ifndef MIR_EMIT_H
#define MIR_EMIT_H

#include "compiler/mir/module.h"
#include "compiler/mir/target.h"

// mir code emission and compilation pipeline

// compile mir module to executable binary
int mir_emit_executable(MIRModule *module, const MIRTarget *target, const char *output_path);

// compile mir module to object file
int mir_emit_object(MIRModule *module, const MIRTarget *target, const char *output_path);

#endif // MIR_EMIT_H
