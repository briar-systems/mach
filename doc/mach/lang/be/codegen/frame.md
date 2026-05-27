# mach.lang.be.codegen.frame

Stack frame layout and prologue / epilogue emission. Decides the
stack offsets for spill slots, allocas, and ABI-required scratch
(shadow space on Win64, red-zone on SysV AMD64); generates the
prologue (save callee-saved regs, allocate frame, set up frame
pointer) and the epilogue (reverse).

Source is `new/lang/be/codegen/frame.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(
    tgt: *target.Target,
    mir: *mir.MirModule,
) Result[bool, str]
```

Per-function:

1. Collect every spill slot from the
   [`MirVReg`](mir.md#mirvreg)s with non-negative `spill_slot`.
2. Collect every `OP_ALLOCA` from the source IR (lowered into MIR
   as static slots).
3. Compute the layout: align each slot, sum sizes, round the total
   to the ABI's
   [`stack_align`](../../target/abi.md#abivtable).
4. Determine the callee-saved set actually clobbered by this
   function (intersection of physical regs assigned by regalloc and
   the ABI's callee-saved table).
5. Emit prologue at function entry: push callee-saved regs (or
   `stp` pairs on ARM64), allocate the frame, set up the frame
   pointer if requested.
6. Emit epilogue before each `ret`: reverse of prologue.
7. Populate [`MirFrame`](mir.md#mirframe).

| Param | Type                                                  | Description                                          |
|-------|-------------------------------------------------------|------------------------------------------------------|
| tgt   | [`*target.Target`](../../target.md#target)            | Active target — supplies the ABI's stack alignment / red zone / shadow space. |
| mir   | [`*mir.MirModule`](mir.md#mirmodule-mirfunction-mirblock-mirinstr) | MIR module to mutate.                      |

Returns `ok(true)` after every function's frame has been
materialised.

## Per-ABI quirks

| ABI                                  | Quirks                                                                  |
|--------------------------------------|-------------------------------------------------------------------------|
| [`sysv`](../../target/abi/sysv.md) (AMD64) | 128-byte red zone allowed in leaf functions; can skip the frame allocation when the function fits entirely. |
| [`sysv`](../../target/abi/sysv.md) (AAPCS64) | Frame pointer pair `(FP, LR)` saved as a `stp` at function entry. |
| [`win64`](../../target/abi/win64.md) (AMD64) | 32-byte shadow space reserved by every non-leaf caller; XMM6–XMM15 callee-saved. |
| [`win64`](../../target/abi/win64.md) (AArch64) | Unwind information layout differs from SysV; this module emits the data; [`obj`](../obj.md) writes it into the object file. |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.target`](../../target.md),
[`mach.lang.be.codegen.mir`](mir.md).
