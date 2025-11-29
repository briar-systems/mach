#ifndef COMPTIME_H
#define COMPTIME_H

#include "compiler/ast.h"
#include "compiler/sema.h"

// attempt to resolve a compile-time constant expression starting with $mach
// returns 0 on success (and sets node->comptime.value_kind), -1 on failure.
int comptime_lookup(Sema *sema, AstNode *node);

#endif
