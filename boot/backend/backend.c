#include "backend.h"
#include "codegen.h"
#include <stdio.h>

bool backend_emit_with_target(const BackendTarget *target, MirModule *module, const char *output_path)
{
    if (!target || !module || !output_path || !target->isa || !target->writer)
        return false;

    BackendCodegenResult result;
    backend_codegen_result_init(&result);

    bool ok = target->isa->lower(target, module, &result);
    if (ok)
    {
        ok = target->writer->write_executable(target, &result, output_path);
        if (!ok)
            fprintf(stderr, "backend: object writer failed for target\n");
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
    const BackendTarget *target = backend_target_lookup(desc);
    if (!target)
        return false;
    return backend_emit_with_target(target, module, output_path);
}
