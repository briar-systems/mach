#include "compiler/masm/os/spec.h"
#include <stddef.h>

static const MasmOSSpec OS_LINUX = {
    .name = "linux",
};

const MasmOSSpec *masm_os_spec_select(MasmTargetOS os)
{
    switch (os)
    {
    case MASM_OS_LINUX:
        return &OS_LINUX;
    default:
        return NULL;
    }
}
