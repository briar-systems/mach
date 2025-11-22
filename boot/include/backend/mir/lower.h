#ifndef BACKEND_MIR_LOWER_H
#define BACKEND_MIR_LOWER_H

#include "backend/mir/mir.h"
#include "backend/target.h"
#include <stdbool.h>

bool backend_mir_lower(const Target *target, MirModule *module, BackendCodegenResult *result);

#endif // BACKEND_MIR_LOWER_H
