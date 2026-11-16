[mach](../README.md) / [lower](README.md) / **stmt**

# mach.lower.stmt

the lowering of statements, with `asm` blocks and the comptime `$if` and `$each` among them

an `asm` block reaches mirl as inline assembly whose `{name}` substitutions are memory operands
bound to the slots of the locals they name. a `$each` is lowered once per element of the array
value sema recorded for it.

<sub>source: [`src/lower/stmt.mach`](../../../src/lower/stmt.mach) · imported by 1 module</sub>

<sub>`lower_context` is [`mach.lower.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../front/ast/id.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`lower_stmt`](#lower_stmt) | |

---

## Functions

### lower_stmt

```mach
pub fun lower_stmt(ctx: *lower_context.LowerContext, sid: ast_id.StmtId) err[fail.Fail]
```

