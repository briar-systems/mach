# mach.lang.be.codegen.regalloc

Register allocator. Resolves virtual register operands
(`MIR_OP_VREG`) to physical registers (`MIR_OP_PREG`) per the
active ISA's register classes
([`target.arch.reg_classes`](../../target/isa.md#isavtable)) and
the active ABI's callee-saved / caller-saved partition
([`target.abi`](../../target/abi.md#abivtable)). Spills go into the
function's stack frame.

Source is `new/lang/be/codegen/regalloc.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(
    tgt: *target.Target,
    mir: *mir.MirModule,
) Result[bool, str]
```

Per-function:

1. **Compute live intervals.** Linear-scan over the
   [`MirBlock`](mir.md#mirmodule-mirfunction-mirblock-mirinstr) RPO; for
   each vreg, record `[first_use, last_use]` ranges (one range per
   vreg, conservatively merged across blocks).
2. **Assign.** Linear-scan allocator. For each interval in start
   order, pick a free physical register in the vreg's class; if no
   register is free, spill the interval with the latest end point
   (or the new one if it ends latest).
3. **Rewrite operands.** Each `MIR_OP_VREG` is rewritten to
   `MIR_OP_PREG` (assigned register) or `MIR_OP_MEM` against the
   frame pointer at the spill slot's displacement.
4. **Resolve phis.** Insert moves at predecessor block ends to
   materialise the phi's destination from each incoming Value.
5. **Reload spilled values.** Insert `load`s before each use and
   `store`s after each def of a spilled vreg.

Reserved registers (e.g. `rsp` / `rbp` on x86\_64) are excluded
from the allocatable pool by the per-arch register-class
description; the allocator never sees them as candidates.

| Param | Type                                                  | Description                                          |
|-------|-------------------------------------------------------|------------------------------------------------------|
| tgt   | [`*target.Target`](../../target.md#target)            | Active target — provides register classes and ABI calling convention. |
| mir   | [`*mir.MirModule`](mir.md#mirmodule-mirfunction-mirblock-mirinstr) | MIR module to mutate.                      |

Returns `ok(true)` after every function has been allocated.

## ABI integration

Across `OP_CALL` boundaries, regalloc:

- Reserves caller-saved registers for live values across the call —
  these get spill ranges automatically because the call clobbers
  them.
- Honours the [`AbiVTable`](../../target/abi.md#abivtable)'s
  argument-register table when materialising call setups (the
  argument vregs get pinned to the canonical arg registers).
- Honours the callee-saved register set when emitting prologue /
  epilogue saves (handed off to [`frame`](frame.md)).

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.target`](../../target.md),
[`mach.lang.be.codegen.mir`](mir.md).
