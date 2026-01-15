# Globals

This document describes global variables and constants in MIR.

## Overview

Globals represent data that exists for the entire program lifetime. They are allocated in specific sections of the object file and referenced by name.

## Global Kinds

MIR supports three kinds of global data:

| Kind | Description | Section | Mutable |
|------|-------------|---------|---------|
| `MIR_GLOBAL_VAL` | Read-only value | `.rodata` | No |
| `MIR_GLOBAL_VAR` | Initialized variable | `.data` | Yes |
| `MIR_GLOBAL_UNINIT` | Uninitialized variable | `.bss` | Yes |

## Global Structure

```c
typedef struct MIRGlobal {
    char *name;              // global symbol name
    Type *type;              // type of the global
    MIRGlobalKind kind;      // val, var, or uninit
    bool is_exported;        // visible to linker?
    
    union {
        int64_t int_value;       // for integer constants
        double float_value;      // for float constants  
        const char *string_value; // for string literals
        void *array_data;        // for array data
    } init;
} MIRGlobal;
```

## Creating Globals

### Integer Constants

```c
MIRGlobal *global = mir_global_create("my_const", 
                                      type_get_primitive(TYPE_I64),
                                      MIR_GLOBAL_VAL, 
                                      false);
mir_global_set_int_init(global, 42);
```

In MIR pseudo-code:
```mir
@my_const: i64 = 42  # read-only constant
```

### Float Constants

Float literals are stored as synthetic globals:
```c
MIRGlobal *global = mir_global_create("__float_const_0",
                                      type_get_primitive(TYPE_F64),
                                      MIR_GLOBAL_VAL,
                                      false);
mir_global_set_float_init(global, 3.14159);
```

In Mach:
```mach
var x: f64 = 3.14;  # Creates @__float_const_0
```

### String Literals

```c
MIRGlobal *global = mir_global_create("__str_0",
                                      type_get_primitive(TYPE_PTR),
                                      MIR_GLOBAL_VAL,
                                      false);
mir_global_set_string_init(global, "Hello, world!");
```

### Mutable Variables

```c
MIRGlobal *global = mir_global_create("global_counter",
                                      type_get_primitive(TYPE_I64),
                                      MIR_GLOBAL_VAR,
                                      true);  // exported
mir_global_set_int_init(global, 0);
```

### Uninitialized Variables

```c
MIRGlobal *global = mir_global_create("buffer",
                                      type_create_array(type_get_primitive(TYPE_U8), 1024),
                                      MIR_GLOBAL_UNINIT,
                                      false);
```

## Using Globals

### Loading Global Address

```mir
%addr = mov @global_var  # Load address of global
```

### Loading Global Value

```mir
%addr = mov @global_var
%value = load %addr      # Load value from global
```

### Storing to Global

```mir
%addr = mov @global_var
store %addr, %value      # Store value to global
```

## Section Placement

Globals are emitted to different ELF sections:

```
┌──────────────┐
│   .rodata    │ ← MIR_GLOBAL_VAL (constants)
├──────────────┤
│    .data     │ ← MIR_GLOBAL_VAR (initialized variables)
├──────────────┤
│    .bss      │ ← MIR_GLOBAL_UNINIT (uninitialized)
└──────────────┘
```

### `.rodata` (Read-Only Data)
- Constant values
- String literals
- Float constants
- Marked read-only in ELF flags

### `.data` (Initialized Data)
- Mutable global variables with initial values
- Writable in memory

### `.bss` (Uninitialized Data)
- Variables without initial values
- Zero-initialized by loader
- Doesn't occupy space in the object file

## Float Literal Handling

Float literals require special handling because they cannot be immediate operands:

1. **Lowering** creates a synthetic global:
```c
char name[64];
snprintf(name, sizeof(name), "__float_const_%d", count++);
MIRGlobal *global = mir_global_create(name, TYPE_F64, MIR_GLOBAL_VAL, false);
mir_global_set_float_init(global, 3.14159);
```

2. **Code generation** loads the value:
```mir
%addr = mov @__float_const_0   # LEA instruction
%val = load %addr              # MOVSD instruction
```

3. **Object emission** writes the float value to `.rodata`:
```c
double value = global->init.float_value;
elf_write_section_data(ctx->elf, rodata_section, (uint8_t *)&value, 8);
```

## Symbol Visibility

Globals can be:
- **Exported** (`is_exported = true`) - Visible to linker, can be referenced by other modules
- **Internal** (`is_exported = false`) - Local to this module

Exported symbols:
```mir
@main: function     # exported (entry point)
@global_var: i64    # exported global
```

Internal symbols:
```mir
@__float_const_0: f64  # internal (synthetic)
@helper_func: function # internal helper
```

## Implementation

Globals are implemented in:
- `boot/include/compiler/mir/global.h`
- `boot/src/compiler/mir/global.c`
- `boot/src/compiler/mir/emit.c` (section emission)

## See Also

- [Opcodes](opcodes.md) - `mov`, `load`, `store` instructions
- [Codegen](codegen.md) - How globals are accessed in assembly
- [Overview](overview.md) - Module structure
