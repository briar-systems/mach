# Types

This document describes the machine-level type system used in MASM.

## Overview

MASM carries type information throughout the compilation pipeline to enable correct instruction selection, register allocation, and memory layout computation.

The MASM type system is defined in `src/compiler/masm/type.mach`. It is a minimal machine-level representation: each type records only its size, alignment, and kind. There are no recursive nested types — compound types (arrays, structs) store their total size and element alignment directly.


## Type Kinds

| Kind | Value | Description |
|------|-------|-------------|
| `TK_VOID` | 0 | No type / void return |
| `TK_INT` | 1 | Integer (signed or unsigned) |
| `TK_FLOAT` | 2 | IEEE 754 floating-point |
| `TK_PTR` | 3 | Pointer |
| `TK_ARRAY` | 4 | Fixed-size array |
| `TK_STRUCT` | 5 | Record (struct) |


## Type Structure

```mach
pub rec Type {
    size:  usize;     # size in bytes
    align: usize;     # alignment in bytes
    kind:  TypeKind;
}
```

All compound type information (element type, field offsets, etc.) lives in the semantic layer. At the MASM level, only `size`, `align`, and `kind` matter for code generation.


## Constructors

```mach
type.void_type()                                    # size 0, align 1
type.int_type(size: usize)                          # TK_INT
type.float_type(size: usize)                        # TK_FLOAT
type.ptr_type(ptr_size: usize)                      # TK_PTR
type.array_type(elem_size, elem_align, count)        # TK_ARRAY
type.struct_type(size: usize, align: usize)          # TK_STRUCT
```

### Convenience Constructors

```mach
type.i8()     # int_type(1)
type.i16()    # int_type(2)
type.i32()    # int_type(4)
type.i64()    # int_type(8)
type.f32()    # float_type(4)
type.f64()    # float_type(8)
```


## Primitive Types

| Type | Size | Alignment | Kind | Register Class |
|------|------|-----------|------|----------------|
| `i8`, `u8` | 1 | 1 | `TK_INT` | Integer |
| `i16`, `u16` | 2 | 2 | `TK_INT` | Integer |
| `i32`, `u32` | 4 | 4 | `TK_INT` | Integer |
| `i64`, `u64` | 8 | 8 | `TK_INT` | Integer |
| `f32` | 4 | 4 | `TK_FLOAT` | Float (XMM) |
| `f64` | 8 | 8 | `TK_FLOAT` | Float (XMM) |
| `ptr` | 8 | 8 | `TK_PTR` | Integer |


## Array Types

Arrays store total size and element alignment.

```mach
# [4]u64 → total size 32, align 8
val t: type.Type = type.array_type(8, 8, 4);
# t.size  == 32
# t.align == 8
# t.kind  == TK_ARRAY
```


## Struct Types

Struct types are created from the final computed size and alignment (after layout by the semantic layer):

```mach
# rec Point { x: f64; y: f64; }  →  size 16, align 8
val t: type.Type = type.struct_type(16, 8);
```


## Utilities

### Alignment Padding

```mach
type.align_to(size: usize, align: usize) usize
```

Rounds `size` up to the nearest multiple of `align` (must be a power of 2).

```mach
type.align_to(3, 4)   # → 4
type.align_to(9, 8)   # → 16
```

### Type Predicates

```mach
type.is_float(t: *Type) bool       # TK_FLOAT
type.is_aggregate(t: *Type) bool   # TK_STRUCT or TK_ARRAY
```


## Type-Directed Code Generation

Types influence instruction selection:

### Register Class

```mach
if (type.is_float(?t)) {
    # use XMM registers (SSE instructions)
} # else: general-purpose registers
```

### Instruction Selection

```mach
# integer add
ir.block_append(alloc, block, ir.OP_ADD, dst, src1, src2, ir.CC_NONE);

# float add
ir.block_append(alloc, block, ir.OP_FADD, dst, src1, src2, ir.CC_NONE);
```

### Size Field

Each `Inst` carries a `size` field (in bytes) that the encoder uses to select the correct instruction variant:

```mach
block.insts[idx].size = t.size::u8;
```


## Common Size/Alignment Values

| Type | Size | Alignment |
|------|------|-----------|
| `u8` / `i8` | 1 | 1 |
| `u16` / `i16` | 2 | 2 |
| `u32` / `i32` / `f32` | 4 | 4 |
| `u64` / `i64` / `f64` / `ptr` | 8 | 8 |


## See Also

- [Operands](operands.md) - How types affect operand size fields
- [ABI](abi.md) - How types affect calling conventions
- [Code Generation](codegen.md) - Type-directed instruction selection
