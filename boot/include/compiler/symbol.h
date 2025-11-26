#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>
#include <stdint.h>

// forward declarations
typedef struct Type Type;
typedef struct AstNode AstNode;

typedef enum SymbolKind
{
    SYMBOL_VARIABLE, // var or val
    SYMBOL_FUNCTION, // fun
    SYMBOL_TYPE,     // rec, uni, def
    SYMBOL_PARAMETER, // function parameter
} SymbolKind;

// symbol represents a named entity in the program
typedef struct Symbol
{
    SymbolKind     kind;
    char          *name;
    char          *export_name; // name used in object file (can be overridden)
    char          *mangled_name; // unique mangled name
    Type          *type;
    AstNode       *decl;        // declaration node
    bool           is_public;   // pub qualifier
    bool           is_mutable;  // false for val, true for var
    struct Symbol *next;        // for linked list in scope
} Symbol;

// symbol table with nested scopes
typedef struct SymbolTable
{
    Symbol             *symbols;       // linked list of symbols in this scope
    struct SymbolTable *parent;        // parent scope (NULL for global)
    char               *scope_name;    // for debugging
} SymbolTable;

// symbol operations
Symbol *symbol_create(const char *name, SymbolKind kind);
void    symbol_destroy(Symbol *symbol);
void    symbol_mangle(Symbol *symbol);
const char *symbol_get_linkage_name(Symbol *symbol);

// symbol table operations
SymbolTable *symbol_table_create(SymbolTable *parent);
void         symbol_table_destroy(SymbolTable *table);
int          symbol_table_insert(SymbolTable *table, Symbol *symbol);
Symbol      *symbol_table_lookup(SymbolTable *table, const char *name);
Symbol      *symbol_table_lookup_local(SymbolTable *table, const char *name);

#endif
