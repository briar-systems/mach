#include "scope.h"
#include <stdlib.h>
#include <string.h>

bool symbol_table_init(SymbolTable *table)
{
    if (!table)
    {
        return false;
    }

    table->symbols = NULL;
    table->len = 0;
    table->cap = 0;

    return true;
}

void symbol_table_free(SymbolTable *table)
{
    if (!table)
        return;

    for (int i = 0; i < table->len; i++)
    {
        free(table->symbols[i].name);
        table->symbols[i].name = NULL;
    }

    free(table->symbols);
    table->symbols = NULL;

    free(table);
}

Symbol *symbol_table_add(SymbolTable *table, const char *name)
{
    if (table->len >= table->cap)
    {
        int new_cap = table->cap == 0 ? 8 : table->cap * 2;
        Symbol *new_symbols = realloc(table->symbols, new_cap * sizeof(Symbol));
        if (!new_symbols)
            return NULL;

        table->symbols = new_symbols;
        table->cap = new_cap;
    }

    Symbol *symbol = &table->symbols[table->len++];
    symbol->name = strdup(name);
    symbol->value = NULL;
    symbol->type = NULL;

    return symbol;
}

Symbol *symbol_table_lookup(SymbolTable *table, const char *name)
{
    for (int i = 0; i < table->len; i++)
    {
        if (strcmp(table->symbols[i].name, name) == 0)
        {
            return &table->symbols[i];
        }
    }
    return NULL;
}

bool scope_init(Scope *scope, Scope *parent)
{
    if (!scope)
    {
        return false;
    }

    scope->parent = parent;
    if (!symbol_table_init(&scope->symbols))
    {
        return false;
    }

    return true;
}

void scope_free(Scope *scope)
{
    if (!scope)
    {
        return;
    }

    for (int i = 0; i < scope->symbols.len; i++)
    {
        free(scope->symbols.symbols[i].name);
        scope->symbols.symbols[i].name = NULL;
    }
    free(scope->symbols.symbols);
    scope->symbols.symbols = NULL;

    free(scope);
}

Symbol *scope_lookup(Scope *scope, const char *name)
{
    Scope *current = scope;
    while (current)
    {
        Symbol *symbol = symbol_table_lookup(&current->symbols, name);
        if (symbol)
        {
            return symbol;
        }

        current = current->parent;
    }

    return NULL;
}

Symbol *scope_define(Scope *scope, const char *name, void *value, Type *type)
{
    Symbol *symbol = symbol_table_add(&scope->symbols, name);
    if (symbol)
    {
        symbol->value = value;
        symbol->type = type;
    }

    return symbol;
}
