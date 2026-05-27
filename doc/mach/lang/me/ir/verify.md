# mach.lang.me.ir.verify

IR validator. Walks a [`Module`](../ir.md#module) and asserts every
invariant the rest of `me/` and `be/` rely on: SSA single-assignment,
operand types match instruction signatures, every block ends in
exactly one terminator, terminator targets exist, phis cover every
predecessor, and dominator constraints hold.

A verification failure is always a **compiler bug** — lowering or a
pass produced malformed IR — never a user source error. The verifier
therefore produces its own typed [`VerifyReport`](#verifyreport)
rather than user-facing [`diagnostic`](../../diagnostic.md)s; the
caller treats a non-empty report as an internal compiler error.

Source is `new/lang/me/ir/verify.mach` (currently empty).

## Types

### `VerifyCheck`

```mach
pub def VerifyCheck: u8;
```

Identifies which invariant a [`Violation`](#violation) failed; one of
[`VC_*`](#constants).

### `Violation`

```mach
pub rec Violation {
    function: u32;
    block:    id.BlockId;
    instr:    id.InstructionId;
    check:    VerifyCheck;
}
```

One invariant failure, located as precisely as the check allows.

| Field    | Type                                          | Description                                                |
|----------|-----------------------------------------------|------------------------------------------------------------|
| function | `u32`                                         | Index into [`Module.functions`](../ir.md#module).          |
| block    | [`id.BlockId`](id.md#blockid)                 | Offending block, or [`BLOCK_NIL`](id.md#block_nil) for function-level checks. |
| instr    | [`id.InstructionId`](id.md#instructionid)     | Offending instruction, or [`INSTR_NIL`](id.md#instr_nil) for block / function-level checks. |
| check    | [`VerifyCheck`](#verifycheck)                 | Which invariant failed.                                    |

### `VerifyReport`

```mach
pub rec VerifyReport {
    alloc:      *Allocator;
    violations: *Violation;
    count:      u32;
}
```

The verifier's output. `count == 0` means the module is well-formed.

| Field      | Type                          | Description                          |
|------------|-------------------------------|--------------------------------------|
| alloc      | `*Allocator`                  | Allocator backing `violations`.      |
| violations | [`*Violation`](#violation)    | Recorded failures, in discovery order. |
| count      | `u32`                         | Length of `violations`.              |

## Constants

```mach
pub val VC_SSA:             VerifyCheck = 0;
pub val VC_TERM_PRESENCE:   VerifyCheck = 1;
pub val VC_TERM_UNIQUE:     VerifyCheck = 2;
pub val VC_SUCC_EXISTS:     VerifyCheck = 3;
pub val VC_PRED_CONSISTENT: VerifyCheck = 4;
pub val VC_PHI_COVERAGE:    VerifyCheck = 5;
pub val VC_OPERAND_TYPE:    VerifyCheck = 6;
pub val VC_CONST_TYPE:      VerifyCheck = 7;
pub val VC_DOMINANCE:       VerifyCheck = 8;
pub val VC_REACHABILITY:    VerifyCheck = 9;
```

## Functions

### `verify_module`

```mach
pub fun verify_module(
    m:     *ir.Module,
    alloc: *Allocator,
) Result[VerifyReport, str]
```

Runs the full check suite on `m`, recording one
[`Violation`](#violation) per failure — verification does not
short-circuit on the first failure. Returns a
[`VerifyReport`](#verifyreport) (`count == 0` on a clean module);
`err` is reserved for allocation failure.

| Param | Type                              | Description                                          |
|-------|-----------------------------------|------------------------------------------------------|
| m     | [`*ir.Module`](../ir.md#module)   | Module to validate.                                  |
| alloc | `*Allocator`                      | Allocator for the report's `violations` array.       |

### `dnit_report`

```mach
pub fun dnit_report(r: *VerifyReport)
```

Releases the report's `violations` array. `nil` is a no-op.

### `describe`

```mach
pub fun describe(check: VerifyCheck) zstr
```

Returns a static human-readable description of a
[`VerifyCheck`](#verifycheck) — used by callers (the
[`pipeline`](../pipeline.md)) to format an internal-compiler-error
message from a non-empty report.

## Check suite

| Check | `VerifyCheck` | Violation |
|-------|---------------|-----------|
| SSA                     | `VC_SSA`             | A result `%n` is assigned more than once (would only happen on a bug in [`builder`](builder.md)). |
| Terminator presence     | `VC_TERM_PRESENCE`   | A non-empty block has `terminator == `[`INSTR_NIL`](id.md#instr_nil) or has a non-terminator at the end. |
| Terminator uniqueness   | `VC_TERM_UNIQUE`     | Any instruction other than `Block.terminator` is a terminator kind. |
| Successor existence     | `VC_SUCC_EXISTS`     | A terminator targets a [`BlockId`](id.md#blockid) ≥ `Function.block_count`. |
| Predecessor consistency | `VC_PRED_CONSISTENT` | `Block.preds` doesn't match the set of blocks whose terminators target this block. |
| Phi coverage            | `VC_PHI_COVERAGE`    | A phi's incoming list doesn't have exactly one entry per predecessor. |
| Operand typing          | `VC_OPERAND_TYPE`    | An operand's [`Value.ty`](value.md#value) doesn't match the slot's expected type per [`InstrKind`](instruction.md#instrkind). |
| Constant typing         | `VC_CONST_TYPE`      | A `VAL_CONST_*` operand carries a type incompatible with its kind (e.g. `VAL_CONST_FLOAT` with `IRT_INT`). |
| Dominance               | `VC_DOMINANCE`       | A `VAL_INSTR` operand references an instruction that doesn't dominate the use site. |
| Reachability            | `VC_REACHABILITY`    | A block other than the entry has empty `preds` and is referenced nowhere. |

The verifier runs unconditionally in debug builds; release builds
skip it unless explicitly invoked. Backend passes assume verified
input — they are allowed to crash on malformed IR.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.id`](id.md),
[`mach.lang.me.ir.instruction`](instruction.md),
[`mach.lang.me.ir.value`](value.md),
[`mach.lang.me.ir.type`](type.md).
