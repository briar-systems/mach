# What a behavioural golden cannot observe

A behavioural test builds a program, runs it, and compares its output against a
recorded answer. It is the right first reach and it is what this repo has reached
for by default. This document records where that is **structurally** blind, which
surface can see each of those places instead, and - the question that decides
where a case belongs - which of them genuinely need an integration test at all.

**This is not documentation of any harness.** Nothing here describes a file
layout, a case format, a producer, or how to add a case, because all of that
expires. What does not expire is the relationship between a defect class and the
observation that can see it: that is a property of the defects and of the
surfaces. Two suites act on it today - `test/` executes cases against a C anchor
and diffs golden disassembly, and `test/link/` reads emitted images and links
foreign objects - and this document is why each of them exists, not a manual for
either.

Every entry below was paid for once already. The cost was never writing the fix.
It was the time between the defect existing and anything being able to see it.

## The one rule everything follows from

A behavioural golden is a statement about **one string**. It is not a statement
about the program, the image, or the compiler.

So the question a case must answer is never "does this exercise the feature". It
is "if the defect were present, would this string differ". If the answer is no,
the case is maintenance with no return, however green it looks and however much
of the feature it runs.

## Class to surface

| Defect class | Why running the program cannot see it | Surface that can |
|---|---|---|
| Loader or image layout the running program never consults | the loader resolves the ambiguity and the program is correct | a **property** asserted over the emitted headers, or driving a real consumer of them |
| A self-consistent format error | the producer and the consumer are both ours and share the mistake, so they agree | an artifact from a **foreign** toolchain, or an external oracle that does not share our opinion |
| A wrong value that is still a value | the program prints a plausible number and exits 0 | an independently known answer - hand-computed, or from a foreign toolchain. never another target's output |
| A failure the success path swallows | the wrapper's own error check never fires, so failure returns as success | assert the **effect**, not the return |
| Anything on a leg that never executed | there is no observation at all, and no observation reports green | real execution on the real platform, plus an explicit two-way known-failures list until there is one |
| Emitted shape where the answer is unchanged | a correct answer computed the wrong way is still correct | an assertion over what was emitted - instruction forms, counts, pool entries |
| A type error the backend tolerates | the backend selects from operand types, so the mistyping never reaches the output | the IR verifier |
| A diagnostic that does not fail its pass | "clean" and "recorded an error, returned ok" render identically | read the diagnostic sink, not the pass's return value |
| A performance regression of any size | the answer is unchanged by construction | a measurement harness. not this one, and it should not grow a timing observable |

## Which of these actually needed an integration test

This is the sharp question, and the honest answer is: **most of them did not.**

| Defect | Needed integration? | What would have caught it |
|---|---|---|
| #2795, two `PT_LOAD` segments at one address | **no** | a property assertion over the writer's emitted program headers. this is literally how it was fixed - the guard lives in `elf.mach`'s own tests |
| #2766, a float access substituting another width's form | **no** | byte comparison against `llvm-mc` in a unit test, which is already this repo's practice for encoder work |
| #2813, `e_flags = 0` claiming a soft-float ABI | **no** | an assertion over the emitted header field |
| the `pthread` / errno wrapper | **no** | assert the wrapper reports failure when the underlying call returns a nonzero error. the pthread contract is knowable without a mac |
| #2797, a pc-relative pair resolved without chasing its label | **discovery yes, regression no** | a hand-built object carrying the psABI spelling with a nonzero symbol offset and a nonzero high-half addend. constructible locally once you know the spelling exists |
| `narrow-stack-args` / #2598, Apple arm64 stack-argument size | **yes** | nothing local. the fact is about Apple's ABI, and only Apple's toolchain and Apple's hardware state it |
| the darwin eleven | **yes** | nothing local. what a current kernel does is not derivable from our source |

Four of seven were fully local. One split. Two genuinely required something
outside the repo.

**The lesson is not the one the framing suggests.** These were mostly not missed
because a behavioural golden is blind. They were missed because **nothing looked
at the emitted artifact at all** - and that is a unit-test-shaped gap, not an
integration-shaped one. #2795 sat in every static image ever produced, and the
assertion that would have caught it on day one runs in-process against the
writer's own output with no linking, no runner, and no other machine.

