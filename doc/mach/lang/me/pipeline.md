# mach.lang.me.pipeline

Middle-end pass pipeline. Runs the canonical sequence of
optimisation passes against an [`ir.Module`](ir.md#module) and
returns the same module mutated in place. The pipeline shape is
fixed by optimisation level; passes are not user-pluggable.

[`run`](#run) is invoked by the [`Q_LOWER`](../query.md#integration)
compute body directly after [`lower_module`](lower.md#lower_module),
before the `ir.Module` is published as the query result. The
in-place mutation is therefore internal to the query — the
unoptimised IR is never observed by another consumer, so the query
cache holds only the final optimised module.

Source is `new/lang/me/pipeline.mach` (currently empty).

## Types

### `OptLevel`

```mach
pub def OptLevel: u8;
```

| Constant     | Value | Meaning                                                |
|--------------|-------|--------------------------------------------------------|
| `OPT_DEBUG`  | 0     | Verifier + `mem2reg` only. Default for `mach build`.    |
| `OPT_RELEASE`| 1     | Full pipeline (see [Stages](#stages)).                  |

## Constants

```mach
pub val OPT_DEBUG:   OptLevel = 0;
pub val OPT_RELEASE: OptLevel = 1;
```

## Functions

### `run`

```mach
pub fun run(
    m:     *ir.Module,
    tgt:   *target.Target,
    level: OptLevel,
) Result[bool, str]
```

Drives the pipeline. Always runs
[`verify_module`](ir/verify.md#verify_module) before and after the
level's stages, using `m.alloc` for the
[`VerifyReport`](ir/verify.md#verifyreport). A non-empty report is
an internal compiler error — pipeline failures are impossible from a
verified input, and a post-pipeline failure is a pass bug.

| Param | Type                                              | Description                                          |
|-------|---------------------------------------------------|------------------------------------------------------|
| m     | [`*ir.Module`](ir.md#module)                      | Module to optimise.                                  |
| tgt   | [`*target.Target`](../target.md#target)           | Target — passed to passes that need it (inline cost model). |
| level | [`OptLevel`](#optlevel)                           | Selects the stage sequence below.                    |

A clean pipeline returns `ok(true)`. A non-empty
[`VerifyReport`](ir/verify.md#verifyreport) is formatted — via
[`verify.describe`](ir/verify.md#describe) — into the returned
`err: str`, which the driver surfaces as an internal compiler error.

## Stages

| Stage             | Debug | Release | Notes                                                                       |
|-------------------|-------|---------|-----------------------------------------------------------------------------|
| `verify` (entry)  | ✓     | ✓       | Sanity check the input.                                                     |
| `mem2reg`         | ✓     | ✓       | Promote allocas. Always runs — code generation expects mostly-SSA input.    |
| `dce` (early)     |       | ✓       | Sweep up the dead loads / stores `mem2reg` left behind.                     |
| `inline`          |       | ✓       | Inline single-use and small functions.                                      |
| `dce` (late)      |       | ✓       | Sweep up unused calls / now-dead branches from inlining.                    |
| `verify` (exit)   | ✓     | ✓       | Final invariant check before the backend takes over.                        |

Future passes (constant folding, GVN, loop transforms) plug in
between the early and late `dce` runs; the sequence is intentionally
small for the rewrite.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.target`](../target.md),
[`mach.lang.me.ir`](ir.md),
[`mach.lang.me.ir.verify`](ir/verify.md),
[`mach.lang.me.pass.dce`](pass/dce.md),
[`mach.lang.me.pass.inline`](pass/inline.md),
[`mach.lang.me.pass.mem2reg`](pass/mem2reg.md).
