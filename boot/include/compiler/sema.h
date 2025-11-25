#ifndef SEMA_H
#define SEMA_H

#include "compiler/ast.h"
#include "compiler/symbol.h"
#include <stdbool.h>

// semantic analyzer context
typedef struct Sema Sema;

// create/destroy semantic analyzer
Sema *sema_create();
void  sema_destroy(Sema *sema);

// analyze ast and populate types/symbols
// returns 0 on success, -1 on error
int sema_analyze(Sema *sema, AstNode *ast);

// get root symbol table (for inspection/codegen)
SymbolTable *sema_get_root_table(Sema *sema);

// error reporting
int  sema_get_error_count(Sema *sema);
void sema_print_errors(Sema *sema);

#endif // SEMA_H