So the rule for where a case belongs:

> An integration test earns its place only when the fact it asserts is owned by
> something outside this repo - a foreign toolchain's conventions, a real
> kernel's behaviour, a real linked image, another architecture's execution.
> Everything else is a unit test that has not been written yet.

The corollary is uncomfortable and worth saying: an integration case written for
a fact we own is not merely redundant, it is *worse* than the unit test it
displaced. It runs later, fails slower, localizes nothing, and its green is read
as covering more than it does.

## Loader and image layout the program never consults

**#2795.** Every static ELF mach had ever produced carried two `PT_LOAD` segments
claiming `0x400000`: the header block, and `.text`. Every one of those programs
ran correctly, on all three ELF targets, because the loader maps segments in order
and the later mapping wins. No behavioural observation could ever have differed,
whatever the program computed, because a program's execution never reads its own
program headers.

Three things generalize.

**The loud consumer was luck.** `gdb 17.2` aborted at process start, which is how
this was found. A crash reporter, profiler, or symbolizer deriving load addresses
the same way would have attributed addresses to the wrong file range and reported
a confident wrong answer. "A consumer noticed" is not a detection mechanism.

**Assert a property, not a pair.** The guard reads the emitted program headers and
asserts that no two load segments overlap in virtual address, rather than
asserting something about `phdr[0]`. That is what makes it survive the next
segment-layout change instead of quietly ceasing to apply.

**It is a unit test.** No image needs to be linked or run to check this. See the
section above.

## Self-consistent format errors

**#2797.** mach resolved `R_RISCV_PCREL_LO12_I/_S` by taking the low half's own
symbol as the target. For the psABI spelling that symbol is a **label at the
paired `auipc`**, and the real target must be read off the `R_RISCV_PCREL_HI20`
sitting there. mach never chased the label, and measured from a fixed
`patch_va - 4`, so the displacement collapsed to a number about where the two
instructions sit, with the actual target nowhere in it - at every distance,
unconditionally.

It took a foreign object to see, because mach's own emitter and mach's own reader
shared the assumption. mach-built inputs came out right and every mach-only case
agreed with the bug. That is the defining property of this class: **the oracle is
not independent of the thing it is checking.**

Two refinements the repo has already paid for:

- **It needed that *specific* clang.** A newer one builds the same constant with
  `lui`+`slli` and emits no relocation at all, so there is no pair to get wrong.
  Whether a probe trips a relocation defect is a property of the shape its
  compiler happened to emit, never of the compiler being new enough. A fixture
  depending on a foreign toolchain emitting a particular shape must **assert the
  shape is still present**, or it will stay green forever while testing nothing.
- **A structural observation is not automatically independent either.** A
  producer that re-derives its fact through the code under test is self-consistent
  under the bug in exactly the same way (#2759, where reading a `DW_OP_fbreg`
  offset a second way through the same path could not see the defect). Looking at
  the artifact instead of the output buys nothing on its own. Independence is the
  property, not location.

**#2766** is the same class caught the right way round: the float load/store
encoders substituted another width's form rather than refusing, and byte
comparison against `llvm-mc` settles that, because `llvm-mc` does not share our
opinion about what the bytes should be. Note that this needs no integration test -
an external oracle is not the same thing as an external test run.

## A wrong value that is still a value

The failure mode that survives longest produces a believable number. #2797's
observable was `regs=0 0` / `tail=0 0 0` - not a crash, not garbage, just zeros,
plausible enough that the case was set aside on that leg twice under two different
wrong diagnoses.

- **Carry the values, and carry an effect that must have moved.** A store resolved
  to the wrong address leaves the original value in place and corrupts a
  neighbour, so a readback having *changed* is the assertion, not that the call
  returned.
- **Never cross-check one target against another.** #2749 had two targets agreeing
  with each other and both being right for reasons unrelated to the defect. Every
  lane wants a hand-computed known value.
