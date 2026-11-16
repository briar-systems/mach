[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **guard**

# mach.front.sema.guard

tag payload guards: the lexical regions in which a payload place may be used

a tag payload place `P.c` is readable, writable and addressable only inside a guard for that
place and case. a guard opens on `if (sel P.c)`, after a chain of exiting `sel` tests on one
place that leaves one case untested, and in the final `or` of such a chain. guards are frames on
the caller's stack, linked through the sema context and closed when their region ends, so no
flow fact is ever kept. a place is matched by its spelling, so one that spelling cannot match is
refused at the `sel`, and inside a guard no assignment may replace the guarded place whole or
an object it is reached through.

Source: [`src/front/sema/guard.mach`](../../../../src/front/sema/guard.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`push`](#push), [`end`](#end) | |
| [`sel_begin`](#sel_begin) | opens a guard for `P.c` when the condition is exactly `sel P.c` |
| [`guarded`](#guarded) | |
| [`exit_chain_begin`](#exit_chain_begin) | a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing which cases remain; when exactly one remains the rest of the block is guarded for it |
| [`final_or_begin`](#final_or_begin) | a chain whose arms all test one place with `sel` or `!sel` reaches its final bare `or` knowing which cases remain, exiting or not; when exactly one remains the `or` body is guarded for it, as the rest of the block is after a chain that exits |
| [`check_assign`](#check_assign) | whole-value assignment to a guarded place, or to an object it is reached through, replaces the case |
| [`check_stable`](#check_stable) | the spelling rule `place_equal` implements, enforced where a guard place is written so an unmatchable place is rejected at the `sel` rather than at every read inside the region |
| [`span_text_equal`](#span_text_equal) | |

---

## Functions

### push

```mach
pub fun push(sc: *mach.front.sema.context.SemaContext, frame: *mach.front.sema.context.Guard, place: mach.front.ast.id.ExprId, case_name: mach.base.intern.StrId)
```

### end

```mach
pub fun end(sc: *mach.front.sema.context.SemaContext, frame: *mach.front.sema.context.Guard)
```

### sel_begin

```mach
pub fun sel_begin(sc: *mach.front.sema.context.SemaContext, cond: mach.front.ast.id.ExprId, frame: *mach.front.sema.context.Guard) res[bool, mach.base.fail.Fail]
```

opens a guard for `P.c` when the condition is exactly `sel P.c`

### guarded

```mach
pub fun guarded(sc: *mach.front.sema.context.SemaContext, place: mach.front.ast.id.ExprId, case_name: mach.base.intern.StrId) res[bool, mach.base.fail.Fail]
```

### exit_chain_begin

```mach
pub fun exit_chain_begin(sc: *mach.front.sema.context.SemaContext, sid: mach.front.ast.id.StmtId, frame: *mach.front.sema.context.Guard) res[bool, mach.base.fail.Fail]
```

a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing
which cases remain; when exactly one remains the rest of the block is guarded for it

### final_or_begin

```mach
pub fun final_or_begin(sc: *mach.front.sema.context.SemaContext, sid: mach.front.ast.id.StmtId, frame: *mach.front.sema.context.Guard) res[bool, mach.base.fail.Fail]
```

a chain whose arms all test one place with `sel` or `!sel` reaches its final bare `or` knowing
which cases remain, exiting or not; when exactly one remains the `or` body is guarded for it,
as the rest of the block is after a chain that exits

### check_assign

```mach
pub fun check_assign(sc: *mach.front.sema.context.SemaContext, lhs: mach.front.ast.id.ExprId, span: mach.base.source.Span)
```

whole-value assignment to a guarded place, or to an object it is reached through, replaces the case

### check_stable

```mach
pub fun check_stable(sc: *mach.front.sema.context.SemaContext, place: mach.front.ast.id.ExprId, span: mach.base.source.Span) bool
```

the spelling rule `place_equal` implements, enforced where a guard place is written so an
unmatchable place is rejected at the `sel` rather than at every read inside the region

### span_text_equal

```mach
pub fun span_text_equal(sc: *mach.front.sema.context.SemaContext, a: mach.base.source.Span, b: mach.base.source.Span) bool
```

