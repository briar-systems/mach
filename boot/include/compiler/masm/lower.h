#ifndef MASM_LOWER_H
#define MASM_LOWER_H

#include "compiler/ast.h"
#include "compiler/masm/masm.h"
#include "compiler/symbol.h"

// lower ast to masm
Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols);

#endif // MASM_LOWER_H
