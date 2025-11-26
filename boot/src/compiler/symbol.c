#include "compiler/symbol.h"
#include <stdlib.h>
#include <string.h>

Symbol *symbol_create(const char *name, SymbolKind kind)
{
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol)
    {
        return NULL;
    }

    symbol->kind = kind;
    symbol->name = name ? strdup(name) : NULL;
    symbol->export_name = NULL;
    symbol->type = NULL;
    symbol->decl = NULL;
    symbol->is_public = false;
    symbol->is_mutable = false;
    symbol->next = NULL;

    return symbol;
}

void symbol_destroy(Symbol *symbol)
{
    if (!symbol)
    {
        return;
    }

    if (symbol->name)
    {
        free(symbol->name);
    }
    if (symbol->export_name)
    {
        free(symbol->export_name);
    }

    free(symbol);
}

SymbolTable *symbol_table_create(SymbolTable *parent)
{
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
    {
        return NULL;
    }

    table->symbols = NULL;
    table->parent = parent;
    table->scope_name = NULL;

    return table;
}

void symbol_table_destroy(SymbolTable *table)
{
    if (!table)
    {
        return;
    }

    // free all symbols
    Symbol *sym = table->symbols;
    while (sym)
    {
        Symbol *next = sym->next;
        symbol_destroy(sym);
        sym = next;
    }

    if (table->scope_name)
    {
        free(table->scope_name);
    }

    free(table);
}

int symbol_table_insert(SymbolTable *table, Symbol *symbol)
{
    if (!table || !symbol)
    {
        return -1;
    }

    // check for duplicate in current scope only
    Symbol *existing = symbol_table_lookup_local(table, symbol->name);
    if (existing)
    {
        return -1; // duplicate
    }

    // add to front of list
    symbol->next = table->symbols;
    table->symbols = symbol;

    return 0;
}

Symbol *symbol_table_lookup_local(SymbolTable *table, const char *name)
{
    if (!table || !name)
    {
        return NULL;
    }

    for (Symbol *sym = table->symbols; sym; sym = sym->next)
    {
        if (sym->name && strcmp(sym->name, name) == 0)
        {
            return sym;
        }
    }

    return NULL;
}

Symbol *symbol_table_lookup(SymbolTable *table, const char *name)
{
    if (!table || !name)
    {
        return NULL;
    }

    // search current scope
    Symbol *sym = symbol_table_lookup_local(table, name);
    if (sym)
    {
        return sym;
    }

    // search parent scopes
    if (table->parent)
    {
        return symbol_table_lookup(table->parent, name);
    }

    return NULL;
}
