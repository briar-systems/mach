# mach.lang.me.lower

Typed-AST-to-IR lowering. Takes a sema'd module and produces an
[`ir.Module`](ir.md#module) ready for the optimisation pipeline and
the backend. The dispatch is per-decl
([`lower.decl`](lower/decl.md)), per-stmt
([`lower.stmt`](lower/stmt.md)), and per-expr
([`lower.expr`](lower/expr.md)); this module owns the orchestration
and the per-function context.

Source is `new/lang/me/lower.mach` (currently empty).

## Types

### `LowerContext`

```mach
pub rec LowerContext {
    s:           *sess.Session;
    tgt:         *target.Target;
    a:           *ast.Ast;
    sema_result: *sema.SemaResult;
    rr:          *resolve.ResolveResult;
    ctx:         *comptime.ComptimeCtx;
    m:           *ir.Module;
    builder:     builder.Builder;
    locals:      map.Map[resolve.SymbolId, value.Value];
}
```

Per-module lowering state. The [`builder`](ir/builder.md) tracks the
insertion cursor; `locals` maps source-level
[`SymbolId`](../fe/resolve.md#symbolid)s to the IR
[`Value`](ir/value.md#value)s that hold their current SSA name (or
the alloca slot pointer for address-taken locals).

## Functions

### `lower_module`

```mach
pub fun lower_module(
    s:           *sess.Session,
    tgt:         *target.Target,
    a:           *ast.Ast,
    sema_result: *sema.SemaResult,
    rr:          *resolve.ResolveResult,
    ctx:         *comptime.ComptimeCtx,
) Result[ir.Module, str]
```

The lowering stage of [`Q_LOWER`](../query.md#integration). Builds a
[`LowerContext`](#lowercontext), iterates the AST's top-level decls,
and dispatches each through [`lower.decl`](lower/decl.md), producing
an unoptimised [`ir.Module`](ir.md#module). The `Q_LOWER` compute
body then runs the [`pipeline`](pipeline.md) on that module in
place — the optimised module is what the query caches.

| Param       | Type                                                  | Description                                          |
|-------------|-------------------------------------------------------|------------------------------------------------------|
| s           | [`*sess.Session`](../session.md#session)              | Shared session.                                      |
| tgt         | [`*target.Target`](../target.md#target)               | Selected target — drives type sizing and ABI lowering for calls. |
| a           | [`*ast.Ast`](../fe/ast.md#ast)                        | Parsed AST.                                          |
| sema_result | [`*sema.SemaResult`](../fe/sema.md)                   | Typed AST output. Every expression / pattern has a final type. |
| rr          | [`*resolve.ResolveResult`](../fe/resolve.md#resolveresult) | Per-module name resolution.                     |
| ctx         | [`*comptime.ComptimeCtx`](../fe/comptime.md#comptimectx) | Comptime context — supplies the folded `CTValue`s for comptime-evaluable global initialisers. |

Returns the lowered module or an error (allocation, malformed input —
verifier-grade invariants are checked here too).

## Lowering rules

| Source construct             | IR form                                                                  |
|------------------------------|--------------------------------------------------------------------------|
| `fun` decl                   | One [`Function`](ir.md#function); params become `VAL_PARAM`; body becomes blocks. |
| `val` / `var` (local)        | `alloca` on first use; subsequent reads / writes become `load` / `store` until [`mem2reg`](pass/mem2reg.md) collapses them. |
| `val` / `var` (module-level) | One [`Global`](ir.md#global); comptime-evaluable inits become constants. |
| `if` / `or`                  | Diamond of blocks: a `cbr` head, one block per arm, and a merge block. Lowering emits no `phi`s — [`mem2reg`](pass/mem2reg.md) inserts them for variables mutated across arms. |
| `for`                        | Header block with `cbr` to body / exit; body's last block branches back to header. An absent condition is an infinite loop (`br` straight to the body). |
| `ret`                        | `ret` terminator; current block closes; further stmts in the source produce unreachable code that [`dce`](pass/dce.md) removes. |
| `fin`                        | Defers the body's lowered block(s); inserted before every reachable terminator in the enclosing function (drop semantics). |
| `asm` block                  | `OP_ASM` instruction; operands carried verbatim from [`parser/asm.md`](../fe/parser/asm.md). |
| `::` cast                    | One of `TRUNC` / `SEXT` / `ZEXT` / `FP_TRUNC` / `FP_EXT` / `BITCAST` based on the source / target type pair. Coerce semantics handled upstream in [`sema.coerce`](../fe/sema/coerce.md). |
| `call`                       | `call` with ABI-classified args; the actual reg / stack assignment happens during isel against the target's [`AbiVTable`](../target/abi.md#abivtable). |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
`std.allocator`, `std.collections.map`,
[`mach.lang.session`](../session.md),
[`mach.lang.target`](../target.md),
[`mach.lang.fe.ast`](../fe/ast.md),
[`mach.lang.fe.resolve`](../fe/resolve.md),
[`mach.lang.fe.sema`](../fe/sema.md),
[`mach.lang.fe.comptime`](../fe/comptime.md),
[`mach.lang.me.ir`](ir.md),
[`mach.lang.me.ir.builder`](ir/builder.md),
[`mach.lang.me.ir.value`](ir/value.md),
[`mach.lang.me.lower.decl`](lower/decl.md),
[`mach.lang.me.lower.stmt`](lower/stmt.md),
[`mach.lang.me.lower.expr`](lower/expr.md).
