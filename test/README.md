# The codegen corpus

One question, asked directly: **is the compiler generating the code we told it to
generate, on every registered target?**

The corpus is a flat set of single-file mach programs, one codegen surface per file,
run by one driver through three independent oracle layers. Nothing here restates a
constant the compiler owns. Every oracle is grounded outside mach: an external
disassembler, an external validator, or execution compared against a C reference
compiled by the host's own C compiler.

```
test/
  run.sh                          driver entry point
  tools.lock                      pinned external tool versions, checked at startup
  engines.conf                    the target registry: one row per target
  cases/<group>/<case>.mach       one case, one codegen surface, target-independent
  ref/<group>/<case>.c            the C reference translation of the case
  golden/<target>/<g>/<c>.dis     blessed external-disassembler text
  golden/<target>/SKIPS           declared per-target skips, each with a reason
  lib/                            driver internals, plus the fold prelude both sides use
  out/                            all build products, gitignored
```

## Running it

```
run.sh                          # every case, every target this host serves, every layer
run.sh --runner <label>         # the targets engines.conf assigns to that CI runner
run.sh --target <t>             # one target (repeatable)
run.sh --case <group>/<name>    # one case (repeatable), still all layers
run.sh --layer a|b|c            # one layer (repeatable)
run.sh --bless [--target <t>]   # regenerate goldens, print diffs, never in CI
run.sh --matrix                 # print the coverage matrix from the last run
```

Nonzero exit on any failure, any unexplained empty cell, or any tool-pin mismatch.

Two environment variables:

- `MACH_CORPUS_OUT` is the output directory, default `test/out`. Everything a run
  writes lives under it, so **two invocations that name different directories are
  fully independent and may run concurrently**. One directory takes one invocation
  at a time, and a second is refused by name rather than allowed to corrupt the first.
- `MACH_CORPUS_MACH` is the compiler under test, default the checkout's
  `out/<host>/debug/bin/mach`. The driver never falls back to a `mach` on `PATH`,
  which is usually an older release, and it prints the path and version it measured
  at startup.

## The case contract

A case is one file, `cases/<group>/<case>.mach`. The driver enforces this contract
rather than trusting it.

1. **One public function.** `pub fun checksum(seed: u64) u64`. The case defines no
   entry point, prints nothing, and reaches no standard library. The driver owns the
   entry: on a hosted target it wraps the case in an entry that prints the checksum,
   and on a target that executes nothing it compiles the case file directly. That
   split is what lets one case file serve `spirv` and `riscv32` as well as linux.

2. **Target-independent source.** No target conditionals, no inline asm, no
   OS-specific calls. The same source compiles for every target, or is declared in
   that target's `SKIPS` with a reason.

3. **Deterministic.** No input, no time, no address observed as a value, no float
   printing. Terminates in well under a second on the slowest engine.

4. **Folds every intermediate with FNV-1a**, through `corpus.lib.fold`. Float results
   fold by bit pattern (`mix_f32`, `mix_f64`), never by value, so NaN and `-0.0` are
   exact. Fold in a fixed order: order is what makes a wrong lane or a wrong element
   change the checksum instead of cancelling out.

5. **Operands stay inside mach's defined semantics AND the defined-behaviour C
   mapping below.** No case may depend on behaviour that is UB in the C translation.

6. **Everything that matters flows through the opaque `seed` at least once.** `seed`
   is `argc - 1`, which is `0` at run time and unknowable at compile time, so the
   optimizer cannot fold the case to a constant. Add it into a value, or use it as
   an index or a loop bound the compiler cannot pin. A case whose result the
   optimizer can compute entirely is exercising the constant folder, not codegen.
   The `comptime` group deliberately does the opposite.

The fold prelude is `lib/fold.mach`, module `corpus.lib.fold`:

```mach
use corpus.lib.fold;

pub fun checksum(seed: u64) u64 {
    var h: u64 = fold.init();
    h = fold.mix_u32(h, some_u32_result);
    h = fold.mix_f32(h, some_f32_result);
    ret h;
}
```

