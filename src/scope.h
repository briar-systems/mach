#ifndef SCOPE_H
#define SCOPE_H

#include "type.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;
typedef struct Scope Scope;

struct Symbol
{
    char *name;
    void *value;
    Type *type;
};

struct SymbolTable
{
    Symbol *symbols;
    int len;
    int cap;
};

struct Scope
{
    Scope *parent;
    SymbolTable symbols;
};

bool symbol_table_init(SymbolTable *table);
void symbol_table_free(SymbolTable *table);

Symbol *symbol_table_add(SymbolTable *table, const char *name);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);

bool scope_init(Scope *scope, Scope *parent);
void scope_free(Scope *scope);

Symbol *scope_lookup(Scope *scope, const char *name);
Symbol *scope_define(Scope *scope, const char *name, void *value, Type *type);

#endif // SCOPE_H