- **A blessed golden records whatever the compiler currently does.** A defect that
  predates the case is blessed into it. A golden written by blessing has never
  been checked by anything.

## A failure the success path swallows

The darwin libSystem work hit the sharpest instance. Every other libSystem entry
point returns `-1` and leaves the reason in errno, so the module's house rule is:

```mach
if (rc < 0) { ret ls.fail_errno(); }
```

**The pthread family returns the error number directly and never touches errno.**
`rc` is never negative, so the branch never fires and a failure returns as
**success**. A test asserting "the call succeeded" is asserting the wrapper's own
opinion of itself, and the wrapper is the thing under test.

Same shape, two other doors: `ut_lower_ok` reported `run_lower_pass`'s return
value while lowering recorded errors on the session sink and returned normally, so
"clean" and "recorded an error, returned ok" rendered identically (#2297). And
`ut_diag_has` scanned a diagnostic's `message` only, so until `ut_diag_note_has`
(#2819) no test in the tree could assert anything about a note's text.

The surface is always the same: **assert the effect, not the return.** The darwin
pthread tests are the model - object sizes checked by locking a real mutex inside
a guard-filled slab and verifying the guard bytes past the declared size are
untouched rather than asserting a constant against itself, a freed context block
checked by requiring `mprotect` over that range to fail afterwards, a bucket
collision constructed by scanning for two words that hash together rather than
assumed. All local. None of it needed a mac.

## Anything on a leg that never executed

mach-std's CI compiled the darwin backends from linux and **ran nothing**, for
months, reporting green. The first native lane reported `777 passed, 11 failed`,
identical on both macOS architectures, the same eleven names on both, all
predating the work that added the lane. Three subsystems were non-functional and
the suite had said nothing, because a suite that does not run reports no failures.

This is the class that genuinely requires execution on the real platform. What a
current kernel does is not derivable from our source, and a cross-compile proves
only that something was emitted.

Weaker versions of the same gap, worth carrying into the replacement:

- **An emulator is not a kernel.** qemu-user does not fault an `mprotect` over
  address space the loader never mapped, and its guest page size need not match a
  real kernel's, so mprotect-class behaviour is proven only by native execution
  (#1885, where a 64K-aligned aarch64 image faulted on a native 4K-page kernel
  that every emulated run had reported green).
- **A build-only check proves emission and nothing else.** That is honest, and it
  is also the whole of what it says.
- **A skipped leg must leave a trace.** A leg a case never applies to and a leg it
  was excluded from must not look identical in the output.

Until a runner exists, the honest substitute is an explicit list that fails **both
ways** - on a new failure and on a listed test that starts passing. That is what
forces it to shrink instead of quietly becoming permanent.

## Emitted shape where the answer is unchanged

A vectorizer that stops firing, an inliner that stops inlining, a constant pool
that stops deduplicating, an encoder that stages every float operand through
scratch - all of them compute the right answer. A call and the body inlined in its
place compute the same value by definition. Running the program is blind to every
one of these by construction, and an assertion over what was emitted is the only
thing that sees them.

This is the one place a shape assertion is the right instrument, which is exactly
where the next section draws its boundary.

## Assert values and effects, not shape

Prefer an observable that is a **value or an effect** - a lane value, a file mode,
a resolved symbol, a readback that moved, a count that changed - over one that is
the emitted form.

A value observable survives codegen changes. A shape observable breaks on all of
them, every break costs a rebless, and every rebless is an opportunity to bless a
defect. This repo has already paid it: goldens reblessed wholesale for an
unrelated symbol-mangling change, pure cost that a value-shaped observable would
not have incurred.

Shape assertions are **correct** where shape genuinely is the contract:

- `--emit-asm` output, which is a printed interface with its own consumers
- encoder output, where the bytes are the deliverable and an external assembler
  can be the oracle
- the class above, where the shape *is* the property under test and no value
  observable exists

They are wrong as a general-purpose assertion. If a case can state its fact as
"this value came back as 42" rather than "these instructions were emitted", state
it that way.

Where a case does mix the two, state each emitted fact as its **own** number, and
say plainly which of them do not catch the defect the case exists for. A check
that reads the same before and after the fix is worth keeping if it fails closed
for something else - but folding it in and letting it look like coverage is how a
suite comes to be believed for more than it does.

## The degenerate-fixture trap

A fixture can exercise a feature end to end, fail closed in principle, and still
be indistinguishable from several **wrong** implementations, because its inputs
are at the identity value on every axis at once.

**`test/link/cases/narrow-stack-args` is the worked example, and it is left exactly as
it is - this is a note about it, not a change to it.** It does fail pre-fix and it
is a real end-to-end guard. But under clang 18 its probe reaches exactly **one**
pooled constant: at section offset 0, with a zero addend on the high half, four
bytes from its low half, through the `_I` spelling only. That is the easiest
configuration on every axis a pc-relative pairing can get wrong. A fix that
recovered the target section but **dropped the symbol's offset**, or **dropped the
high half's addend**, or **still assumed a four-byte gap**, or handled `_I` and
not `_S`, passes it unchanged.

The generalization, and the sentence worth carrying to every fixture review:

> **Offset zero validates any scheme that drops the offset.** Adding nothing is
> indistinguishable from adding correctly.

It reaches far past relocations. A zero addend, a zero index, a single element
where the defect needs two, a gap that happens to equal the constant the code
wrongly assumes, one spelling of a shared code path, one target of three - each
makes a wrong implementation and a right one produce identical bytes.

`test/link/cases/riscv-pcrel-pair` is the answer to this particular one, built
non-trivial on each axis simultaneously and out of real clang output rather than a
contrivance: a symbol at a nonzero offset in the same section as another, nonzero
addends on the high half, hi-to-lo gaps of 4, 6 and 8 bytes, and both the `_I` and
`_S` spellings, which share one resolution path that only one of them exercised.

When reviewing a fixture, list the axes the defect could vary along and check
whether the inputs sit at the identity value on any of them. **Write down what is
not covered.** That is a real outcome on its own - it is what stops the next
person reading "this case exists" as "this axis is covered".

## Demonstrate the failure before adding the case

A case is worth its maintenance only if it can be shown to fail against the
unpatched compiler. Run it against the commit immediately before the fix and
record what happened, per platform.

This is not a formality. One recent batch added eleven probes over combinations
nothing else reached, and exactly **one** of them flipped to wrong against a
pre-fix compiler, on one architecture only - the other two read correct pre-fix
for reasons unrelated to the defect. The other ten are real coverage of untested
combinations and are **not** regression pins, and both the case and its PR say so.

That is the standard. A case that cannot be shown failing may still be worth
keeping, but it must be **labelled as coverage rather than as a pin**, because the
alternative is a suite whose green is believed to mean more than it does.

## A diagnosis can be plausible and wrong

Not a blind spot of any harness, but a failure mode this kind of defect invites,
and it has cost more time here than any missing observation.

- **#2797's own issue body diagnosed the wrong cause.** It attributed the collapse
  to `auipc` sharing at link time - nine pairs merged onto one shared `auipc` with
  only the first pair's low-12 surviving. That mechanism does not exist in the
  linker. The wrong diagnosis was internally coherent and matched the disassembly.
- **A correlated observable is not a cause.** `e_flags = 0` on a linked riscv64
  image claims a soft-float ABI, and a soft-float / hard-float mismatch produces
  *precisely the zeros* #2797 produced. Two of the fixes chased for that symptom
  (`-mabi=lp64d`, `-march=rv64gc`) were real, necessary, correct, and did not fix
  it.
- **#2795 looked like an inlining bug.** It was found while investigating why
  inlining an asm-bearing callee made gdb abort. The inlining only changed segment
  sizes enough to tip an already-broken computation over, which is why every gate
  keyed on the inlining appeared to fix it.

Two habits fall out. **Change exactly one field and nothing else** - #2795 was
root-caused by patching only `phdr[0].p_vaddr` in an already-built binary, same
code bytes, same everything, and watching gdb start working. And **state a control
observable that separates the candidate causes**, so that when the primary
observable goes wrong the report names which of the two it was.