`init()`, then `mix_u8`/`u16`/`u32`/`u64`, `mix_i8`/`i16`/`i32`/`i64`, `mix_f32`,
`mix_f64`. Unsigned widths zero-extend, signed widths sign-extend and fold their
two's-complement pattern, floats fold their bits.

## The C reference, and its rules

`ref/<group>/<case>.c` is a line-for-line translation of the case. It is the
semantic anchor: the checksum's truth comes from the host platform's C toolchain,
not from mach. It defines `uint64_t checksum(uint64_t seed)` and includes
`corpus.h`, which supplies the same fold and the `main` that prints the result, so
the reference file holds the translation and nothing else.

- All arithmetic is on unsigned fixed-width types (`uint8_t`..`uint64_t`) with an
  explicit cast at every width change. Signed mach operations are expressed via
  two's-complement identities on unsigned types.
- **No operation in any reference file may be UB in C.** A disagreement must always
  indict mach, never the reference. This is enforced, not asked for: every reference
  is also built with `-fsanitize=undefined -fno-sanitize-recover=all` and run, and
  any UB aborts the run as a harness defect. That build is a check on the reference
  **sources**, which are one set of files, so a host whose toolchain can run it
  settles it for every leg. A host whose cannot says so in a `no-cflags` row and the
  driver names it at startup: today that is windows, where mingw ships no UBSan
  runtime.
- The reference is compiled at both `-O0` and `-O2` and the two must agree with each
  other before either is compared against mach. Disagreement between them fails the
  run as a harness defect, not a mach defect.
- The exact flags are `cflags` rows in `tools.lock`. `-ffp-contract=off` is one of
  them: a fused multiply-add on one side and not the other would make the anchor
  disagree about rounding rather than about semantics.
- The reference runs on the host only. Cross-target agreement is transitive: every
  target's output equals the host C answer independently, so no leg talks to another.

## The oracle layers

Every artifact passes through three independent layers. A layer never consumes
another layer's output.

**Layer A, structural validation.** Every object and artifact, every target, both
pipelines, plus a third `-g` build. `llvm-readobj --file-header --sections --relocs`
must parse cleanly, and the class, machine and endianness must match the target.
SPIR-V goes to `spirv-val` under the environment the module itself declares.
`llvm-dwarfdump --verify` must pass on the debug build.

**Layer B, golden disassembly.** The release pipeline only, to bound churn. The
case's own object is disassembled by an **external** decoder and diffed against a
checked-in text golden. The flag set is a `disasm-flags` row in `tools.lock`, not a
constant in the driver, because a golden is only reproducible against a named tool
*and* a named flag set. `--symbolize-operands` and `--no-leading-addr` mean the
golden holds instructions and operands and no addresses, so a layout shift that
changed no code changes no golden. The point of the external decoder: an instruction
mach encodes wrong is displayed wrong or refused, so the golden cannot agree with a
bad encoding by construction. `run.sh --bless` regenerates goldens, prints the full
diff, and refuses to run when `CI` is set. A golden change lands in the same PR as
the compiler change that caused it.

**Layer C, differential execution.** The miscompile net. Every case is built at both
`-O0` and `-O2` for every target with an execution engine, run, and its checksum
compared against the C reference answer. All four numbers must be identical. An
`-O0`/`-O2` disagreement inside mach is a pipeline miscompile caught with no golden
and no reference needed. This layer's expected values never change unless language
semantics change, which is what retires regression maintenance.

## engines.conf

The single source of truth for how each target is built, executed and decoded.
Registering a target is one row and nothing else: the driver generates its manifest
from those columns. `engine = none` is a public label, not a default, and a target
carrying it has no layer C column at all.

Two different questions get asked of this file and it is worth keeping them apart.
**Which leg carries a target** is what the `runner` column answers, and it is the
only question CI asks. **Can this machine serve a target** is a different one, and
on a developer's machine it is the right selector: `run.sh` with no assignment runs
everything the host can do.

In CI the runner column is the **assignment** and host capability is only a
**check**. `run.sh --runner <label>` covers exactly the rows the registry gives that
label, and a row this host cannot serve is a refusal that names the row and the
reason, never a quiet substitution of some other set. A row the runner does not own
is named in the run header as `not assigned`, so a leg a run was never applicable to
and a leg it was dropped from do not look alike.

