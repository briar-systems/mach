[mach](../README.md) / [lower](README.md) / **stmt**

# mach.lower.stmt

the lowering of statements, with `asm` blocks and the comptime `$if` and `$each` among them

an `asm` block reaches mirl as inline assembly whose `{name}` substitutions are memory operands
bound to the slots of the locals they name. a `$each` is lowered once per element of the array
value sema recorded for it.

Source: [`src/lower/stmt.mach`](../../../src/lower/stmt.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.front.ast.id`](../front/ast/id.md), [`mach.lower.context`](context.md).

| Functions | |
| :--- | :--- |
| [`lower_stmt`](#lower_stmt) | lower the statement `sid` into the function being built, at its own source location |

---

## Functions

### lower_stmt

```mach
pub fun lower_stmt(ctx: *mach.lower.context.LowerContext, sid: mach.front.ast.id.StmtId) err[mach.base.fail.Fail]
```

lower the statement `sid` into the function being built, at its own source location

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context, positioned in the block the statement continues |
| `sid` | the statement |
| **returns** | ok, or the failure located at the statement, a statement nested past the stack's reach refused |

