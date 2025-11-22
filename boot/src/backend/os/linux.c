#include "backend/os/linux.h"

const TargetOS *os_linux()
{
    static TargetOS os;
    os.kind = TARGET_OS_KIND_LINUX;
    os.name = "linux";

    return &os;
}
