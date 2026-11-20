[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **guard**

# mach.front.sema.guard

tag payload guards: the lexical regions in which a payload place may be used

a tag payload place `P.c` is readable, writable and addressable only inside a guard for that
place and case. a guard opens on `if (sel P.c)`, after a chain of exiting `sel` tests on one
place that leaves one case untested, and in the final `or` of such a chain. guards are frames on
the caller's stack, linked through the sema context and closed when their region ends. a place is
matched by its spelling, so one that spelling cannot match is refused at the `sel`, and inside a
guard no assignment may replace the guarded place whole or an object it is reached through.

the one flow fact kept is a guard's end: an assignment to anything else the spelling names, a
subscript or a pointer the place is reached through, moves the spelling onto storage nothing
tested, so the frame is marked ended and every later lookup in its region misses it. later is
the text after the assignment, the whole of a loop that holds one, since a later iteration runs
after it, and a `fin` body when the rest of its block holds one, since the fin replays after it.

Source: [`src/front/sema/guard.mach`](../../../../src/front/sema/guard.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`push`](#push) | opens `frame` as a live guard for `place` and `case_name`, innermost on the context's stack |
| [`end`](#end) | closes `frame`, which must be the innermost guard |
| [`sel_begin`](#sel_begin) | opens a guard for `P.c` when the condition is exactly `sel P.c` |
| [`guarded`](#guarded) | whether an open guard holds `place` for `case_name`, or only one that has ended does |
| [`loop_begin`](#loop_begin) | a loop runs its condition and body again after any assignment in them, so a guard open at the loop that an assignment inside it moves is ended for the whole loop |
| [`fin_begin`](#fin_begin) | a `fin` body replays when its block ends, after the rest of the block, so inside the body a guard that an assignment in that rest moves does not hold |
| [`fin_end`](#fin_end) | lets every guard the `fin` body `fin` hid hold again, once that body has been typed |
| [`exit_chain_begin`](#exit_chain_begin) | a chain whose arms all test one place with `sel` or `!sel` and all exit leaves the block knowing which cases remain; when exactly one remains the rest of the block is guarded for it |
| [`final_or_begin`](#final_or_begin) | a chain whose arms all test one place with `sel` or `!sel` reaches its final bare `or` knowing which cases remain, exiting or not; when exactly one remains the `or` body is guarded for it, as the rest of the block is after a chain that exits |
| [`check_assign`](#check_assign) | whole-value assignment to a guarded place, or to an object it is reached through, replaces the case and is refused; an assignment to anything else the place's spelling names moves it and ends the guard |
| [`check_stable`](#check_stable) | the spelling rule `place_equal` implements, enforced where a guard place is written so an unmatchable place is rejected at the `sel` rather than at every read inside the region |
| [`span_text_equal`](#span_text_equal) | whether two spans of the module's source hold the same text |

---

## Functions

### push

```mach
pub fun push(sc: *mach.front.sema.context.SemaContext, frame: *mach.front.sema.context.Guard, place: mach.front.ast.id.ExprId, case_name: mach.base.intern.StrId)
```

opens `frame` as a live guard for `place` and `case_name`, innermost on the context's stack

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `frame` | the caller's frame, which must outlive the guard's region |
| `place` | the guarded place expression |
| `case_name` | the case the place is guarded for |

### end

```mach
pub fun end(sc: *mach.front.sema.context.SemaContext, frame: *mach.front.sema.context.Guard)
```

closes `frame`, which must be the innermost guard

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `frame` | the guard to close |

### sel_begin

```mach
pub fun sel_begin(sc: *mach.front.sema.context.SemaContext, cond: mach.front.ast.id.ExprId, frame: *mach.front.sema.context.Guard) res[bool, mach.base.fail.Fail]
```

opens a guard for `P.c` when the condition is exactly `sel P.c`

### guarded

```mach
pub fun guarded(sc: *mach.front.sema.context.SemaContext, place: mach.front.ast.id.ExprId, case_name: mach.base.intern.StrId) res[mach.front.sema.context.Hold, mach.base.fail.Fail]
```

whether an open guard holds `place` for `case_name`, or only one that has ended does

### loop_begin

```mach
pub fun loop_begin(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span) err[mach.base.fail.Fail]
```

a loop runs its condition and body again after any assignment in them, so a guard open at the
loop that an assignment inside it moves is ended for the whole loop

### fin_begin

```mach
pub fun fin_begin(sc: *mach.front.sema.context.SemaContext, fin: mach.front.ast.id.StmtId, span: mach.base.source.Span) err[mach.base.fail.Fail]
```

a `fin` body replays when its block ends, after the rest of the block, so inside the body a guard
that an assignment in that rest moves does not hold

### fin_end

```mach
pub fun fin_end(sc: *mach.front.sema.context.SemaContext, fin: mach.front.ast.id.StmtId)
```

lets every guard the `fin` body `fin` hid hold again, once that body has been typed

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `fin` | the `fin` statement whose body ended |

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
and is refused; an assignment to anything else the place's spelling names moves it and ends the
guard. a guard a `fin` body does not see is ended by either, since the fin replays inside its region

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

whether two spans of the module's source hold the same text

| Parameter | |
| :--- | :--- |
| `sc` | the sema context whose source is read |
| `a` | the first span |
| `b` | the second span |
| **returns** | true when both spans have the same length and bytes |

