# Symbols

This document describes symbol management in MASM.

## Overview

Symbols are named locations within a MASM module. They associate names with positions in sections, enabling linking, function calls, and global variable access.


## Symbol Kinds

| Kind | Description | Example |
|------|-------------|---------|
| `MASM_SYMBOL_LABEL` | Code label within a function | `.loop_start` |
| `MASM_SYMBOL_FUNCTION` | Function entry point | `main`, `add` |
| `MASM_SYMBOL_DATA` | Data location | `global_counter`, `__str_0` |


## Symbol Binding

Binding determines symbol visibility during linking:

| Binding | Description | Use Case |
|---------|-------------|----------|
| `MASM_BIND_LOCAL` | Visible only within this module | Internal helpers, string literals |
| `MASM_BIND_GLOBAL` | Visible to linker, exported | Public functions, `pub` declarations |
| `MASM_BIND_WEAK` | Can be overridden by another definition | Default implementations |


## Symbol Structure

```c
typedef struct MasmSymbol {
    char          *name;         // symbol name
    MasmSymbolKind kind;         // label, function, or data
    MasmSymbolBind bind;         // local, global, or weak
    
    // location within the module
    char    *section_name;       // section containing this symbol
    uint64_t offset;             // byte offset within section
    size_t   size;               // size in bytes (0 for labels)
} MasmSymbol;
```


## Creating Symbols

```c
MasmSymbol *sym = masm_symbol_create(
    "my_function",          // name
    MASM_SYMBOL_FUNCTION,   // kind
    MASM_BIND_GLOBAL        // binding
);

// set location
sym->section_name = strdup(".text");
sym->offset = 0;
sym->size = 42;  // function size in bytes
```


## Managing Symbols

### Adding to Module

```c
masm_add_symbol(masm, symbol);
```

### Looking Up

```c
MasmSymbol *sym = masm_get_symbol(masm, "my_function");
if (sym != NULL) {
    // symbol exists
}
```


## Symbol Types

### Function Symbols

Function symbols mark the entry points of callable functions:

```c
MasmSymbol *fn = masm_symbol_create("add", MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
fn->section_name = strdup(".text");
fn->offset = text_section->data_size;  // current position in .text
fn->size = 0;  // filled in after function is emitted
```

After emitting the function body, update the size:

```c
fn->size = text_section->data_size - fn->offset;
```

### Data Symbols

Data symbols reference global variables and constants:

```c
// global variable
MasmSymbol *var = masm_symbol_create("counter", MASM_SYMBOL_DATA, MASM_BIND_GLOBAL);
var->section_name = strdup(".data");
var->offset = data_section->data_size;
var->size = 8;  // i64

// string literal (internal)
MasmSymbol *str = masm_symbol_create("__str_0", MASM_SYMBOL_DATA, MASM_BIND_LOCAL);
str->section_name = strdup(".rodata");
str->offset = rodata_section->data_size;
str->size = 14;  // "Hello, World!\0"
```

### Label Symbols

Labels mark positions within functions for control flow:

```c
MasmSymbol *label = masm_symbol_create(".loop_body", MASM_SYMBOL_LABEL, MASM_BIND_LOCAL);
label->section_name = strdup(".text");
label->offset = current_offset;
label->size = 0;  // labels have no size
```


## Symbol Naming Conventions

### User-Defined Symbols

- Function names: Match Mach identifier (may be mangled)
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

When code references a symbol, a relocation entry is created:

```c
// in code generation
emit_call_relocation(ctx, "target_function", R_X86_64_PLT32);
emit_data_relocation(ctx, "global_var", R_X86_64_PC32);
```

The linker fills in the actual addresses based on final layout.


## External Symbols

External symbols reference functions or data provided by other modules or libraries:

```mach
ext "C:printf" printf: fun(fmt: *u8, ...) i32;
```

These create undefined symbols that must be resolved at link time.


## ELF Symbol Table

When emitting ELF, symbols are written to the `.symtab` section:

```
┌─────────────────────────────────┐
│  Index 0: NULL symbol           │
├─────────────────────────────────┤
│  Local symbols (STB_LOCAL)      │
│    - __str_0                    │
│    - __float_0                  │
│    - .loop_start                │
├─────────────────────────────────┤
│  Global symbols (STB_GLOBAL)    │
│    - main                       │
│    - add                        │
│    - global_counter             │
├─────────────────────────────────┤
│  Undefined symbols              │
│    - printf                     │
│    - malloc                     │
└─────────────────────────────────┘
```

Local symbols must come before global symbols in ELF.


## Common Patterns

### Defining a Public Function

```c
// create symbol
MasmSymbol *fn = masm_symbol_create("add", MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
fn->section_name = strdup(".text");
fn->offset = text->data_size;

// add to module
masm_add_symbol(masm, fn);

// emit function body
// ...

// update size
fn->size = text->data_size - fn->offset;
```

### Defining an Internal Helper

```c
MasmSymbol *helper = masm_symbol_create("__helper", MASM_SYMBOL_FUNCTION, MASM_BIND_LOCAL);
helper->section_name = strdup(".text");
helper->offset = text->data_size;
masm_add_symbol(masm, helper);
```

### Referencing a Symbol

In IR, symbols are referenced via operands:

```c
MasmOperand target = masm_operand_symbol("printf");
MasmInstruction call = masm_inst_2(MASM_IR_CALL, target, args);
```


## Cleanup

```c
void masm_symbol_destroy(MasmSymbol *symbol);
```

Symbols are typically owned by the MASM module and destroyed when the module is destroyed.


## See Also

- [Sections](sections.md) - Where symbols live
- [IR Opcodes](ir.md) - Instructions that reference symbols
- [Code Generation](codegen.md) - Relocation handling