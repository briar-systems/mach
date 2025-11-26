# MIR Types

This document describes the type system in MIR.

## Type Representation

All MIR instructions and values carry type information used for:
- Instruction selection (integer vs. floating-point operations)
- Register allocation (general-purpose vs. XMM registers)
- Memory layout calculation
- ABI compliance (parameter passing)

## Type Kinds

```c
typedef enum TypeKind {
    // Primitives
    TYPE_U8, TYPE_U16, TYPE_U32, TYPE_U64,
    TYPE_I8, TYPE_I16, TYPE_I32, TYPE_I64,
    TYPE_F32, TYPE_F64,
    TYPE_PTR,  // Untyped pointer
    
    // Compound types
    TYPE_POINTER,  // Typed pointer (*T)
    TYPE_ARRAY,    // Fixed-size array ([N]T)
    TYPE_FUNCTION, // Function type
    TYPE_STRUCT,   // Record
    TYPE_UNION     // Union
} TypeKind;
```

## Primitive Types

| Type | Size | Description |
|------|------|-------------|
| `u8` | 1 | Unsigned 8-bit integer |
| `u16` | 2 | Unsigned 16-bit integer |
| `u32` | 4 | Unsigned 32-bit integer |
| `u64` | 8 | Unsigned 64-bit integer |
| `i8` | 1 | Signed 8-bit integer |
| `i16` | 2 | Signed 16-bit integer |
| `i32` | 4 | Signed 32-bit integer |
| `i64` | 8 | Signed 64-bit integer |
| `f32` | 4 | 32-bit IEEE 754 float |
| `f64` | 8 | 64-bit IEEE 754 double |
| `ptr` | 8 | Untyped pointer (64-bit) |

## Pointer Types

### Typed Pointers

Pointers to specific types, with const qualification:

```c
Type *int_ptr = type_create_pointer(type_get_primitive(TYPE_I64), false);  // *i64
Type *const_ptr = type_create_pointer(type_get_primitive(TYPE_U8), true);  // &u8
```

### Untyped Pointers

Generic `ptr` type (like C's `void*`):

```c
Type *ptr = type_get_primitive(TYPE_PTR);
```

## Array Types

Fixed-size arrays with element type and count:

```c
Type *array = type_create_array(type_get_primitive(TYPE_I32), 10);  // [10]i32
```

Properties:
- Fixed size known at compile time
- Contiguous memory layout
- Element access via pointer arithmetic

## Struct Types

Records with named fields:

```c
TypeField fields[] = {
    { "x", type_get_primitive(TYPE_F64), 0 },
    { "y", type_get_primitive(TYPE_F64), 8 }
};
Type *point = type_create_struct("Point", fields, 2);
```

Memory layout is sequential with proper alignment.

## Function Types

Function signatures with parameters and return type:

```c
Type *param_types[] = { 
    type_get_primitive(TYPE_I64),
    type_get_primitive(TYPE_I64)
};
Type *fn = type_create_function(
    type_get_primitive(TYPE_I64),  // return type
    param_types,
    2  // param count
);
```

## Type Properties

Each type has:
- **Size** - Memory size in bytes
- **Alignment** - Required memory alignment
- **Kind** - Type category (primitive, pointer, etc.)

```c
struct Type {
    TypeKind kind;
    size_t size;
    size_t alignment;
    // ... kind-specific data
};
```

## Type Queries

Helper functions to check type properties:

```c
bool type_is_integer(Type *t);  // u8-u64, i8-i64
bool type_is_float(Type *t);    // f32, f64
bool type_is_numeric(Type *t);  // integer or float
bool type_equals(Type *a, Type *b);  // Structural equality
```

## Example Usage

### Instruction with Type

```c
// Create typed ADD instruction
MIRInst *inst = mir_inst_create(MIR_OP_ADD, type_get_primitive(TYPE_I64));
```

### Float vs. Integer Operations

Type determines which assembly instruction is used:

```c
// Integer add
inst->type = TYPE_I64;  // → ADD instruction

// Float add  
inst->type = TYPE_F64;  // → ADDSD instruction
```

### Register Class Selection

```c
bool x86_64_reg_is_fp(X86_64_Reg reg) {
    return reg >= X86_64_XMM0 && reg <= X86_64_XMM15;
}

// Allocate based on type
if (type_is_float(value->type)) {
    reg = allocate_xmm_register();
} else {
    reg = allocate_gp_register();
}
```

## Singleton Types

Primitive types are cached as singletons:

```c
Type *type_get_primitive(TypeKind kind);
```

This ensures:
- Memory efficiency (one instance per primitive)
- Fast equality checks (pointer comparison)
- Simplified type management

## Implementation

Type system is implemented in:
- `boot/include/compiler/type.h`
- `boot/src/compiler/type.c`

## See Also

- [Opcodes](opcodes.md) - How types affect operations
- [Values](values.md) - Typed SSA values
- [Codegen](codegen.md) - Type-directed code generation
