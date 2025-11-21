#include "backend/target.h"
#include <stddef.h>

#define MAX_BACKEND_TARGETS 16

static const Target *g_targets[MAX_BACKEND_TARGETS];
static size_t g_target_count = 0;

bool backend_target_register(const Target *target)
{
    if (!target || !target->isa || !target->abi || !target->writer || !target->runtime)
        return false;

    // check for duplicates
    for (size_t i = 0; i < g_target_count; i++)
    {
        if (g_targets[i]->desc.arch == target->desc.arch && g_targets[i]->desc.os == target->desc.os)
        {
            g_targets[i] = target;
            return true;
        }
    }

    if (g_target_count >= MAX_BACKEND_TARGETS)
        return false;

    g_targets[g_target_count++] = target;
    return true;
}

const Target *backend_target_lookup(TargetDescriptor desc)
{
    for (size_t i = 0; i < g_target_count; i++)
    {
        if (g_targets[i]->desc.arch == desc.arch && g_targets[i]->desc.os == desc.os)
            return g_targets[i];
    }
    return NULL;
}
