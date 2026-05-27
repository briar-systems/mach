# mach.lang.me.lower.stmt

Per-stmt lowering. Drives control flow emission: every branching or
looping construct creates the blocks it needs and stitches the
predecessor edges.

Source is `new/lang/me/lower/stmt.mach` (currently empty).

## Functions

### `lower_stmt`

```mach
pub fun lower_stmt(ctx: *lower.LowerContext, sid: id.StmtId) Result[bool, str]
```

Dispatches by [`StmtKind`](../../fe/ast/stmt.md#stmtkind). After a
terminator-producing stmt (`ret`, an exhaustive `if`/`or` chain,
infinite `for`), `ctx.builder.block` is left set to a fresh
unreachable block — the caller's next stmt will land there and
[`dce`](../pass/dce.md) will drop it.

| `STMT_KIND_*`   | Action                                                                  |
|-----------------|-------------------------------------------------------------------------|
| `EXPR`          | Lower the expression, discard the result (kept for side effects). Assignment is an `EXPR_KIND_BINARY` / `BIN_ASSIGN` expression and lowers here. |
| `BLOCK`         | New scope for [`LowerContext.locals`](../lower.md#lowercontext); recurse over the contained stmts. |
| `IF`            | Diamond of `cbr`-headed blocks. The parser folds the `or` chain into [`StmtIf.else_block`](../../fe/ast/stmt.md#stmtif), so there is no separate `or` case — a chained `if` is just an `IF` in the else block. |
| `FOR`           | Header + body + exit blocks; back-edge from the body's last block to the header. An absent condition (`EXPR_NIL`) is an infinite loop. |
| `RET`           | Lower the return value (if any), emit `ret`.                            |
| `BRK`           | Branch to the enclosing loop's exit block.                              |
| `CNT`           | Branch to the enclosing loop's header block.                            |
| `FIN`           | Record the deferred body on the function's `fin` stack; emit it before every reachable terminator. |
| `DECL`          | Local `val` / `var`: `alloca` + initial `store` if there's an init; bind the [`SymbolId`](../../fe/resolve.md#symbolid) to the alloca pointer in [`LowerContext.locals`](../lower.md#lowercontext). |
| `ASM`           | Emit an `OP_ASM` instruction whose operands are the `IN` / `OUT` Values, and register an [`IrAsm`](../ir.md#irasm) payload in [`Function.asm`](../ir.md#function) — interning the asm body text and ISA so the IR carries no source spans. |
| `COMPTIME_IF`   | Lower only the arm marked active by comptime evaluation; the inactive arms contribute nothing. |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.fe.ast`](../../fe/ast.md),
[`mach.lang.fe.ast.id`](../../fe/ast/id.md),
[`mach.lang.fe.ast.stmt`](../../fe/ast/stmt.md),
[`mach.lang.me.lower`](../lower.md),
[`mach.lang.me.lower.expr`](expr.md),
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.builder`](../ir/builder.md),
[`mach.lang.me.ir.value`](../ir/value.md).
