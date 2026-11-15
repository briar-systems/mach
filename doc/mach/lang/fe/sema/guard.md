# mach.lang.fe.sema.guard

## fun push

```mach
pub fun push(sc: *sema_context.SemaContext, frame: *sema_context.Guard, place: ast_id.ExprId, case_name: intern.StrId);
```

## fun end

```mach
pub fun end(sc: *sema_context.SemaContext, frame: *sema_context.Guard);
```

## fun sel_begin

```mach
pub fun sel_begin(sc: *sema_context.SemaContext, cond: ast_id.ExprId, frame: *sema_context.Guard) res[bool, fail.Fail];
```

opens a guard for `P.c` when the condition is exactly `sel P.c`

## fun guarded

```mach
pub fun guarded(sc: *sema_context.SemaContext, place: ast_id.ExprId, case_name: intern.StrId) res[bool, fail.Fail];
```

## fun exit_chain_begin

```mach
pub fun exit_chain_begin(sc: *sema_context.SemaContext, sid: ast_id.StmtId, frame: *sema_context.Guard) res[bool, fail.Fail];
```

a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing
which cases remain; when exactly one remains the rest of the block is guarded for it

## fun final_or_begin

```mach
pub fun final_or_begin(sc: *sema_context.SemaContext, sid: ast_id.StmtId, frame: *sema_context.Guard) res[bool, fail.Fail];
```

a chain whose arms all test one place with `sel` or `!sel` reaches its final bare `or` knowing
which cases remain, exiting or not; when exactly one remains the `or` body is guarded for it,
as the rest of the block is after a chain that exits

## fun check_assign

```mach
pub fun check_assign(sc: *sema_context.SemaContext, lhs: ast_id.ExprId, span: lang_source.Span);
```

whole-value assignment to a guarded place, or to an object it is reached through, replaces the case

## fun check_stable

```mach
pub fun check_stable(sc: *sema_context.SemaContext, place: ast_id.ExprId, span: lang_source.Span) bool;
```

the spelling rule `place_equal` implements, enforced where a guard place is written so an
unmatchable place is rejected at the `sel` rather than at every read inside the region

## fun span_text_equal

```mach
pub fun span_text_equal(sc: *sema_context.SemaContext, a: lang_source.Span, b: lang_source.Span) bool;
```

