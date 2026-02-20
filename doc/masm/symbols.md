# Symbols

This document describes symbol management in MASM.

## Overview

Symbols are named locations within a MASM module. They associate names with positions in sections, enabling linking, function calls, and global variable access.

Symbol management is defined in `src/compiler/masm/symbol.mach`.


## Symbol Structure

```mach
pub rec Symbol {
    name:     str;
    binding:  SymBinding;
    sym_type: SymKind;
    section:  u64;       # section index (not a pointer)
    offset:   usize;     # byte offset within section
    size:     usize;
    defined:  bool;      # true = defined here, false = external reference
    src_file: str;       # source file path (for debug info)
    src_line: i32;       # source line number (for debug info)
}
```


## Symbol Binding

Binding determines symbol visibility during linking:

| Constant | Value | Description | Use Case |
|----------|-------|-------------|----------|
| `BIND_LOCAL` | 0 | Visible only within this module | Internal helpers, string literals |
| `BIND_GLOBAL` | 1 | Visible to linker, exported | Public functions, `pub` declarations |
| `BIND_WEAK` | 2 | Can be overridden by another definition | Default implementations |


## Symbol Kinds

| Constant | Value | Description |
|----------|-------|-------------|
| `SYM_NONE` | 0 | Unspecified |
| `SYM_FUNC` | 1 | Function entry point |
| `SYM_OBJECT` | 2 | Data object (global variable, string literal) |
| `SYM_SECTION` | 3 | Section symbol |
| `SYM_FILE` | 4 | Source file name |


## Symbol Operations

### Creating a Symbol

```mach
use sym: mach.compiler.masm.symbol;

val r: Result[*sym.Symbol, str] = sym.create(alloc, "my_function");
val s: *sym.Symbol = result_unwrap_ok[*sym.Symbol, str](r);
s.binding = sym.BIND_GLOBAL;
s.sym_type = sym.SYM_FUNC;
```

### Defining a Symbol

```mach
# mark as defined at section[0] offset 0, size 0
sym.define(s, 0, 0, 0);
```

### Adding to a Context

```mach
use masm: mach.compiler.masm;

masm.add_symbol(m, s);
```

### Looking Up a Symbol

```mach
val found: *sym.Symbol = masm.get_symbol(m, "my_function");
```

The context maintains an O(1) hash map for symbol lookup by name.

### Query Helpers

```mach
sym.is_defined(s)    bool   # s.defined
sym.is_global(s)     bool   # s.binding == BIND_GLOBAL
sym.is_function(s)   bool   # s.sym_type == SYM_FUNC
```


## Symbol Table

For standalone symbol table use (outside a MASM context):

```mach
pub rec SymbolTable {
    symbols: *Symbol;
    count:   i32;
    cap:     i32;
}

sym.table_create(alloc)                  Result[*SymbolTable, str]
sym.table_add(alloc, tbl, sym_value)     i32          # returns index or -1
sym.table_get(tbl, index)               *Symbol
sym.table_destroy(alloc, tbl)
```


## Symbol Naming Conventions

### User-Defined Symbols

- Function names: Match Mach identifier (may be mangled for generics)
- Global variables: Match Mach identifier

### Compiler-Generated Symbols

The lowering phase generates synthetic symbols:

| Pattern | Description |
|---------|-------------|
| `__str_N` | String literal N |
| `__float_N` | Float constant N |
| `__anon_N` | Anonymous record/union literal |

### Local Labels

Labels within functions use a dot prefix:

| Pattern | Description |
|---------|-------------|
| `.if_then_N` | If-then branch |
| `.if_else_N` | If-else branch |
| `.if_end_N` | End of if statement |
| `.for_cond_N` | Loop condition |
| `.for_body_N` | Loop body |
| `.for_end_N` | End of loop |


## Symbol Resolution

During linking, symbols are resolved in this order:

1. **Local symbols** — Resolved within the same module
2. **Global symbols** — Matched across modules by name
3. **External symbols** — Provided by libraries or system

### Relocations

When code references a symbol, a relocation entry is created in the containing section. The linker fills in the actual addresses based on final layout. See [Sections](sections.md) for relocation types.


## External Symbols

External symbols reference functions or data provided by other modules or libraries:

```mach
ext fun memcpy(dst: ptr, src: ptr, n: usize) ptr;
```

These create undefined symbols (`defined = false`) that must be resolved at link time.


## ELF Symbol Table

When emitting ELF, symbols are written to the `.symtab` section:

```
+----------------------------------+
|  Index 0: NULL symbol            |
+----------------------------------+
|  Local symbols (STB_LOCAL)       |
|    - __str_0                     |
|    - __float_0                   |
+----------------------------------+
|  Global symbols (STB_GLOBAL)     |
|    - main                        |
|    - add                         |
|    - global_counter              |
+----------------------------------+
|  Undefined symbols               |
|    - printf                      |
|    - malloc                      |
+----------------------------------+
```

Local symbols must come before global symbols in ELF.


## See Also

- [Sections](sections.md) - Where symbols live
- [IR Opcodes](ir.md) - Instructions that reference symbols
- [Code Generation](codegen.md) - Relocation handling
