# The test status range

A test reports its result as the exit status of the process that ran it, and
a process exit status is eight bits. This page records why the test protocol
was closed over the range `0..255` at both ends instead of growing a wider
result channel, and what that decision cost.

## The defect it closes

A test that returned `256` was reported as passing on linux and darwin and as
failing on windows (#3241). The dispatcher returned the test's `i32` result
from `main` unchanged, the POSIX kernel kept the low eight bits, and `mach
test` classified an exit status of `0` as a pass. Windows preserves the full
32-bit code, so the same test failed there. A RISC-V constant-time regression
sat behind a bit mask that had grown to nine bits and was invisible on POSIX
for the life of its branch; the tree carried three more nine-plus-bit masks
and one `ret 1000 + op` that would have hidden the same way.

The rule from the windows migration, judge success from the full preserved
status, holds in the other direction too: POSIX must not fabricate success
from a truncated one.

## The alternatives

**A result channel other than the exit code.** The dispatcher would write the
`i32` to a pipe, a status line on the captured output, or a result file, and
the runner would read it back. The runner has no pipe to the child: it
redirects the child's stdout and stderr into a capture file and reads the
exit status, nothing else. Any of those channels needs the dispatcher to
perform I/O, and the dispatcher is synthesized as target-agnostic IR with no
runtime behind it. Writing bytes from it means a per-OS `write` in the
compiler (a linux syscall, a darwin libSystem import, a kernel32 import), a
second copy of what `std.runtime` exists to own. Calling into `std` by
linkage name from the synthesized module was rejected too: a std module is
compiled only when something uses it, so the symbol is not guaranteed to be
linked, and it ties the compiler's test protocol to a std release. The
channel was not built.

**Keep the exit code and reserve nothing.** Fold a wide result to its low
eight bits unless that would read as a pass. That preserves more digits in
some cases (`257` would arrive as `1`) at the cost of a rule nobody can
apply while reading a failure line. Rejected for the simpler rule below.

**Trap on a wide result.** A `ud2`/`brk` in the dispatcher turns `256` into a
signal. Loud, but it reports as a crash on every host and as an exception
code on windows, and says nothing about the value. Rejected.

## The rule

The status range `0..255` is part of the protocol, and it is enforced where
each kind of value can be seen:

- **At the `ret`, at compile time.** A `ret` in a test body whose value is
  literal-shaped (a literal, a negated literal, or arithmetic over literals,
  the same shapes literal coercion already folds) outside `0..255` is refused
  with a located diagnostic naming the value. The check reuses the literal
  range probe against the `u8` range without retyping the expression, so an
  ordinary `i32` function returning `256` is untouched. A suffixed literal
  (`256i32`) is typed, not literal-shaped, and is left to the run-time fold.
- **At the dispatcher, at run time.** The synthesized entry compares the
  result against `255` unsigned, which puts every negative value past the
  range in one instruction, and exits `255` for anything outside it. The
  fold is one shared block; the compare is one per test. The value is lost,
  the failure is not, and the same status is reported on every host.
- **In the readout.** `(exit 255)` is the top of the range and the fold
  target both; `code` in the JSON stream is documented as `1..255`.

The dispatcher's own statuses are unchanged: `2` for a missing, malformed, or
out-of-range index.

## What it cost

Nine-plus-bit masks in the tree were rewritten to return the ordinal of the
first failing check, which keeps every check distinguishable and loses the
"all failures at once" reading of a mask. A test that wants that reading
has eight bits to spend. The compile-time check cannot see a mask that grows
past eight bits at run time; that case reaches the fold and reads as `255`,
which is the documented signal to look for a wide result.

The guards are covered by an IR-level test that reads the synthesized module
(every branch on a `255 <u result` compare takes its true edge to a block
whose only instruction is `ret 255`), a driver test that runs the built
dispatcher on results of `256`, `300`, `-1`, `3`, `0` and `255`, and a driver
test that the three literal shapes are refused at the `ret` with the value in
the message. Each guard was mutated once: unwiring the fold's true edge fails
the first two, disabling the sema check fails the third.
