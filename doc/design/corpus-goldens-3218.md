# Why the layer B goldens changed on the v5 integration branch

Every layer B golden in the codegen corpus changed on `feat/3218`: 91 cases for
each of the six machine targets and 82 for SPIR-V, 628 files in all. A change
that wide is exactly the shape a bulk bless is supposed to hide, so this document
records what produced it, which commit produced it, and what was checked before
the goldens were rewritten.

Layer B compares `llvm-objdump` disassembly of the `o2` build against
`test/golden/<target>/<group>/<case>.dis`. It is decode-only, so one x86-64 host
serves every target's column.

## The mechanism

Every corpus case folds its operands through `corpus.lib.fold`, and every
`fold.mix_*` entry point tail-calls the one byte loop in `fold.mix_u64`. Before
the change those folds were out-of-line calls. After it they are inlined, and the
eight-iteration byte loop appears once per fold site.

For `bits/logic_u32` on `x86_64-linux` the old disassembly is a sequence of
`callq` to the fold helper. The new one has no call at all: at each fold site the
FNV constants (`0xCBF29CE484222325`, `0x100000001B3`) and the shift-mask-multiply
loop appear inline. That is the same edit repeated across the corpus.

## What changed, per target

Cases are classified by the first rule that applies: fewer call instructions than
before (inlining), then a smaller first frame adjustment with no call removed,
then a larger first frame adjustment, then anything else. Calls are `call*` on
x86-64, `bl`/`blr` on aarch64, `jal`/`jalr` on riscv64, and `OpFunctionCall` on
SPIR-V. The frame adjustment is the first `subq $N, %rsp`, `sub sp, sp, #N`, or
`addi sp, sp, -N` in the case.

| target | cases changed | calls removed | frame shrank without call removal | frame grew | other | new case |
| --- | --- | --- | --- | --- | --- | --- |
| aarch64-darwin | 91 | 91 | 0 | 0 | 0 | 0 |
| aarch64-linux | 91 | 91 | 0 | 0 | 0 | 0 |
| riscv64-linux | 91 | 91 | 0 | 0 | 0 | 0 |
| spirv | 82 | 81 | 0 | 0 | 0 | 1 |
| x86_64-darwin | 91 | 91 | 0 | 0 | 0 | 0 |
| x86_64-linux | 91 | 91 | 0 | 0 | 0 | 0 |
| x86_64-windows | 91 | 91 | 0 | 0 | 0 | 0 |

No case falls in "frame grew" or "other", so there is nothing to list by name.
Every case that had a prior golden lost calls. The single case with no prior
golden is `spirv call/call_mixed`, which SPIR-V did not cover before; the whole
file is new, so its first differing line is line 1.

Frame direction is reported separately because it does not classify a case on its
own. Inlining a callee moves the callee's locals into the caller's frame, so a
frame can grow and a call can disappear in the same case.

| target | calls before | calls after | cases reaching zero calls | frame smaller | frame same | frame larger | no frame adjustment |
| --- | --- | --- | --- | --- | --- | --- | --- |
| aarch64-darwin | 4304 | 1760 | 31 | 32 | 9 | 26 | 24 |
| aarch64-linux | 4304 | 1760 | 31 | 32 | 9 | 26 | 24 |
| riscv64-linux | 4304 | 1760 | 31 | 64 | 5 | 8 | 14 |
| spirv | 3744 | 1453 | 31 | 0 | 0 | 0 | 81 |
| x86_64-darwin | 4316 | 1772 | 31 | 10 | 44 | 37 | 0 |
| x86_64-linux | 4316 | 1772 | 31 | 10 | 44 | 37 | 0 |
| x86_64-windows | 4316 | 1772 | 31 | 48 | 28 | 15 | 0 |

The call totals agree across targets to within the twelve calls x86-64 spends on
its own calling convention, which is what a single target-independent decision in
the middle end should look like.

## Which commit

Bisected on one case, `bits/logic_u32` for `x86_64-linux`, over the 73
first-parent commits in `a4107eb4..feat/3218`.

The bisect holds everything but the compiler fixed. Older commits carry a corpus
harness that emits manifests the compiler of that era rejects, and older manifests
are rejected by the bootstrap, so probing a commit in its own tree measures the
manifest schema rather than codegen. Each probe instead builds only the compiler
at the candidate commit, then runs the corpus from a fresh checkout of `d141956f`
with its original goldens and `dep/std` at `ad7add30`. Harness, case source,
golden and corpus dependency are identical in every probe.

**`c2fbd635` is the first commit whose `o2` output stops matching the original
golden.** Its immediate predecessor `dd119caa` still matches, so both sides of the
transition were measured. `c2fbd635` rewrites `src/lang/me/pass/inline.mach`
(+196/-94) and adds `src/lang/me/ir/body.mach`, which is the body cloning the
inliner needs, and its message names the change.

The branch head produces byte-identical output to `c2fbd635` for this case, so no
second commit contributed. Every probe from `c2fbd635` forward returns the same
SHA-256 prefix `fa9d345a8598`, which is also the SHA-256 of the blessed golden.

| commit | position | verdict |
| --- | --- | --- |
| `fec48251` | 10 | matches the original golden |
| `dd119caa` | 30 | matches the original golden |
| `c2fbd635` | 31 | differs, `fa9d345a8598` |
| `5a5cbcd2` | 32 | differs, `fa9d345a8598` |
| `002eea5e` | 33 | differs, `fa9d345a8598` |
| `d141956f` | 70 | differs, `fa9d345a8598` |

`c2fbd635` also bumps `dep/std`, from `a2304886` to `ad7add30`, so the dependency
was ruled out separately. Rebuilding the `dd119caa` compiler against `ad7add30`
and rerunning the same probe still matches the original golden, SHA-256 prefix
`9e15a968b476`. The dependency bump changes nothing here; the compiler source
change does.

Two stretches of the range could not be probed. `52a53af8` and everything before
it declares no `vectorize` key under `[profile.debug]` and the bootstrap refuses
the manifest. Positions 7 to 9 and 17 to 25 pin `dep/std` commits that exist in no
local checkout and that the `mach-std` remote does not serve, so their compilers
cannot be built at all. Neither stretch touches the transition, which sits
entirely between two probed commits.

## Verification

Layer B was rerun for all seven targets against the blessed goldens using a
compiler built from the branch by the bootstrap. 628 cells pass and none fail.
The nine skips are the nine corpus cases SPIR-V does not cover.

| target | pass | fail | skip |
| --- | --- | --- | --- |
| aarch64-darwin | 91 | 0 | 0 |
| aarch64-linux | 91 | 0 | 0 |
| riscv64-linux | 91 | 0 | 0 |
| spirv | 82 | 0 | 9 |
| x86_64-darwin | 91 | 0 | 0 |
| x86_64-linux | 91 | 0 | 0 |
| x86_64-windows | 91 | 0 | 0 |
