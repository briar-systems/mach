# Types

This document describes the type system used in MASM.

## Overview

MASM carries type information throughout the compilation pipeline to enable correct instruction selection, register allocation, and memory layout computation.


## Type Kinds

| Kind | Size | Description |
|------|------|-------------|
| `MASM_TYPE_VOID` | 0 | No type / void return |
| `MASM_TYPE_I8` | 1 | Signed 8-bit integer |
| `MASM_TYPE_U8` | 1 | Unsigned 8-bit integer |
| `MASM_TYPE_I16` | 2 | Signed 16-bit integer |
| `MASM_TYPE_U16` | 2 | Unsigned 16-bit integer |
| `MASM_TYPE_I32` | 4 | Signed 32-bit integer |
| `MASM_TYPE_U32` | 4 | Unsigned 32-bit integer |
| `MASM_TYPE_I64` | 8 | Signed 64-bit integer |
| `MASM_TYPE_U64` | 8 | Unsigned 64-bit integer |
| `MASM_TYPE_F32` | 4 | 32-bit IEEE 754 float |
| `MASM_TYPE_F64` | 8 | 64-bit IEEE 754 double |
| `MASM_TYPE_PTR` | 8 | Pointer (64-bit on x86_64) |
| `MASM_TYPE_ARRAY` | varies | Fixed-size array |
| `MASM_TYPE_RECORD` | varies | Record (struct) |
| `MASM_TYPE_FUNCTION` | N/A | Function signature |


## Type Structure

```c
typedef struct MasmType {
    MasmTypeKind kind;
    size_t       size;      // size in bytes
    size_t       align;     // alignment in bytes
    
    // for arrays
    MasmType *elem_type;
    size_t    elem_count;
    
    // for records
    struct {
        MasmType **fields;
        size_t     count;
        size_t    *offsets;
    } record;
    
    // for functions
    struct {
        MasmType  *ret_type;
        MasmType **params;
        size_t     param_count;
        bool       is_variadic;
    } function;
} MasmType;
```


## Primitive Types

Primitive types have fixed sizes and alignments:

| Type | Size | Alignment | Register Class |
|------|------|-----------|----------------|
| `i8`, `u8` | 1 | 1 | Integer |
| `i16`, `u16` | 2 | 2 | Integer |
| `i32`, `u32` | 4 | 4 | Integer |
| `i64`, `u64` | 8 | 8 | Integer |
| `f32` | 4 | 4 | Float (XMM) |
| `f64` | 8 | 8 | Float (XMM) |
| `ptr` | 8 | 8 | Integer |

### Creating Primitives

```c
MasmType *t = masm_type_create(MASM_TYPE_I64);
```

Primitive type sizes and alignments are set automatically.


## Array Types

Arrays are fixed-size sequences of elements.

### Structure

- `elem_type` - Type of each element
- `elem_count` - Number of elements
- `size` - `elem_type->size * elem_count`
- `align` - Same as `elem_type->align`

### Creating Arrays

```c
MasmType *elem = masm_type_create(MASM_TYPE_I32);
MasmType *arr = masm_type_create_array(elem, 10);
// arr->size == 40 (10 * 4)
// arr->align == 4
```

### Example

Mach code:
```mach
val data: [4]u64 = [4]u64{ 1, 2, 3, 4 };
```

MASM type:
```c
elem_type: MASM_TYPE_U64
elem_count: 4
size: 32
align: 8
```


## Record Types

Records (structs) are collections of named fields.

### Structure

- `record.fields` - Array of field types
- `record.count` - Number of fields
- `record.offsets` - Byte offset of each field
- `size` - Total size including padding
- `align` - Maximum alignment of any field

### Creating Records

```c
MasmType *fields[2];
fields[0] = masm_type_create(MASM_TYPE_I32);  // x
fields[1] = masm_type_create(MASM_TYPE_I32);  // y
MasmType *point = masm_type_create_record(fields, 2);
// point->size == 8
// point->align == 4
// point->record.offsets == [0, 4]
```

### Layout Rules

Fields are laid out sequentially with padding for alignment:

```mach
rec Example {
    a: u8;   # offset 0, size 1
    # padding: 7 bytes for alignment
    b: u64;  # offset 8, size 8
    c: u8;   # offset 16, size 1
    # padding: 7 bytes for struct alignment
}
# total size: 24, alignment: 8
```

### Example

Mach code:
```mach
rec Point {
    x: f64;
    y: f64;
}
```

MASM type:
```c
record.fields: [f64, f64]
record.count: 2
record.offsets: [0, 8]
size: 16
align: 8
```


## Function Types

Function types describe callable signatures.

### Structure

- `function.ret_type` - Return type (or void)
- `function.params` - Array of parameter types
- `function.param_count` - Number of parameters
- `function.is_variadic` - True if function accepts `...`

### Creating Functions

```c
MasmType *params[2];
params[0] = masm_type_create(MASM_TYPE_I64);
params[1] = masm_type_create(MASM_TYPE_I64);
MasmType *ret = masm_type_create(MASM_TYPE_I64);

MasmType *fn = masm_type_create_function(ret, params, 2, false);
```

### Example

Mach code:
```mach
fun add(a: i64, b: i64) i64;
```

MASM type:
```c
function.ret_type: i64
function.params: [i64, i64]
function.param_count: 2
function.is_variadic: false
```

### Variadic Functions

```mach
ext "C:printf" printf: fun(fmt: *u8, ...) i32;
```

MASM type:
```c
function.ret_type: i32
function.params: [ptr]
function.param_count: 1
function.is_variadic: true
```


## Type-Directed Code Generation

Types influence instruction selection:

### Register Class

```c
if (type->kind == MASM_TYPE_F32 || type->kind == MASM_TYPE_F64) {
    // use XMM registers
    reg = allocate_float_reg();
} else {
    // use general-purpose registers
    reg = allocate_int_reg();
}
```

### Instruction Selection

```c
// addition
if (is_float_type(type)) {
    emit(MASM_OP_X86_ADDSD, dst, src);  // SSE
} else {
    emit(MASM_OP_X86_ADD_RR, dst, src);  // integer
}
```

### Memory Operations

```c
// load
if (type->kind == MASM_TYPE_F64) {
    emit(MASM_OP_X86_MOVQ, dst, mem);   // movsd for f64
} else if (type->size == 8) {
    emit(MASM_OP_X86_MOV_RM, dst, mem); // mov for i64/ptr
} else if (type->size == 4) {
    emit(MASM_OP_X86_MOV_RM, dst, mem); // mov for i32
}
```


## Size and Alignment

### Querying

```c
size_t size = type->size;
size_t align = type->align;
```

### Common Values

| Type | Size | Alignment |
|------|------|-----------|
| `u8` / `i8` | 1 | 1 |
| `u16` / `i16` | 2 | 2 |
| `u32` / `i32` / `f32` | 4 | 4 |
| `u64` / `i64` / `f64` / `ptr` | 8 | 8 |

### Computing Padded Size

For arrays and stack allocation:

```c
size_t aligned_size(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
}
```


## Cleanup

Types should be destroyed when no longer needed:

```c
void masm_type_destroy(MasmType *type);
```

This recursively frees nested types (array elements, record fields, function parameters).


## See Also

- [Operands](operands.md) - Type operands in instructions
- [ABI](abi.md) - How types affect calling conventions
- [Code Generation](codegen.md) - Type-directed instruction selection