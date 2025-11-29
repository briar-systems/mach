#ifndef MIR_ABI_H
#define MIR_ABI_H

#include "compiler/mir/target.h"
#include "compiler/type.h"
#include <stddef.h>

// generic abi interface for target-independent lowering
// abstracts calling convention details across different ABIs
//
// this interface allows lowering code to remain ABI-agnostic by delegating
// calling convention decisions to ABI-specific implementations.
//
// currently supported ABIs:
//   - MIR_ABI_SYSV64: System V AMD64 ABI (Linux, BSD, macOS on x86_64)
//
// the classification functions return the number of 8-byte register slots
// a type occupies when passed as a parameter or returned from a function:
//   - 0: passed/returned indirectly via memory (pointer)
//   - 1: single register slot (8 bytes or less)
//   - 2: two register slots (9-16 bytes for sysv64)
//   - n: n register slots (ABI-dependent)

// parameter/return value classification
// returns number of 8-byte register slots needed (0 = memory/indirect)
int mir_abi_classify_param(MIRTargetABI abi, Type *type);
int mir_abi_classify_return(MIRTargetABI abi, Type *type);

#endif // MIR_ABI_H
