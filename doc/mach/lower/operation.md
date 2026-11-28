[mach](../README.md) / [lower](README.md) / **operation**

# mach.lower.operation

an `#[intrinsic("mirl", "ir", name)]` declaration held to its operation's row

the declaration states the row whole: one parameter per fixed operand, in the row's order,
and the row's result as its return type. mirl's checker types the row over the parameters'
types, so what the operation takes and gives is the row's alone, never a list here. each
break is refused where the function is declared, the count with `op.operand_count` and any
other with `op.signature`, whether or not anything calls it

Source: [`src/lower/operation.mach`](../../../src/lower/operation.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.lower.context`](context.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`declared`](#declared) | the declaration of the operation `code`, of function type `fn`, held to the operation's row at `span` |

---

## Functions

### declared

```mach
pub fun declared(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, code: Opcode, fn: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

the declaration of the operation `code`, of function type `fn`, held to the operation's row
at `span`

