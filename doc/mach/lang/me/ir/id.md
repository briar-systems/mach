# mach.lang.me.ir.id

IR index-handle types. Leaf module holding the `u32` handles the IR
data model passes around by value: [`BlockId`](#blockid) and
[`InstructionId`](#instructionid).

Kept in their own module so that [`ir.value`](value.md) and
[`ir.instruction`](instruction.md) — which reference each other's
handles — can both depend on this leaf instead of forming an import
cycle. [`ir`](../ir.md) re-exports these types, so consumers that
already import `ir` reach them as `ir.BlockId` / `ir.InstructionId`.

Source is `new/lang/me/ir/id.mach` (currently empty).

## Types

### `BlockId`

```mach
pub def BlockId: u32;
```

Index into [`Function.blocks`](../ir.md#function); `0` always denotes
the entry block.

### `InstructionId`

```mach
pub def InstructionId: u32;
```

Function-local handle into the flat instruction pool owned by
[`Function`](../ir.md#function) (storage layout left to the
implementation — a typed pool, not exposed in this spec).

## Constants

### `BLOCK_NIL`

```mach
pub val BLOCK_NIL: BlockId = 0xFFFFFFFF;
```

Absent-[`BlockId`](#blockid) sentinel — the builder's cursor before
the first [`set_block`](builder.md#init-set_function-set_block).

### `INSTR_NIL`

```mach
pub val INSTR_NIL: InstructionId = 0xFFFFFFFF;
```

Absent-[`InstructionId`](#instructionid) sentinel —
[`Block.terminator`](../ir.md#block) before a terminator is emitted.

Consumers receiving either sentinel where a real handle is expected
must treat it as an error or as the explicit "no value" case, never
as a default.

## Dependencies

`std.types.size`.
