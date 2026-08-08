# What a behavioural golden cannot see

`int/run.sh` builds a case, runs a producer over the result, and diffs the
producer's normalized text against a golden. For most cases the producer is
`exec`: run the program, take its stdout. That is the tool this repo reaches for
first, and it is the right default. This document is about where it is
**structurally** blind, and which surface can see each of those places instead.

The purpose is to shorten the loop. Every entry below was paid for once already,
and the cost was not writing the eventual fix - it was the time between the
defect existing and anything being able to observe it. Reading this before
choosing a `run:` mode is the whole point of it living next to
[`lib/produce.sh`](lib/produce.sh)'s producer catalogue.

Scope: this describes the harness **as it is today**. It is a map of the current
system's observational limits, not a proposal to change it.

## The one rule everything below follows from

A golden observes exactly what its producer prints, and nothing else. It is not a
statement about the program, the image, or the compiler. It is a statement about
one string.

So the question a case must answer is never "does this test the feature". It is
"if the defect I am guarding against were present, would this string differ". If
the answer is no, the case is maintenance with no return, however green it looks
and however much of the feature it exercises.

## Class to surface

| Defect class | Why `exec` cannot see it | Surface that can |
|---|---|---|
| Loader / image layout the running program never consults | the loader resolves the ambiguity and the program is correct | structural assertion over the emitted headers, `field` / `relro` / `macho-framing` producers, or driving a real consumer (`debugger-gdb`) |
| A self-consistent format error | every producer and consumer in the tree shares the mistake, so they agree | a **foreign** artifact as input (`lib/cc.sh`, `macho-foreign-*`, `pe-foreign-*`), or an external oracle (`llvm-mc`, `llvm-dwarfdump --verify`, `spirv-val`, `readelf`) |
| A wrong value that is still a value | the program prints a plausible number and exits 0 | an independently known answer - hand-computed values, a foreign toolchain's result, never another target's output |
| A failure the success path swallows | the wrapper's own error check never fires, so the failure returns as success | assert the **effect**, not the return code |
| Anything on a leg that never executed | there is no observation at all, and no observation reports green | a native runner, plus an explicit known-failures list until there is one |
| Emitted shape where the answer is unchanged | a correct answer computed the wrong way is still a correct answer | the emission producers - `vector-emit`, `vector-lanes`, `call-shape`, `float-emit`, `const-pool`, `embed-dedup`, `spirv-shader` |
| A type error the backend tolerates | the backend selects from operand types, so the mistyping never reaches the output | `--verify-ir` |
| A diagnostic that does not fail its pass | "clean" and "recorded an error, returned ok" render identically | the sink-reading helpers, `ut_lower_errs` / `ut_lower_clean`, `ut_diag_note_has` |
| A performance regression of any size | the answer is unchanged by construction | not `int`. this suite has no timing observable and should not grow one |

The rest of this file is the evidence for each row.

## Loader and image layout the program never consults

**#2795.** Every static ELF mach had ever produced carried two `PT_LOAD` segments
claiming `0x400000`: the header block, and `.text`. Every one of those programs
ran correctly, on all three ELF legs, because the loader maps segments in order
and the later mapping wins. No behavioural golden could ever have differed, no
matter what the program computed, because the program's own execution never reads
its own program headers.

Two things are worth taking from it beyond the fix.

**The loud consumer was luck.** `gdb 17.2` aborted at process start, which is how
this was found. A crash reporter, profiler, or symbolizer deriving load addresses
the same way would have attributed addresses to the wrong file range and reported
a confident wrong answer. The detection mechanism here was not "a consumer
noticed", it was "the consumer that noticed happened to be one that dies loudly".
Do not build a coverage argument on that.

**The assertion is a property, not a pair.** The test reads the emitted program
headers and asserts that no two load segments overlap in virtual address, rather
than asserting something about `phdr[0]`. That is what makes it survive the next
segment-layout change instead of quietly ceasing to apply.

Related, same class: `#2813`, where a linked riscv64 image carries `e_flags = 0`
and so claims a soft-float non-compressed ABI it is not. Nothing at runtime reads
the field, so no execution can ever disagree with it. `int/surface/elf-relro`,
`macho-framing-*`, `pe-aslr` and the `field` producer all exist for this class.

## Self-consistent format errors

