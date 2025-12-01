#ifndef MASM_SYMBOL_H
#define MASM_SYMBOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum MasmSymbolKind
{
    MASM_SYMBOL_LABEL,
    MASM_SYMBOL_FUNCTION,
    MASM_SYMBOL_DATA
} MasmSymbolKind;

typedef enum MasmSymbolBind
{
    MASM_BIND_LOCAL,
    MASM_BIND_GLOBAL,
    MASM_BIND_WEAK
} MasmSymbolBind;

typedef struct MasmSymbol
{
    char          *name;
    MasmSymbolKind kind;
    MasmSymbolBind bind;
    
    // location
    char    *section_name;
    uint64_t offset;
    size_t   size;
} MasmSymbol;

MasmSymbol *masm_symbol_create(const char *name, MasmSymbolKind kind, MasmSymbolBind bind);
void        masm_symbol_destroy(MasmSymbol *symbol);

#endif // MASM_SYMBOL_H
