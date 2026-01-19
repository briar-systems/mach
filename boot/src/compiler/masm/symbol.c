#include "compiler/masm/symbol.h"
#include <stdlib.h>
#include <string.h>

MasmSymbol *masm_symbol_create(const char *name, MasmSymbolKind kind, MasmSymbolBind bind)
{
    MasmSymbol *symbol = malloc(sizeof(MasmSymbol));
    if (!symbol) return NULL;
    
    memset(symbol, 0, sizeof(MasmSymbol));
    symbol->name = strdup(name);
    symbol->kind = kind;
    symbol->bind = bind;
    
    return symbol;
}

void masm_symbol_destroy(MasmSymbol *symbol)
{
    if (!symbol) return;
    
    if (symbol->name) free(symbol->name);
    if (symbol->section_name) free(symbol->section_name);
    
    free(symbol);
}
