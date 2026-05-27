# mach.lang.me.pass.dce

Dead code elimination. Removes instructions whose results are unused
and whose [`is_pure`](../ir/instruction.md#is_pure) is true, and
blocks that no `br` / `cbr` terminator targets (unreachable code).

Source is `new/lang/me/pass/dce.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(m: *ir.Module) Result[bool, str]
```

Single fixed-point pass:

1. Compute a use map: for every [`Instruction`](../ir/instruction.md#instruction),
   the count of `VAL_INSTR` operands referencing its
   [`InstructionId`](../ir/id.md#instructionid).
2. Worklist of unused-pure instructions; popping one drops it from
   its block and decrements the use count of its operands. New zeros
   feed back into the worklist.
3. Reachability sweep: mark every block reachable from the entry
   block ([`BlockId`](../ir/id.md#blockid) `0`); blocks left unmarked
   are removed (the function's `block` array compacts; phis in
   surviving successors are rewritten).

Volatile loads / stores and calls (which may have side effects) are
preserved unconditionally even when their results are unused.

| Param | Type                              | Description                                          |
|-------|-----------------------------------|------------------------------------------------------|
| m     | [`*ir.Module`](../ir.md#module)   | Module to mutate in place.                           |

Returns `ok(true)` on a successful pass.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.instruction`](../ir/instruction.md),
[`mach.lang.me.ir.value`](../ir/value.md).
