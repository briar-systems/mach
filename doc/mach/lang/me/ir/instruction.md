# mach.lang.me.ir.instruction

IR instruction set. Defines [`Instruction`](#instruction), the
[`InstrKind`](#instrkind) enumeration, and the operand-encoding
conventions. Instruction handles are
[`id.InstructionId`](id.md#instructionid). Every instruction lives
in a [`Block`](../ir.md#block) and either produces a
[`VAL_INSTR`](value.md#valuekind) result or, for terminators and
stores, produces no result.

Source is `new/lang/me/ir/instruction.mach` (currently empty).

## Types

### `InstrKind`

```mach
pub def InstrKind: u8;
```

Discriminator for the operation an instruction performs.

### `Instruction`

```mach
pub rec Instruction {
    kind:     InstrKind;
    ty:       ir_type.IrTypeId;
    operands: *value.Value;
    operand_count: u32;
    block:    id.BlockId;
    flags:    u16;
}
```

| Field         | Type                                          | Description                                                |
|---------------|-----------------------------------------------|------------------------------------------------------------|
| kind          | [`InstrKind`](#instrkind)                     | One of [`OP_*`](#constants).                               |
| ty            | [`ir_type.IrTypeId`](type.md#irtypeid)        | Type of the produced result; [`IRT_VOID`](type.md#constants) for terminators / stores. |
| operands      | [`*value.Value`](value.md#value)              | Operand vector. Kind decides the count and meaning.        |
| operand_count | `u32`                                         | Length of `operands`.                                      |
| block         | [`id.BlockId`](id.md#blockid)                 | Owning block.                                              |
| flags         | `u16`                                         | Bitfield of [`INSTR_FLAG_*`](#constants).                  |

## Constants

```mach
# arithmetic
pub val OP_ADD:    InstrKind = 0;
pub val OP_SUB:    InstrKind = 1;
pub val OP_MUL:    InstrKind = 2;
pub val OP_DIV_S:  InstrKind = 3;
pub val OP_DIV_U:  InstrKind = 4;
pub val OP_REM_S:  InstrKind = 5;
pub val OP_REM_U:  InstrKind = 6;
pub val OP_NEG:    InstrKind = 7;

# bitwise
pub val OP_AND:    InstrKind = 8;
pub val OP_OR:     InstrKind = 9;
pub val OP_XOR:    InstrKind = 10;
pub val OP_SHL:    InstrKind = 11;
pub val OP_SHR_S:  InstrKind = 12;
pub val OP_SHR_U:  InstrKind = 13;
pub val OP_NOT:    InstrKind = 14;

# comparison
pub val OP_CMP_EQ: InstrKind = 15;
pub val OP_CMP_NE: InstrKind = 16;
pub val OP_CMP_LT_S: InstrKind = 17;
pub val OP_CMP_LT_U: InstrKind = 18;
pub val OP_CMP_LE_S: InstrKind = 19;
pub val OP_CMP_LE_U: InstrKind = 20;

# conversion
pub val OP_TRUNC:    InstrKind = 21;
pub val OP_SEXT:     InstrKind = 22;
pub val OP_ZEXT:     InstrKind = 23;
pub val OP_FP_TRUNC: InstrKind = 24;
pub val OP_FP_EXT:   InstrKind = 25;
pub val OP_FP_TO_SI: InstrKind = 26;
pub val OP_FP_TO_UI: InstrKind = 27;
pub val OP_SI_TO_FP: InstrKind = 28;
pub val OP_UI_TO_FP: InstrKind = 29;
pub val OP_BITCAST:  InstrKind = 30;

# memory
pub val OP_ALLOCA: InstrKind = 31;
pub val OP_LOAD:   InstrKind = 32;
pub val OP_STORE:  InstrKind = 33;
pub val OP_GEP:    InstrKind = 34;

# aggregate
pub val OP_EXTRACT: InstrKind = 35;
pub val OP_INSERT:  InstrKind = 36;

# control
pub val OP_PHI:         InstrKind = 37;
pub val OP_CALL:        InstrKind = 38;
pub val OP_BR:          InstrKind = 39;
pub val OP_CBR:         InstrKind = 40;
pub val OP_RET:         InstrKind = 41;
pub val OP_UNREACHABLE: InstrKind = 42;

# inline asm
pub val OP_ASM: InstrKind = 43;
```

| Group       | Operands                                                                  |
|-------------|---------------------------------------------------------------------------|
| Arithmetic / bitwise / comparison | Two operands (one for `NEG` / `NOT`). Result type matches operand type for arithmetic; comparisons produce an 8-bit `IRT_INT` — the IR has no 1-bit type (see [`ir.type`](type.md)). |
| Conversion  | One operand; target type carried in `ty`.                                 |
| `ALLOCA`    | One operand: size in elements (a [`const_int`](value.md#functions) for fixed-size, a Value for dynamic). `ty` is the pointer type to the allocated region. |
| `LOAD`      | One operand: pointer.                                                     |
| `STORE`     | Two operands: `[value, pointer]`. Produces void.                          |
| `GEP`       | Variable: `[base_ptr, index0, index1, ...]` — element pointer computation.  |
| `EXTRACT`   | Two operands: `[aggregate, index_const]`.                                 |
| `INSERT`    | Three operands: `[aggregate, value, index_const]`.                        |
| `PHI`       | `2*n` operands: `[block0, val0, block1, val1, ...]` — paired by predecessor. |
| `CALL`      | `[callee, arg0, arg1, ...]`. Callee is a [`VAL_FN`](value.md#valuekind) or `VAL_INSTR` (indirect). |
| `BR`        | One operand encoding a [`id.BlockId`](id.md#blockid) as a Value. Terminator. |
| `CBR`       | Three operands: `[cond, then_block, else_block]`. Terminator.             |
| `RET`       | Zero or one operand. Terminator.                                          |
| `UNREACHABLE` | No operands. Terminator.                                                |
| `ASM`       | Operands are the `IR_ASM_IN` / `IR_ASM_OUT` Values, in order. Template, ISA, and operand roles / constraints live in the instruction's [`IrAsm`](../ir.md#irasm) payload, reached via [`Function.asm`](../ir.md#function). |

```mach
pub val INSTR_FLAG_NSW:      u16 = 0x01;
pub val INSTR_FLAG_NUW:      u16 = 0x02;
pub val INSTR_FLAG_EXACT:    u16 = 0x04;
pub val INSTR_FLAG_VOLATILE: u16 = 0x08;
```

| Constant            | Applies to              | Meaning                                                  |
|---------------------|-------------------------|----------------------------------------------------------|
| `INSTR_FLAG_NSW`    | `ADD` / `SUB` / `MUL` / `SHL` | No signed overflow (UB if violated; passes may exploit). |
| `INSTR_FLAG_NUW`    | same                    | No unsigned overflow.                                    |
| `INSTR_FLAG_EXACT`  | `DIV` / `SHR`           | Exact (no truncation); UB if violated.                   |
| `INSTR_FLAG_VOLATILE` | `LOAD` / `STORE`      | Side-effecting; passes must not reorder or elide.        |

## Varargs

`va_start` / `va_arg` / `va_end` have no dedicated
[`InstrKind`](#instrkind). They are compiler-known intrinsic
functions and lower to ordinary `OP_CALL`s. The backend recognises
the intrinsic callee and expands each call into the target ABI's
varargs sequence during MIR lowering — `me/ir` itself stays free of
ABI-specific operations.

## Functions

### `is_terminator`

```mach
pub fun is_terminator(k: InstrKind) bool
```

`true` for `OP_BR`, `OP_CBR`, `OP_RET`, `OP_UNREACHABLE`.

### `is_pure`

```mach
pub fun is_pure(k: InstrKind) bool
```

`true` when the instruction has no observable side effects and the
result is solely a function of its operands. Used by
[`dce`](../pass/dce.md) and
[`mem2reg`](../pass/mem2reg.md).

## Dependencies

`std.types.bool`, `std.types.size`,
[`mach.lang.me.ir.id`](id.md),
[`mach.lang.me.ir.type`](type.md),
[`mach.lang.me.ir.value`](value.md).
