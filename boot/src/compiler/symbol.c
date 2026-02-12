#include "compiler/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Symbol *symbol_create(const char *name, SymbolKind kind, const char *module_path)
{
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol)
    {
        return NULL;
    }

    symbol->kind               = kind;
    symbol->name               = name ? strdup(name) : NULL;
    symbol->export_name        = NULL;
    symbol->mangled_name       = NULL;
    symbol->module_path        = module_path ? strdup(module_path) : NULL;
    symbol->type               = NULL;
    symbol->decl               = NULL;
    symbol->is_public          = false;
    symbol->is_mutable         = false;
    symbol->is_generic         = false;
    symbol->is_generic_param   = false;
    symbol->is_being_analyzed  = false;
    symbol->generic_param_name = NULL;
    symbol->next               = NULL;

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
    if (symbol->mangled_name)
    {
        free(symbol->mangled_name);
    }
    if (symbol->module_path)
    {
        free(symbol->module_path);
    }
    if (symbol->generic_param_name)
    {
        free(symbol->generic_param_name);
    }

    free(symbol);
}

void symbol_mangle(Symbol *symbol)
{
    if (!symbol || !symbol->name)
    {
        return;
    }

    if (symbol->mangled_name)
    {
        return; // already mangled
    }

    // mangling scheme: _M<encoded_module_path>N<name_len><name>
    // encoded module path: length-prefixed segments of dot-separated path
    // e.g. "std.print" -> "3std5print"

    // default module "main" if not specified
    const char *path = symbol->module_path ? symbol->module_path : "main";

    // first pass: calculate length
    size_t encoded_len = 0;
    char  *path_copy   = strdup(path);
    char  *saveptr;
    char  *token = strtok_r(path_copy, ".", &saveptr);
    while (token)
    {
        char len_str[32];
        sprintf(len_str, "%zu", strlen(token));
        encoded_len += strlen(len_str) + strlen(token);
        token = strtok_r(NULL, ".", &saveptr);
    }
    free(path_copy);

    size_t name_len = strlen(symbol->name);
    char   name_len_str[32];
    sprintf(name_len_str, "%zu", name_len);

    // _M + encoded_len + N + name_len_str + name + null
    size_t total_len = 2 + encoded_len + 1 + strlen(name_len_str) + name_len + 1;

    symbol->mangled_name = malloc(total_len);
    if (!symbol->mangled_name)
    {
        return;
    }

    // second pass: build string
    char *ptr = symbol->mangled_name;
    ptr += sprintf(ptr, "_M");

    path_copy = strdup(path);
    token     = strtok_r(path_copy, ".", &saveptr);
    while (token)
    {
        ptr += sprintf(ptr, "%zu%s", strlen(token), token);
        token = strtok_r(NULL, ".", &saveptr);
    }
    free(path_copy);

    sprintf(ptr, "N%zu%s", name_len, symbol->name);
}

const char *symbol_linkage_name(Symbol *symbol)
{
    if (!symbol)
    {
        return NULL;
    }

    if (symbol->export_name)
    {
        return symbol->export_name;
    }

    if (!symbol->mangled_name)
    {
        symbol_mangle(symbol);
    }

    return symbol->mangled_name ? symbol->mangled_name : symbol->name;
}

SymbolTable *symbol_table_create(SymbolTable *parent)
{
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
    {
        return NULL;
    }

    table->symbols    = NULL;
    table->parent     = parent;
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
    symbol->next   = table->symbols;
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


