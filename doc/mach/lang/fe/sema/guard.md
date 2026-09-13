# mach.lang.fe.sema.guard

## fun push

```mach
pub fun push(sc: *context.SemaContext, frame: *context.Guard, place: id.ExprId, case_name: intern.StrId);
```

## fun close

```mach
pub fun close(sc: *context.SemaContext, frame: *context.Guard);
```

## fun open_sel

```mach
pub fun open_sel(sc: *context.SemaContext, cond: id.ExprId, frame: *context.Guard) bool;
```

opens a guard for `P.c` when the condition is exactly `sel P.c`

## fun guarded

```mach
pub fun guarded(sc: *context.SemaContext, place: id.ExprId, case_name: intern.StrId) bool;
```

## fun open_exit_chain

```mach
pub fun open_exit_chain(sc: *context.SemaContext, sid: id.StmtId, frame: *context.Guard) bool;
```

a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing
which cases remain; when exactly one remains the rest of the block is guarded for it

## fun check_assign

```mach
pub fun check_assign(sc: *context.SemaContext, lhs: id.ExprId, span: token.Span);
```

whole-value assignment to a guarded place, or to an object it is reached through, replaces the case

## fun sel_case_name

```mach
pub fun sel_case_name(sc: *context.SemaContext, s: *expr.ExprSel) intern.StrId;
```

the case a `sel` names, spelled directly or through a comptime case descriptor; STR_NIL when the
descriptor does not evaluate, which type checking reports at the `sel` itself

## fun stmt_exits

```mach
pub fun stmt_exits(sc: *context.SemaContext, sid: id.StmtId, loops: u32) bool;
```

every reachable path through the statement leaves by `ret`, or by `brk`/`cnt` targeting a loop
outside it; `loops` counts the loops entered since the statement of interest

## fun check_stable

```mach
pub fun check_stable(sc: *context.SemaContext, place: id.ExprId, span: token.Span) bool;
```

the spelling rule `place_equal` implements, enforced where a guard place is written so an
unmatchable place is rejected at the `sel` rather than at every read inside the region

## fun place_equal

```mach
pub fun place_equal(sc: *context.SemaContext, a: id.ExprId, b: id.ExprId) bool;
```

two place expressions name the same storage when they are spelled the same way over the same
bindings; this is lexical identity, exactly what a guard region is

