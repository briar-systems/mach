# mach.lang.me.ir.value

IR operand model. Every position that takes an input —
instruction operands, terminator targets, phi sources — references
a [`Value`](#value): a tagged union over the canonical operand
sources — instruction result, function parameter, constant, global,
and function reference.

Source is `new/lang/me/ir/value.mach` (currently empty).

## Types

### `ValueKind`

```mach
pub def ValueKind: u8;
```

| Constant         | Value | Meaning                                                |
|------------------|-------|--------------------------------------------------------|
| `VAL_INSTR`      | 0     | Result of an [`Instruction`](instruction.md).         |
| `VAL_PARAM`      | 1     | Function parameter (one entry in [`Function.params`](../ir.md#function)). |
| `VAL_CONST_INT`  | 2     | Integer literal.                                       |
| `VAL_CONST_FLOAT`| 3     | Float literal.                                         |
| `VAL_CONST_NULL` | 4     | Null pointer / zero of any type.                       |
| `VAL_CONST_BYTES`| 5     | Constant bytes blob (string / aggregate literal).      |
| `VAL_GLOBAL`     | 6     | Reference to a [`Global`](../ir.md#global) by index.   |
| `VAL_FN`         | 7     | Reference to a [`Function`](../ir.md#function) by index. |

### `ValueBytes`

```mach
pub rec ValueBytes {
    ptr: *u8;
    len: u32;
}
```

Payload for a `VAL_CONST_BYTES` value — a constant byte blob.

| Field | Type   | Description                  |
|-------|--------|------------------------------|
| ptr   | `*u8`  | Pointer to the blob bytes.   |
| len   | `u32`  | Blob length in bytes.        |

### `Value`

```mach
pub rec Value {
    kind: ValueKind;
    ty:   ir_type.IrTypeId;
    data: uni {
        instr:     id.InstructionId;
        param_ix:  u32;
        int_lit:   u64;
        float_lit: f64;
        bytes:     ValueBytes;
        global_ix: u32;
        fn_ix:     u32;
    };
}
```

Compact tagged union. `ty` is the value's IR type; the active `data`
variant is selected by `kind`.

| Field | Type                                | Description                              |
|-------|-------------------------------------|------------------------------------------|
| kind  | [`ValueKind`](#valuekind)           | Active discriminator.                    |
| ty    | [`ir_type.IrTypeId`](type.md#irtypeid) | The value's IR type.                  |
| data  | `uni { ... }`                         | Kind-specific payload (see below).       |

| `data` variant | Type                                          | Active when `kind` is ...   |
|----------------|-----------------------------------------------|-----------------------------|
| instr          | [`id.InstructionId`](id.md#instructionid)     | `VAL_INSTR`                 |
| param_ix       | `u32`                                         | `VAL_PARAM`                 |
| int_lit        | `u64`                                         | `VAL_CONST_INT`             |
| float_lit      | `f64`                                         | `VAL_CONST_FLOAT`           |
| (none)         | —                                             | `VAL_CONST_NULL`            |
| bytes          | [`ValueBytes`](#valuebytes)                   | `VAL_CONST_BYTES`           |
| global_ix      | `u32`                                         | `VAL_GLOBAL`                |
| fn_ix          | `u32`                                         | `VAL_FN`                    |

## Functions

### `instr_value`, `param_value`, `const_int`, `const_float`, `const_null`, `const_bytes`, `global_value`, `fn_value`

```mach
pub fun instr_value(instr: id.InstructionId, ty: ir_type.IrTypeId) Value
pub fun param_value(ix: u32, ty: ir_type.IrTypeId) Value
pub fun const_int(value: u64, ty: ir_type.IrTypeId) Value
# ... etc.
```

Constructors for each kind.

### `is_constant`

```mach
pub fun is_constant(v: Value) bool
```

`true` for `VAL_CONST_*` kinds — used by passes to fold operands.

## Dependencies

`std.types.bool`, `std.types.size`,
[`mach.lang.me.ir.id`](id.md),
[`mach.lang.me.ir.type`](type.md).
