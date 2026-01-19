# Sections

This document describes section management in MASM.

## Overview

Sections are containers for code and data within a MASM module. They correspond to segments in the final object file and determine memory placement, permissions, and linker behavior.


## Section Kinds

| Kind | Name | Purpose | Permissions |
|------|------|---------|-------------|
| `MASM_SECTION_TEXT` | `.text` | Executable code | Read + Execute |
| `MASM_SECTION_DATA` | `.data` | Initialized mutable data | Read + Write |
| `MASM_SECTION_RODATA` | `.rodata` | Read-only data (constants) | Read |
| `MASM_SECTION_BSS` | `.bss` | Uninitialized data | Read + Write |
| `MASM_SECTION_CUSTOM` | User-defined | Custom sections | Varies |


## Section Structure

```c
typedef struct MasmSection {
    MasmSectionKind kind;
    char           *name;
    
    // for text sections (code)
    MasmInstruction *instructions;
    size_t           inst_count;
    size_t           inst_capacity;
    
    // for data sections
    uint8_t        *data;
    size_t          data_size;
    size_t          data_capacity;
} MasmSection;
```

Sections contain either:
- **Instructions** - For `.text` sections
- **Raw data** - For `.data`, `.rodata`, and `.bss` sections


## Creating Sections

```c
MasmSection *section = masm_section_create(MASM_SECTION_TEXT, ".text");
```

### Getting or Creating Sections

Within a MASM module, use these helpers:

```c
// get existing section (returns NULL if not found)
MasmSection *text = masm_get_section(masm, ".text");

// get or create section
MasmSection *data = masm_get_or_create_section(masm, ".data", MASM_SECTION_DATA);
```


## Text Sections

Text sections contain executable instructions.

### Adding Instructions

```c
MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);

MasmInstruction add = masm_inst_3(MASM_IR_ADD, dst, src1, src2);
masm_section_append_inst(text, add);

MasmInstruction ret = masm_inst_0(MASM_IR_RET);
masm_section_append_inst(text, ret);
```

### Instruction Storage

Instructions are stored in a dynamic array that grows as needed:

```c
void masm_section_append_inst(MasmSection *section, MasmInstruction inst);
```


## Data Sections

Data sections contain raw bytes.

### `.data` - Initialized Mutable Data

Global variables with initial values:

```c
MasmSection *data = masm_get_or_create_section(masm, ".data", MASM_SECTION_DATA);

int64_t counter = 0;
masm_section_append_data(data, &counter, sizeof(counter));
```

### `.rodata` - Read-Only Data

Constants, string literals, float literals:

```c
MasmSection *rodata = masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);

const char *msg = "Hello, World!";
masm_section_append_data(rodata, msg, strlen(msg) + 1);
```

### `.bss` - Uninitialized Data

Zero-initialized variables (doesn't occupy file space):

```c
MasmSection *bss = masm_get_or_create_section(masm, ".bss", MASM_SECTION_BSS);

// reserve 1024 bytes
masm_section_append_zero(bss, 1024);
```

### Data Manipulation Functions

```c
// append raw bytes
void masm_section_append_data(MasmSection *section, const void *data, size_t size);

// append zero bytes (for .bss or alignment)
void masm_section_append_zero(MasmSection *section, size_t size);
```


## Section Layout

During object file emission, sections are laid out in a standard order:

```
┌──────────────────┐
│    ELF Header    │
├──────────────────┤
│  Program Headers │
├──────────────────┤
│      .text       │ ← Executable code
├──────────────────┤
│     .rodata      │ ← Read-only constants
├──────────────────┤
│      .data       │ ← Initialized globals
├──────────────────┤
│      .bss        │ ← Uninitialized globals (no file space)
├──────────────────┤
│   Symbol Table   │
├──────────────────┤
│   String Table   │
├──────────────────┤
│  Section Headers │
└──────────────────┘
```


## Common Patterns

### Function Definition

```c
MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);

// function label
masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, 
    masm_operand_symbol("my_function")));

// prologue
masm_section_append_inst(text, masm_inst_1(MASM_IR_STACK_FRAME, 
    masm_operand_imm(16)));

// body
// ...

// return
masm_section_append_inst(text, masm_inst_1(MASM_IR_RET, result));
```

### String Literal

```c
MasmSection *rodata = masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);

// add string data
size_t offset = rodata->data_size;
const char *str = "Hello";
masm_section_append_data(rodata, str, strlen(str) + 1);

// create symbol pointing to string
MasmSymbol *sym = masm_symbol_create("__str_0", MASM_SYMBOL_DATA, MASM_BIND_LOCAL);
sym->section_name = strdup(".rodata");
sym->offset = offset;
sym->size = strlen(str) + 1;
masm_add_symbol(masm, sym);
```

### Global Variable

```c
MasmSection *data = masm_get_or_create_section(masm, ".data", MASM_SECTION_DATA);

// add initial value
size_t offset = data->data_size;
int64_t value = 42;
masm_section_append_data(data, &value, sizeof(value));

// create symbol
MasmSymbol *sym = masm_symbol_create("global_var", MASM_SYMBOL_DATA, MASM_BIND_GLOBAL);
sym->section_name = strdup(".data");
sym->offset = offset;
sym->size = sizeof(value);
masm_add_symbol(masm, sym);
```

### Float Literal

Float literals cannot be immediate operands on most architectures, so they are stored in `.rodata`:

```c
MasmSection *rodata = masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);

// emit float value
size_t offset = rodata->data_size;
double value = 3.14159;
masm_section_append_data(rodata, &value, sizeof(value));

// create synthetic symbol
MasmSymbol *sym = masm_symbol_create("__float_0", MASM_SYMBOL_DATA, MASM_BIND_LOCAL);
sym->section_name = strdup(".rodata");
sym->offset = offset;
sym->size = sizeof(value);
masm_add_symbol(masm, sym);
```


## Alignment

Data sections should maintain proper alignment for their contents:

| Type | Alignment |
|------|-----------|
| `u8`, `i8` | 1 byte |
| `u16`, `i16` | 2 bytes |
| `u32`, `i32`, `f32` | 4 bytes |
| `u64`, `i64`, `f64`, `ptr` | 8 bytes |

Padding can be added with `masm_section_append_zero()`:

```c
// ensure 8-byte alignment
size_t align = 8;
size_t current = section->data_size;
size_t padding = (align - (current % align)) % align;
if (padding > 0) {
    masm_section_append_zero(section, padding);
}
```


## See Also

- [Symbols](symbols.md) - Named locations in sections
- [IR Opcodes](ir.md) - Instructions stored in text sections
- [Code Generation](codegen.md) - How sections are emitted