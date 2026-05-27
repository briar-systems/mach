# mach.lang.me.pass.mem2reg

Promote address-not-taken allocas to SSA registers. Replaces
`alloca` + `load` / `store` chains with direct SSA Values and the
phis needed to merge across control flow. This is the dominant
optimisation by impact — the lowerer emits everything as
alloca-based mutable storage, and `mem2reg` is what turns the result
into real SSA.

Source is `new/lang/me/pass/mem2reg.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(m: *ir.Module) Result[bool, str]
```

Pass entry. Iterates every function and runs the promotion algorithm
per-function.

| Param | Type                              | Description                                          |
|-------|-----------------------------------|------------------------------------------------------|
| m     | [`*ir.Module`](../ir.md#module)   | Module to mutate.                                    |

Returns `ok(true)` after a full pass.

## Algorithm

Per function:

1. **Identify promotable allocas.** An alloca is promotable when
   every use of its result Value is a `load` or a `store` (the
   alloca's address is never captured by anything else — no
   `gep`, no `call`, no `bitcast`, no escape into a
   `VAL_*` constant). One pass to classify.
2. **Compute dominance / dominance-frontier.** Standard SSA
   construction prerequisite.
3. **Insert phis.** For each promotable alloca, place a phi in
   every block on the dominance frontier of any block that stores
   to it.
4. **Rename.** Per-block walk in dominator order: maintain a
   per-alloca renaming stack; replace each `load` with the current
   stack top; each `store` pushes a new SSA value; phis push their
   result Value.
5. **Cleanup.** Drop the original `alloca` / `load` / `store`
   instructions.

Aliasing analysis is intentionally trivial — only address-not-taken
allocas are touched. Anything more sophisticated (escape analysis,
mem-SSA) is out of scope for the rewrite.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.builder`](../ir/builder.md),
[`mach.lang.me.ir.instruction`](../ir/instruction.md),
[`mach.lang.me.ir.value`](../ir/value.md).
