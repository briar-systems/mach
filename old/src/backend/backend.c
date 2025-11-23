#include "backend/backend.h"
#include "backend/codegen.h"
#include "backend/mir/lower.h"
#include <stdio.h>

bool backend_emit_with_target(const Target *target, MirModule *module, const char *output_path)
{
    if (!target || !module || !output_path || !target->isa || !target->writer)
    {
        return false;
    }

    CodegenResult result;
    backend_codegen_result_init(&result);

    bool ok = backend_mir_lower(target, module, &result);
    if (ok)
    {
        ok = target->writer->write_executable(target, &result, output_path);
        if (!ok)
        {
            fprintf(stderr, "backend: object writer failed for target\n");
        }
    }
    else
    {
        fprintf(stderr, "backend: lowering failed for target\n");
    }

    backend_codegen_result_destroy(&result);
    return ok;
}

bool backend_emit_executable(TargetDescriptor desc, MirModule *module, const char *output_path)
{
    const Target *target = target_lookup(desc);
    if (!target)
    {
        return false;
    }
    return backend_emit_with_target(target, module, output_path);
}
