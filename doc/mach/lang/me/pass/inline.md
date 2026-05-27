# mach.lang.me.pass.inline

Function inliner. Replaces call instructions whose callee passes the
inlining heuristic with the callee body, rewriting parameters into
the caller's value space and stitching the control-flow graph.

Source is `new/lang/me/pass/inline.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(m: *ir.Module, tgt: *target.Target) Result[bool, str]
```

Pass entry. Iterates every function and considers every direct
`call` instruction.

| Param | Type                                      | Description                                          |
|-------|-------------------------------------------|------------------------------------------------------|
| m     | [`*ir.Module`](../ir.md#module)           | Module to mutate.                                    |
| tgt   | [`*target.Target`](../../target.md#target)| Target — only used for cost-model parameters (target-derived inline thresholds). |

Returns `ok(true)` after a full pass.

## Heuristic

A call to `f` is inlined when **any** of the following hold:

- `f` carries [`FN_FLAG_INLINE`](../ir.md#constants) — set by lowering
  when the user requested it via the `$<fn>.inline = true` comptime
  setting (see [`decl.md` comptime settings](../../fe/ast/decl.md#declcomptimeattr)).
- `f`'s instruction count is below the inline threshold (~25
  instructions). `inline` only runs in the release pipeline, so
  there is no separate debug threshold.
- `f` is called exactly once across the whole module (single-use
  inline always wins).

Recursive functions are not considered. Calls inside an `OP_ASM`
operand list are likewise not considered.

## Mechanics

1. Pick a candidate `call` instruction at site `S` in caller `C`.
2. Clone the callee's blocks into `C` with fresh
   [`BlockId`](../ir/id.md#blockid)s and fresh
   [`InstructionId`](../ir/id.md#instructionid)s.
3. Substitute each `VAL_PARAM` reference in the clone with the
   corresponding actual argument from `S`.
4. Split `S`'s block at the call site; everything after the call
   moves into a continuation block.
5. Rewrite each `ret` in the clone as `br` to the continuation,
   collecting return values for a `phi` in the continuation if the
   callee returns a value.
6. Replace `S`'s result Value with the continuation phi (or with
   the single ret value when there's only one return point).
7. Drop `S`.

The result is re-run through [`mem2reg`](mem2reg.md) and
[`dce`](dce.md) downstream — the inliner intentionally produces
mechanical output and trusts the cleanup passes.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.target`](../../target.md),
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.builder`](../ir/builder.md),
[`mach.lang.me.ir.instruction`](../ir/instruction.md),
[`mach.lang.me.ir.value`](../ir/value.md).
