#ifndef BACKEND_BACKEND_H
#define BACKEND_BACKEND_H

#include "target.h"
#include "mir/mir.h"

bool backend_emit_executable(TargetDescriptor desc, MirModule *module, const char *output_path);
bool backend_emit_with_target(const Target *target, MirModule *module, const char *output_path);

#endif // BACKEND_BACKEND_H
