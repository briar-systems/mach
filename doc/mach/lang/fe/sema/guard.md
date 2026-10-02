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

## fun check_stable

```mach
pub fun check_stable(sc: *sema_context.SemaContext, place: ast_id.ExprId, span: token.Span) bool;
```

the spelling rule `place_equal` implements, enforced where a guard place is written so an
unmatchable place is rejected at the `sel` rather than at every read inside the region

## fun span_text_equal

```mach
pub fun span_text_equal(sc: *sema_context.SemaContext, a: token.Span, b: token.Span) bool;
```

