#ifndef MIR_LOWER_H
#define MIR_LOWER_H

#include "compiler/ast.h"
#include "compiler/mir/module.h"
#include "compiler/symbol.h"

// lowers high-level mach ast to ssa mir

// lowering entry points
MIRModule   *mir_lower_module(AstNode *ast_module, SymbolTable *symbols);
MIRFunction *mir_lower_function(AstNode *ast_function);
MIRGlobal   *mir_lower_global(AstNode *ast_var);

// opaque context
typedef struct LowerContext LowerContext;

// inline mir block parsing
// parses raw mir text (from ast_node->mir_stmt.content) and injects into function
int mir_parse_inline_block(LowerContext *ctx, MIRFunction *func, MIRBlock *current_block, const char *mir_text);

#endif // MIR_LOWER_H
