#ifndef BACKEND_ISA_X86_64_H
#define BACKEND_ISA_X86_64_H

#include "../target.h"

const TargetArch *backend_arch_x86_64(void);
const TargetISA  *backend_isa_x86_64(void);
const TargetABI  *backend_abi_sysv64(void);

#endif // BACKEND_ISA_X86_64_H
