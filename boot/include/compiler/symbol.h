#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>
#include <stdint.h>

// forward declarations
typedef struct Type    Type;
typedef struct AstNode AstNode;

typedef enum SymbolKind
{
    SYMBOL_VARIABLE,  // var or val
    SYMBOL_FUNCTION,  // fun
    SYMBOL_TYPE,      // rec, uni, def
    SYMBOL_PARAMETER, // function parameter
} SymbolKind;

// symbol represents a named entity in the program
typedef struct Symbol
{
    SymbolKind     kind;
    char          *name;
    char          *export_name;  // override name for export (e.g. from $symbol.name)
    char          *mangled_name; // cached mangled name
    char          *module_path;  // dot-separated module path (e.g. "std.print")
    Type          *type;
    AstNode       *decl;               // declaration node
    bool           is_public;          // pub qualifier
    bool           is_mutable;         // false for val, true for var
    bool           is_generic;         // true if this is a generic template
    bool           is_generic_param;   // true if this is a type parameter binding
    bool           is_being_analyzed;  // true during analysis to prevent recursion
    char          *generic_param_name; // formal parameter name for generic type params
    struct Symbol *next;               // for linked list in scope
} Symbol;

// symbol table with nested scopes
typedef struct SymbolTable
{
    Symbol             *symbols;    // linked list of symbols in this scope
    struct SymbolTable *parent;     // parent scope (NULL for global)
    char               *scope_name; // for debugging
} SymbolTable;

// symbol operations
Symbol     *symbol_create(const char *name, SymbolKind kind, const char *module_path);
void        symbol_destroy(Symbol *symbol);
void        symbol_mangle(Symbol *symbol);
const char *symbol_linkage_name(Symbol *symbol);

// symbol table operations
SymbolTable *symbol_table_create(SymbolTable *parent);
void         symbol_table_destroy(SymbolTable *table);
int          symbol_table_insert(SymbolTable *table, Symbol *symbol);
Symbol      *symbol_table_lookup(SymbolTable *table, const char *name);
Symbol      *symbol_table_lookup_local(SymbolTable *table, const char *name);

// method lookup - finds methods stored with qualified names "TypeName.methodName"
Symbol      *symbol_table_lookup_method_local(SymbolTable *table, const char *method_name);
Symbol      *symbol_table_lookup_method_next(SymbolTable *table, const char *method_name, Symbol *after);

#endif