Capability is checked against layer C only, because that is the layer needing an
engine: one host decodes and validates every column, which is how a target with no
runner of its own still gets goldens.

Before this, the driver selected by capability everywhere, and a target with
`engine none` needs no execution host, so `spirv`, `riscv32` and `mos6502` were
recomputed on the windows and arm runners as well. That is coverage nobody scheduled
and, for spirv, coverage whose pinned decoder has no build for those hosts at all
(#2948). Windows accordingly needs no spirv-tools.

## tools.lock

Every oracle is pinned, and a pin nothing can install is a wish. Each `tool` row says
what a version must be, a `source` row says where that version is obtained, and an
`alias` row says how a host spells the executable when it is not the POSIX name.
A `no-cflags` row names a reference build mode a host's C toolchain cannot run, with
a reason, and the driver prints it at startup rather than dropping it in silence. The
one that exists is `ubsan` on windows, since mingw ships no UBSan runtime: that build
is a harness self-check over the reference **sources**, which are identical on every
host, so a toolchain that can run it settles it for every leg.

`run.sh --tools` prints the rows a given selection depends on, which is what
`ci-tools.sh` installs from, so a workflow never carries a second copy of a version
and cannot install one thing while the driver demands another.

## Declared skips

`golden/<target>/SKIPS` is where a column says what it cannot cover. One entry per
line:

```
<case-glob>  <layers>[:<profiles>]  <reason>
```

`layers` is any of `a`, `b`, `c`, or `*` for all three. Naming pipelines after a
colon declares away those cells and no others, so `a:o2` leaves `o0` and `g`
reporting. That axis exists because a defect is routinely one pipeline wide, and a
skip that swallowed the pipelines either side of it would delete the evidence that
the other two pass, which is the evidence that identifies the defect. A pipeline the
layer never runs is a refusal at startup rather than an entry that quietly matches
nothing.

A `#` opens a comment only at the start of a line: the reason runs to the end of the
line and routinely contains the issue number it dies with.

Two kinds of entry live in these files and they are not interchangeable.

- **Target limitation.** The target cannot express the construct. The corpus has no
  question to ask, and the entry stays until the target or the language changes.
- **Mach defect.** The target can express the construct and mach does not emit it,
  either refusing to build or writing an artifact the external validator rejects.
  The corpus has a question and mach cannot yet answer it. These name an issue and
  are deleted by the change that closes it.

A skip is a reviewed statement about coverage, never a way to make a red cell green.
Where mach computes a **wrong answer** rather than refusing, there is no skip at all:
a refusal leaves a diagnostic behind, and a wrong answer leaves nothing, so declaring
it away would destroy the only evidence. Those cells stay red until the compiler is
fixed.

## Honesty mechanisms

The dominant historical defect class is a test reporting more than it verifies.

- Every run emits the coverage matrix: case × target × layer × pipeline, naming the
  engine that produced each layer C cell, so a qemu result can never be mistaken for
  native ABI evidence.
- The run **fails** if any target has an empty layer A or B column, or if a target
  with a declared engine has an empty layer C column, unless the missing cell is
  covered by a `SKIPS` entry. There are no silent skips: a skip lives in a reviewed
  file with a reason, and the matrix counts skips separately from passes.
- A case that produces no output, wrong-shaped output, or a nonzero exit is a
  **failure**, never a skip.
- The driver verifies `tools.lock` against the machine before running anything and
  refuses to start rather than produce a result an unpinned tool could have shaped.
  It checks only the rows the selected targets and layers actually reach, so a host
  missing a tool no selected leg needs is not blocked by it.

## Adding a case

1. Write `cases/<group>/<name>.mach` to the contract above.
2. Write `ref/<group>/<name>.c` as its translation.
3. `MACH_CORPUS_OUT=<your own dir> ./run.sh --case <group>/<name> --layer c` until
   the four checksums agree.
4. `./run.sh --bless --case <group>/<name>` and read the diff. A golden you have not
   read is not a golden.
5. `./run.sh --case <group>/<name>` must exit 0.
