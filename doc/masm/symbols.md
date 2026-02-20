# Symbols

This document describes symbol management in MASM.

## Overview

Symbols are named locations within a MASM module. They associate names with positions in sections, enabling linking, function calls, and global variable access. Symbol management is handled by the compiler's symbol table (`src/compiler/symbol.mach`).


## Symbol Kinds

| Kind | Description | Example |
|------|-------------|---------|
| Label | Code label within a function | `.loop_start` |
| Function | Function entry point | `main`, `add` |
| Data | Data location | `global_counter`, `__str_0` |


## Symbol Binding

Binding determines symbol visibility during linking:

| Binding | Description | Use Case |
|---------|-------------|----------|
| Local | Visible only within this module | Internal helpers, string literals |
| Global | Visible to linker, exported | Public functions, `pub` declarations |
| Weak | Can be overridden by another definition | Default implementations |


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
| `.L_N` | Generic label N |
| `.if_then_N` | If-then branch |
| `.if_else_N` | If-else branch |
| `.if_end_N` | End of if statement |
| `.for_cond_N` | Loop condition |
| `.for_body_N` | Loop body |
| `.for_end_N` | End of loop |


## Symbol Resolution

During linking, symbols are resolved in this order:

1. **Local symbols** - Resolved within the same module
2. **Global symbols** - Matched across modules by name
3. **External symbols** - Provided by libraries or system

### Relocations

When code references a symbol, a relocation entry is created. The linker fills in the actual addresses based on final layout.


## External Symbols

External symbols reference functions or data provided by other modules or libraries:

```mach
ext fun memcpy(dst: ptr, src: ptr, n: usize) ptr;
```

These create undefined symbols that must be resolved at link time.


## ELF Symbol Table

When emitting ELF, symbols are written to the `.symtab` section:

```
+----------------------------------+
|  Index 0: NULL symbol            |
+----------------------------------+
|  Local symbols (STB_LOCAL)       |
|    - __str_0                     |
|    - __float_0                   |
|    - .loop_start                 |
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
