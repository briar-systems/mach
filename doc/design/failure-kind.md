# The closed failure kind

A function that can fail returns a `Result` whose error is a value the caller
can act on without reading its text. This page records why the compiler's
failure vocabulary was closed, what the kinds are, and what was removed to get
there.

## The defect it closes

The compiler had settled into `Result[bool, str]` as its general outcome
type, and the shape meant five things at once: a real binary answer (did this
pass change anything, was this key found), a validation verdict, a
registration outcome, a diagnostic append, and plain completion. Callers that
needed to know *which* failure they had were left with the message, and 171
production sites branched on error text with `str_contains`. A message
reworded for a user broke a caller that matched it; a caller that did not
match got a fall-through it never meant. Worse, several sites carried a
"no-error" sentinel — an empty string, `ok("")` — that meant absence, and a
discarded append result turned a compiler rejection into a success twice in
one day in unrelated subsystems.

The triage was ruled to be the full tree, not only the branching sites:
4,625 occurrences across 1,172 declarations in 140 files, taken one
subsystem per round (front end, middle end, code generation, targets,
driver and build, CLI), each round byte-identical on the compiler's own
output and self-host fixpoint-clean.

## The rule

Per function, one of three things is true, and the type says which:

- **the bool is a real binary outcome**, in which case it stays and is
  named for what it is (`changed`, `found`, `committed`); a census with a
  tracked allowlist holds the set of real bools;
- **the bool was always true**, in which case the success type is
  `Void` (`std.types.result.Void`, with `ok_void` and `void_of` from the
  standard library) — the compiler already had a `Unit` meaning a code
  generation unit, so the unit-shaped success type needed another name, and
  it was one small standard-library release;
- **a caller branched on the message**, in which case the error carries a
  closed kind and the message becomes presentation only.

No production site branches on error text any more; a census is the guard.
Absence is an `Option`, never a sentinel string.

## The kinds

The compiler has two closed failure types, one per layer.

`mach.lang.fail.Fail` is the language layer's: kind `REPORTED` (the failure
was already recorded as a diagnostic on the session, and there is nothing
further to say) or `MESSAGE` (a failure with text that has not been reported
anywhere). It is carried through the query engine, so a query that fails
because a dependency's phase already reported does not report again, and the
build's fail path reads the kind rather than a string.

`mach.lang.build.outcome.Fail` is the driver's: `FAIL_REPORTED`,
`FAIL_USER` (the input was wrong: a manifest error, a missing project),
`FAIL_INTERNAL` (an invariant the compiler owns was broken), and
`FAIL_ENVIRONMENT` (the machine, not the input: a tool that could not be
spawned, a resource that was not there). `from_fail` lifts the language
layer's kind into it. The distinction is what the exit code is derived from:
`1` for a user error, `2` for an internal error, `3` where a command
distinguishes an environmental one, and a compiler failure never renders as
a test failure.

Smaller closed enumerations sit where one subsystem needed its own: the
compile-time evaluator's `EvalFailKind` replaced three branches on message
text, and the parallel lower and codegen worker channel carries `fail.Fail`
rather than a string.

## What this replaced in the standard library seam

Three producers handed to `std.filesystem.transaction` used to return a bool
whose `false` meant "abort"; the abort arm was the same information the
error arm carried. Standard library 0.37.0 made the transaction's producer
callback return `Void`, so `err` is the only abort, and the five compiler
producers were converted with the pin bump. The one contract bool that
stayed is the validator callback's, which is a real verdict.

## What was deliberately not done

The kinds are closed and small on purpose. A per-site choice among "reject",
"default", and "unsupported" was the disease; a large open-ended error
taxonomy would be the same disease with more names. An unknown tag, opcode,
callback, or capability fails closed under one policy, a resource or
allocation failure is never mapped to a valid-looking default (public,
false, zero, empty, a continuable error type), and a diagnostic append
result is never discarded. Those are the invariants the closed kind exists
to make checkable.
