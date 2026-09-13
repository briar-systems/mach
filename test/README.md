# test

`mach test .` is the unit suite. Everything here proves correctness at the codegen
and image level against something external: `llvm-objdump`, the host C compiler,
qemu, a real linker and loader, `spirv-val`, `llvm-dwarfdump`.

```
test/
  run.sh                        the one driver
  cases/<group>/<case>.mach     one codegen case, target-independent
  ref/<group>/<case>.c          its C reference
  golden/<target>/<g>/<c>.dis   blessed external-decoder text
  golden/<target>/SKIPS         cases that target cannot serve, one per line
  lib/fold.mach, lib/corpus.h   the checksum fold both sides use
  link/cases/<name>/            one link case: a project, case.conf, expect*.txt
  link/check/                   the shared image readers
  fuzz/corpus/                  inputs replayed by the unit suite (see fuzz/README.md)
  out/                          build products, gitignored
```

## Running it

```
bash test/run.sh                          # every golden, plus the differential this host can execute
bash test/run.sh --target x86_64-linux    # one target (repeatable)
bash test/run.sh --case bits/logic_u32    # one case (repeatable)
bash test/run.sh --qemu                   # also execute riscv64-linux under qemu-riscv64
bash test/run.sh --dwarf                  # also build every case with -g and run llvm-dwarfdump --verify
bash test/run.sh --link [--qemu]          # the link cases instead of the corpus (--case <name> selects one)
bash test/run.sh --bless [...]            # write the goldens or expect files instead of diffing, print the diff
```

`MACH` names the compiler under test; the default is the checkout's
`out/<host>/debug/bin/mach` and the run prints what it used. Exit is nonzero on
any failure, one `FAIL` line per cell and a one-line total.

Per case and target the driver builds the object in release, decodes it with
`llvm-objdump -d --no-leading-addr --no-show-raw-insn --symbolize-operands`
(spirv: `spirv-val`, then `spirv-dis --no-color --no-indent`) and diffs the text
against the golden. On a target the host executes it also builds at O0 and O2,
runs both, and compares the checksums with the C reference built by `cc` at
`-O0`, `-O2` and under UBSan, which must agree among themselves first. The tool
versions the goldens were blessed with are stated at the top of `run.sh`.

## The case contract

A case is one file, `cases/<group>/<case>.mach`.

1. **One public function.** `pub fun checksum(seed: u64) u64`. The case defines
   no entry point, prints nothing, and reaches no standard library. The driver
   owns the entry: on a hosted target it wraps the case in one that prints the
   checksum, and on spirv or riscv32 it compiles the case file directly.
2. **Target-independent source.** No target conditionals, no inline asm, no
   OS-specific calls. The same source compiles for every target, or is named in
   that target's `SKIPS` with a reason.
3. **Deterministic.** No input, no time, no address observed as a value, no
   float printing. Terminates in well under a second on the slowest engine.
4. **Folds every intermediate with FNV-1a** through `corpus.lib.fold`
   (`init`, `mix_u8`..`mix_u64`, `mix_i8`..`mix_i64`, `mix_f32`, `mix_f64`).
   Floats fold by bit pattern, never by value, so NaN and `-0.0` are exact. Fold
   in a fixed order so a wrong lane or element changes the checksum instead of
   cancelling out.
5. **Operands stay inside mach's defined semantics and the defined-behaviour C
   mapping.** No case may depend on behaviour that is UB in the C translation;
   the UBSan reference build enforces it.
6. **Everything that matters flows through the opaque `seed` at least once.**
   `seed` is `argc - 1`, unknowable at compile time, so the optimizer cannot fold
   the case to a constant. The `comptime` group deliberately does the opposite.

`ref/<group>/<case>.c` is the line-for-line translation: `uint64_t
checksum(uint64_t seed)` including `corpus.h`, all arithmetic on unsigned
fixed-width types with an explicit cast at every width change, signed operations
through two's-complement identities.

The `vec/rows_*` cases are the vector rows: one `#[noinline]` probe per operation,
signedness and predicate of each lane width, so the golden shows each probe's
body on its own and witnesses which cells the target packs and which it expands
per lane.

## Adding a case

1. Write `cases/<group>/<name>.mach` to the contract and `ref/<group>/<name>.c`.
2. `bash test/run.sh --case <group>/<name> --target <host>` until the four
   checksums agree (build failures and disagreements print as `FAIL` lines).
3. `bash test/run.sh --bless --case <group>/<name>` and read every golden it
   writes. A golden you have not read is not a golden.
4. A target that cannot serve the case gets a line in `golden/<target>/SKIPS`:
   the case name, then the reason.

## Link cases

A link case asks what a checksum cannot: does mach produce a correct image and
link it against what the platform provides. Its fact is a cross-compiled format
read host-side (a PE import table, a Mach-O bind table, a raw image), what the
linker resolved or refused, or an object a foreign toolchain produced. Anything
observable by building and looking on a developer's machine does not belong
here.

`link/cases/<name>/` is a project (`mach.toml`, `src/`, any fixtures) plus:

- `case.conf`, one `key: value` per line, `#` a comment:
  ```
  legs: x86_64-linux aarch64-linux   # machines the case runs on (default: every leg this host serves)
  skip: x86_64-windows               # a leg subtracted, the reason in a comment
  target: x86_64-windows             # the mach target to build (default: the leg)
  profiles: debug release            # (default: both)
  run: pe-imports                    # exec (default), built, build-fails, or a reader in link/check/
  build-flags: --pie
  gbuild: yes                        # also build the -g twin, handed to the check
  self-host: linux-riscv64           # cross-build the compiler for the leg and let it compile the case
  ```
- `check.sh`, when the case reads its own image: invoked as
  `check.sh <engine> <leg> <binary> [<g-binary>] [<profile>]` with `engine`
  `native` or `qemu:<command>`, sourcing `../../check/common.sh` for the shared
  readers; its stdout is the observable. A case with no `check.sh` uses the
  reader `run:` names under `link/check/`, or the built-in modes: `exec` runs
  the program and records its stdout, `built` records that an artifact was
  emitted, `build-fails` records the compiler's `error:` lines.
- the expected output, the most specific of `expect.<target>.<profile>.txt`,
  `expect.<profile>.txt`, `expect.<target>.txt`, `expect.txt`. Create the file
  you want, then `--bless` fills it.

The standard library a case declares is the checkout's own `dep/std`, so a run
tests the compiler against the library it was built with and a bisect over mach
commits is sound. A `[step]` that compiles C runs `../../cc.sh`, which picks the
host `cc` when it targets the leg and a cross-capable clang when it does not.

qemu is compute evidence, never ABI evidence: RELRO and page-size behaviour is
proven only by a native leg.