**#2797.** mach resolved `R_RISCV_PCREL_LO12_I/_S` by taking the low half's own
symbol as the target. For the psABI spelling that symbol is a **label at the
paired `auipc`**, and the real target must be read off the `R_RISCV_PCREL_HI20`
sitting there. mach never chased the label, and measured from `patch_va - 4`, so
the displacement collapsed to a number about where the two instructions sit with
the actual target nowhere in it.

The reason this took a foreign object to see: mach's own emitter and mach's own
reader shared the assumption, so mach-built inputs came out right. Every
mach-only case agreed with the bug. It needed a clang-18 object, and it needed
that specific clang, because a newer one builds the same constant with
`lui`+`slli` and emits no relocation at all - there is no pair to get wrong.

That last point generalizes past this defect. **Whether a probe trips a
relocation bug is a property of the shape its compiler happened to emit, never of
the compiler being new enough.** A fixture that depends on a foreign toolchain
emitting a particular shape must assert that the shape is still there.
`int/surface/riscv-pcrel-pair` prints `label-pairs=yes` for exactly that reason:
if clang stops emitting the pair, the case says so instead of staying green
forever while testing nothing.

The same shape appears inside producers. `varloc-fbreg` exists because reading a
`DW_OP_fbreg` offset alone cannot see #2759 - a producer that derives the offset
a second way through the same code is self-consistent under the bug. **A producer
must not re-derive its observable using the code under test.** That is the
difference between a check and a round trip.

**#2766** is the same class caught the right way round: the float load/store
encoders substituted another width's form rather than refusing, and byte
comparison against `llvm-mc` is what settles a question like that, because
`llvm-mc` does not share mach's opinion about what the bytes should be. Any
external oracle works for this - `spirv-val` for SPIR-V modules, `llvm-dwarfdump
--verify` for DWARF, `readelf` for header facts, a foreign linker consuming
mach's objects (`#2828`).

## A wrong value that is still a value

The failure mode that survives longest is the one that produces a believable
number. #2797's observable was `regs=0 0` / `tail=0 0 0` - not a crash, not
garbage, just zeros, which read as a plausible answer for long enough that the
case was exempted on the leg twice under two different wrong diagnoses.

Consequences for how a case is written:

- **Carry the values, not the verdict.** `riscv-pcrel-pair` prints the loaded
  values, and prints values read back **after** a store, because a store resolved
  to the wrong address leaves the original in place and corrupts a neighbour - the
  readback having moved is the assertion, not that the call returned.
- **Never compare one target's output against another's.** `#2749` had two
  targets agreeing with each other and both being right for reasons unrelated to
  the defect. `int/surface/vector-carry-and-boundary` hand-computes every lane and
  checks it against a known value for this reason.
- **A golden blesses whatever the compiler currently does.** `--bless` makes it
  cheap to record a defect that predates the case as the expected answer. A golden
  written by blessing has never been checked by anything.

## A failure the success path swallows

The darwin libSystem migration hit the sharpest instance of this. Every other
libSystem entry point returns `-1` and leaves the reason in errno, so the module's
house rule is:

```mach
if (rc < 0) { ret ls.fail_errno(); }
```

**The pthread family returns the error number directly and never touches errno.**
`rc` is never negative, so the branch never fires and a failure returns as
**success**. A behavioural test asserting "the call succeeded" is asserting the
wrapper's own opinion of itself, and the wrapper is the thing under test.

