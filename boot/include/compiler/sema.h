#ifndef SEMA_H
#define SEMA_H

#include "compiler/ast.h"
#include "compiler/symbol.h"
#include <stdbool.h>

// forward declarations
struct ConfigDep;

// semantic analyzer context
typedef struct Sema Sema;

// create/destroy semantic analyzer
Sema *sema_create(const char *module_path);
void  sema_destroy(Sema *sema);

// configure module resolution paths
// project_id: the project's id from mach.toml (used as module prefix)
// src_root: absolute path to the source directory
// dep_root: absolute path to the dependencies directory (may be NULL)
// deps: array of dependency pointers from Config (may be NULL)
// dep_count: number of dependencies
void sema_set_module_roots(Sema *sema, const char *project_id, const char *src_root, const char *dep_root, struct ConfigDep **deps, int dep_count);

// add an additional module root mapping.
// this is primarily for single-file compilation mode, where the user may
// provide explicit `-I prefix=/abs/path/to/src` mappings.
void sema_add_module_root(Sema *sema, const char *module_prefix, const char *src_root);

// set file context for error reporting (call before analyzing a file)
void sema_set_file_context(Sema *sema, const char *file_path, const char *source);

// analyze ast and populate types/symbols
// returns 0 on success, -1 on error
int sema_analyze(Sema *sema, AstNode *ast);

// get root symbol table (for inspection/codegen)
SymbolTable *sema_get_root_table(Sema *sema);

// get all loaded module ASTs (for codegen)
// returns count, fills asts array (must be pre-allocated)
int sema_get_loaded_modules(Sema *sema, AstNode **asts, int max_count);

// error reporting
int  sema_get_error_count(Sema *sema);
void sema_print_errors(Sema *sema);

// generic instantiation
Symbol *sema_instantiate_generic(Sema *sema, Symbol *generic_sym, AstList *type_args);

#endif // SEMA_H
