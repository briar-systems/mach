# mach.lang.fe.sema.guard

## fun push

```mach
pub fun push(sc: *sema_context.SemaContext, frame: *sema_context.Guard, place: ast_id.ExprId, case_name: intern.StrId);
```

## fun close

```mach
pub fun close(sc: *sema_context.SemaContext, frame: *sema_context.Guard);
```

## fun open_sel

```mach
pub fun open_sel(sc: *sema_context.SemaContext, cond: ast_id.ExprId, frame: *sema_context.Guard) bool;
```

opens a guard for `P.c` when the condition is exactly `sel P.c`

## fun guarded

```mach
pub fun guarded(sc: *sema_context.SemaContext, place: ast_id.ExprId, case_name: intern.StrId) bool;
```

## fun open_exit_chain

```mach
pub fun open_exit_chain(sc: *sema_context.SemaContext, sid: ast_id.StmtId, frame: *sema_context.Guard) bool;
```

a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing
which cases remain; when exactly one remains the rest of the block is guarded for it

## fun check_assign

```mach
pub fun check_assign(sc: *sema_context.SemaContext, lhs: ast_id.ExprId, span: token.Span);
```

whole-value assignment to a guarded place, or to an object it is reached through, replaces the case

## fun sel_case_name

```mach
pub fun sel_case_name(sc: *sema_context.SemaContext, s: *ast_expr.ExprSel) intern.StrId;
```

the case a `sel` names, spelled directly or through a comptime case descriptor; STR_NIL when the
descriptor does not evaluate, which type checking reports at the `sel` itself

## fun stmt_exits

```mach
pub fun stmt_exits(sc: *sema_context.SemaContext, sid: ast_id.StmtId, loops: u32) bool;
```

every reachable path through the statement leaves by `ret`, or by `brk`/`cnt` targeting a loop
outside it; `loops` counts the loops entered since the statement of interest

## fun check_stable

```mach
pub fun check_stable(sc: *sema_context.SemaContext, place: ast_id.ExprId, span: token.Span) bool;
```

the spelling rule `place_equal` implements, enforced where a guard place is written so an
unmatchable place is rejected at the `sel` rather than at every read inside the region

## fun place_equal

```mach
pub fun place_equal(sc: *sema_context.SemaContext, a: ast_id.ExprId, b: ast_id.ExprId) bool;
```

two place expressions name the same storage when they are spelled the same way over the same
bindings; this is lexical identity, exactly what a guard region is

## fun span_text_equal

```mach
pub fun span_text_equal(sc: *sema_context.SemaContext, a: token.Span, b: token.Span) bool;
```