This is the same shape as `ut_lower_ok`, which reported `run_lower_pass`'s return
value while lowering recorded errors on the session sink and returned normally, so
"clean" and "recorded an error, returned ok" rendered identically. And the same
shape one field over as `ut_diag_has`, which scanned a diagnostic's `message` only,
leaving every note the compiler attaches unpinned until `ut_diag_note_has` (#2819).

The surface is always the same: **assert the effect, not the return.** mach-std's
pthread tests are the model worth copying. Object sizes are checked by locking a
real mutex inside a guard-filled slab and verifying the guard bytes past the
declared size are untouched, rather than asserting a constant against itself. The
context block being freed is checked by requiring `mprotect` over that range to
fail afterwards. A bucket collision is constructed by scanning for two words that
hash together, not assumed.

## Anything on a leg that never executed

mach-std's CI compiled the darwin backends from linux and **ran nothing**, for
months, reporting green. The first native lane found `777 passed, 11 failed`,
identical on `macos-15` and `macos-15-intel`, the same eleven names on both, all
predating the work that added the lane. Three subsystems were non-functional and
the suite had said nothing, because a suite that does not run reports no failures.

Weaker versions of the same gap:

- **qemu is not a native kernel.** qemu-user does not fault an `mprotect` over
  address space the loader never mapped, and its guest page size need not match a
  real kernel's, so RELRO and mprotect-class behaviour is only proven by a
  `native` leg (#1885). `run.sh`'s header states this and it is why the aarch64
  int leg stays native.
- **`built` proves emission and nothing else.** For a cross-built target with no
  host runner the observable is a constant. That is honest and it is also the
  whole of what it says.
- **A case exempted on a leg reports nothing on that leg.** `run.sh` prints a
  `SKIP ... (exempt, see case.conf)` line precisely so a leg a case never applies
  to and a leg it was excluded from stop looking identical.

Until a runner exists, the honest substitute is an explicit list that fails both
ways - mach-std's `test/darwin/known-failures.txt` fails on a new failure **and**
on a listed test that starts passing, which is what forces it to shrink instead of
quietly becoming permanent.

## Emitted shape where the answer is unchanged

A vectorizer that stops firing, an inliner that stops inlining, a constant pool
that stops deduplicating, an encoder that stages every float operand through
scratch - all of them compute the right answer. A call and the body inlined in its
place compute the same value by definition. `exec` is blind to every one of these
by construction, and the emission producers (`vector-emit`, `vector-lanes`,
`call-shape`, `float-emit`, `const-pool`, `embed-dedup`, `spirv-shader`) exist for
exactly this class. `lib/produce.sh` says so per producer.

This is the one place a shape golden is the right instrument, and the boundary
matters - see the next section.

## Assert values and effects, not shape

Prefer an observable that is a **value or an effect** - a lane value, a file mode,
a resolved symbol, a readback that moved, a byte count that changed - over one
that is the emitted form.

A value observable survives codegen changes. A shape observable breaks on all of
them, and every one of those breaks costs a rebless, and every rebless is an
opportunity to bless a defect. The repo has already paid this: `a186437c`
reblessed the `float-emit` and simd-shortfall goldens for an unrelated mangling
change, which is pure cost that a value-shaped observable would not have incurred.

Goldens over emitted shape are **correct** where shape genuinely is the contract:

- `--emit-asm` output, which is a printed interface with its own consumers
- encoder output, where the bytes are the deliverable and an external assembler
  can be the oracle
- the emission producers above, where the shape **is** the property under test and
  no value observable exists

They are wrong as a general-purpose assertion. If a case can state its fact as
"this value came back as 42" instead of "these instructions were emitted", state
it that way.

`riscv-pcrel-pair` shows the honest mixture: it carries the loaded values as the
primary observable, plus three named facts about the emitted image, each stated as
its own number - including `text-relative=0`, which reads 0 both before and after
the fix and is documented as **not** catching this defect. Stating a check that
does not catch the bug, and saying so, is better than folding it in and letting it
look like coverage.

## The degenerate-fixture trap

A fixture can exercise a feature end to end, fail closed in principle, and still
be indistinguishable from several wrong implementations, because its inputs are
degenerate on every axis at once.

**`int/surface/narrow-stack-args` is the worked example.** It does fail pre-fix
and it is a real end-to-end guard. But under clang 18 its probe reaches exactly
**one** pooled constant: `.LCPI1_0`, at section offset 0, with a zero addend on
the high half, four bytes from its low half, through the `_I` spelling only. That
is the easiest configuration on every axis a pc-relative pairing can get wrong. A
fix that recovered the target section but **dropped the symbol's offset**, or
**dropped the high half's addend**, or **still assumed a four-byte gap**, or
handled `_I` and not `_S`, passes it.

The generalization, and the sentence worth carrying to every fixture review:

> **Offset zero validates any scheme that drops the offset.** Adding nothing is
> indistinguishable from adding correctly.

It applies far past relocations. An addend of zero, an index of zero, a single
element where the bug needs two, a gap that equals the constant the code
wrongly assumes, one spelling of a shared path, one target of three - each of
them makes a wrong implementation and a right one produce the same bytes.

`int/surface/riscv-pcrel-pair` is the answer to this one, built non-trivial on
each axis simultaneously and out of real clang output rather than a contrivance:
a symbol at a nonzero offset in the same section as another, nonzero addends on
the high half, hi-to-lo gaps of 4, 6 and 8 bytes, and both the `_I` and `_S`
spellings, which share one resolution path that only one of them was covering.

When reviewing a fixture, list the axes the defect could vary along and check
whether the fixture's inputs are at the identity value on any of them. If they
are, either strengthen the inputs or write down what the case does not cover.
Writing it down is a real outcome - it is what stops the next person reading
"this case exists" as "this axis is covered".

## Demonstrate the failure before adding the case

A new case is worth its maintenance only if it can be shown to fail against the
unpatched compiler. Run it against the commit immediately before the fix and
record what happened, per leg.

This is not a formality. `int/surface/vector-carry-and-boundary` added eleven
probes over combinations nothing else reached, and exactly **one** of them
(`mul_loop3`) flipped to wrong against a pre-fix compiler, on `linux` only -
`linux-arm64` and `linux-riscv64` read correct pre-fix for reasons unrelated to
the defect. The other ten are real coverage of untested combinations and are
**not** regression pins, and both the case header and its PR say so.

That is the standard. A case that cannot be shown failing may still be worth
adding, but it must be labelled as coverage rather than as a pin, because the
alternative is a suite whose green is believed to mean more than it does.

## A diagnosis can be plausible and wrong

Not a blind spot of the harness, but a failure mode the harness invites, and it
has cost more time here than any single missing producer.

- **#2797's own issue body diagnosed the wrong cause.** It attributed the collapse
  to `auipc` sharing at link time - nine pairs merged onto one shared `auipc` with
  only the first pair's low-12 surviving. That mechanism does not exist in the
  linker. The real cause was the reader never chasing the label, so the delta
  collapsed **regardless of gap**, at every distance, unconditionally. The wrong
  diagnosis was internally coherent and matched the disassembly.
- **A correlated observable is not a cause.** `e_flags = 0` on a linked riscv64
  image claims a soft-float ABI, and a soft-float / hard-float mismatch produces
  *precisely the zeros* the real bug produced. Two of the fixes chased for this
  symptom (`-mabi=lp64d`, `-march=rv64gc` in `lib/cc.sh`) were real, necessary,
  correct, and did not fix it.
- **#2795 looked like an inlining bug.** It was found while investigating why
  inlining an asm-bearing callee made gdb abort. Inlining only changed segment
  sizes enough to tip an already-broken computation over, which is why every gate
  keyed on the inlining appeared to fix it.

Two habits fall out. **Change exactly one field and nothing else** - #2795 was
root-caused by patching only `phdr[0].p_vaddr` in an already-built binary, same
code bytes, same everything. And **state a control observable that separates the
candidate causes**: `riscv-pcrel-pair`'s `scale=` row is the only observable in
that case crossing a float over the mach-to-C boundary, so it reports an ABI
mismatch rather than a relocation one, and the case says outright that if it
zeroes while the others stay correct the answer is in `abi =` and not in the
linker.

## Choosing a surface

Before writing a case, answer these in order.

1. **Would the defect change the program's output?** If no, stop reaching for
   `exec`. Go to 2.
2. **Is the fact in the emitted image rather than in the run?** Use a structural
   producer (`field`, `relro`, `macho-framing`, `pe-imports`, the emission
   producers), and assert a **property** over what is emitted, not a claim about
   one entry.
3. **Could mach's own reader agree with mach's own writer about it?** Then the
   input or the oracle must be foreign. `lib/cc.sh`, a `macho-foreign-*` /
   `pe-foreign-*` fixture, `llvm-mc`, `llvm-objdump`, `llvm-dwarfdump --verify`,
   `spirv-val`.
4. **Does the answer come back as a plausible value on failure?** Print the
   values, print an effect that must have changed, and never cross-check against
   another target.
5. **Does the leg actually execute?** A qemu leg does not prove mprotect
   behaviour. A `built` case proves emission. A cross-compile proves nothing about
   running.
6. **Are the fixture's inputs at the identity value on any axis the defect could
   vary along?** If so, fix the inputs or write down the gap.
7. **Does it fail against the unpatched compiler?** Run it and record the answer
   per leg, including the legs where it does not fail.

## Where each surface lives

- producers and their per-mode rationale - [`lib/produce.sh`](lib/produce.sh)
- `case.conf` keys and leg resolution - [`lib/case.sh`](lib/case.sh)
- legs, runners and run-modes - [`targets.conf`](targets.conf)
- foreign C objects for a case - [`lib/cc.sh`](lib/cc.sh)
- run-mode and native-vs-qemu fidelity - [`run.sh`](run.sh)'s header
- IR-level and diagnostic surfaces (`--verify-ir`, `ut_lower_errs` /
  `ut_lower_clean`, `ut_diag_has` / `ut_diag_note_has`) are in the compiler's own
  `mach test .` suite, not here
