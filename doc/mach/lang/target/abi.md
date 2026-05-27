# mach.lang.target.abi

Calling-convention interface and id registry. Defines an
[`AbiVTable`](#abivtable) that every per-ABI implementation under
`target/abi/` registers, plus stable numeric ids selecting which
convention a [`Target`](../target.md#target) uses. Source is
`new/lang/target/abi.mach` (currently empty — this spec is the
design intent).

## Types

### `AbiVTable`

```mach
pub rec AbiVTable {
    id:           u32;
    name:         intern.StrId;
    classify:     *u8;
    arg_passing:  *u8;
    ret_passing:  *u8;
    stack_align:  u32;
    red_zone:     u32;
}
```

Uniform per-ABI interface. The middle-end consults it when lowering
function signatures (which arguments go in which registers, what's
passed on the stack, whether aggregates are returned by reference)
and the backend consults it when emitting prologue / epilogue
sequences.

| Field        | Type                                  | Description                                                                  |
|--------------|---------------------------------------|------------------------------------------------------------------------------|
| id           | `u32`                                 | One of [`ABI_*`](#constants).                                                |
| name         | [`intern.StrId`](../intern.md#strid)  | Canonical name (`"sysv"`, `"win64"`).                                        |
| classify     | `*u8`                                 | Opaque pointer to the per-type argument-class classifier (see [Lowering contract](#lowering-contract)). |
| arg_passing  | `*u8`                                 | Opaque pointer to the per-call argument-marshalling function.                |
| ret_passing  | `*u8`                                 | Opaque pointer to the per-call return-value-marshalling function.            |
| stack_align  | `u32`                                 | Required stack alignment at call boundaries (bytes).                         |
| red_zone     | `u32`                                 | Size of the leaf-function red zone in bytes (0 if absent).                   |

The three `*u8` function pointers follow the same type-erasure
pattern as [`QueryShard.compute`](../query.md#queryshard) — registered
out-of-band, cast back to their concrete signatures by the consumer.

## Constants

```mach
pub val ABI_UNKNOWN: u32 = 0;
pub val ABI_SYSV:    u32 = 1;
pub val ABI_WIN64:   u32 = 2;
```

| Constant       | Value | Meaning                                                                |
|----------------|-------|------------------------------------------------------------------------|
| `ABI_UNKNOWN`  | 0     | Unrecognised ABI (default fallback).                                   |
| `ABI_SYSV`     | 1     | System V AMD64 / AAPCS64 family (Linux, Darwin, BSD).                  |
| `ABI_WIN64`    | 2     | Microsoft x64 / AArch64 Windows ABI.                                   |

## Lowering contract

A `classify(type) → ArgClass` function partitions a logical argument
or return type into the per-ABI register / memory classes used by
the platform calling convention (e.g. SysV's INTEGER, SSE, MEMORY
classes). The middle-end uses this when materialising the per-call
register allocation.

`arg_passing` and `ret_passing` are higher-level entry points that
consume the classification and emit the MIR-level register-and-stack
assignments. They are not direct callers of `classify` — the
middle-end orchestrates the dance — but their behaviour is defined
against the same classes.

The exact shape of `ArgClass` and the marshalling functions is
deferred to the platform impls; this parent spec only pins the
dispatch surface.

## Functions

### `lookup`

```mach
pub fun lookup(abi_id: u32) Option[*AbiVTable]
```

Returns the registered [`AbiVTable`](#abivtable) for `abi_id`, or
`none` if no impl is linked into this build. Called by
[`target.select`](../target.md#select).

### `register`

```mach
pub fun register(vt: *AbiVTable)
```

Registers a platform impl's vtable in the ABI registry. Each
implementation file under [`target/abi/`](../target.md) self-registers
from its module-init function; the compiler imports every ABI impl it
supports, so each runs exactly once before `main`. The registry is
process-global, write-once at startup, read-only thereafter, and
never freed — it holds static platform data, not per-session state.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
[`mach.lang.intern`](../intern.md).
