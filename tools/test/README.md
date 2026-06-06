# Compiler test harnesses

Two scripts that stress the Mach compiler beyond the unit tests: a differential
harness that catches optimization miscompiles, and a fuzzer that catches
compiler crashes. Both are pure shell plus a small `awk` generator and depend
only on a built compiler (`make`) and coreutils.

Run everything from the repository root after `make`.

## `differential.sh` — optimization-level differential testing

For every runnable program in the corpus (the numbered `examples/`), the harness
compiles and runs it at `-O0` and at `-O2`, then asserts the program's **exit
code and stdout are identical** across the two levels. Any divergence is an
optimization miscompile: the program's observable behavior must not depend on
the optimization level.

It optionally also diffs the same programs compiled by `smach` against `mach` at
`-O0` (a cross-compiler check that the bootstrap's last two stages agree).

```sh
tools/test/differential.sh             # full run (both checks)
tools/test/differential.sh --no-smach  # only the -O0 vs -O2 check
tools/test/differential.sh --examples path/to/corpus
```

Output is a per-input PASS/FAIL table. On a `FAIL` the script prints the
diverging exit codes and, when stdout differs, both outputs side by side. The
script exits `0` only if every input is consistent, `1` on any mismatch or build
failure, `2` on a setup error.

Each example is copied (with symlinks dereferenced) into a throwaway temp dir
before building, so `examples/` is never written to.

## `fuzz.sh` + `gen.awk` — crash fuzzing

`gen.awk` is a deterministic generator of small, mostly-valid Mach programs. It
emits a `main` returning `i64` built from a constrained grammar of **safe**
constructs: `i64` locals, integer arithmetic, `if {} or {}` chains, bounded
`for` loops, a `rec` with field access, and a helper function call. Safety guards
(non-zero constant divisors, statically-bounded loops, declare-before-use) keep
programs valid and runtime-safe so the focus stays on the compiler.

Randomness is a seeded Park–Miller LCG, so a given seed always produces the same
program:

```sh
awk -v seed=42 -f tools/test/gen.awk      # print one program for seed 42
```

`fuzz.sh` generates `N` programs (iteration `i` uses `seed = base + i`) and
compiles each with `mach`. It is looking for **compiler crashes** — an internal
error abort (exit `2`) or a fatal signal (segfault / exit `>= 128`) — not for
every random program to compile. Exit `0` (compiled) and exit `1` (rejected with
a normal diagnostic) are both expected, healthy outcomes.

```sh
tools/test/fuzz.sh                       # 100 iterations, random base seed
tools/test/fuzz.sh -n 500 -s 1000        # 500 iterations from a fixed seed
tools/test/fuzz.sh -n 200 --keep         # also keep non-crashing sources
```

A clean run reports `no compiler crashes found` and exits `0`. When a crash is
found the script saves the offending source and the compiler's stderr under
`tmp/fuzz-crashes/` (override with `--out`) and exits `1`. Because the base seed
is fixed and reported, any crash reproduces exactly:

```sh
awk -v seed=<S> -f tools/test/gen.awk      # regenerate the crashing program
```

### Seeding notes

`$RANDOM` (the bash default base seed) is fine for ad-hoc runs, but pass an
explicit `-s` for reproducible CI runs. The generator avoids `Math.random` /
floating point entirely so it behaves identically across sandboxes.

## Interpreting results

- **differential FAIL** — a real `-O0` vs `-O2` divergence (or `smach` vs `mach`
  divergence). This is a miscompile or a pass that breaks the IR; file it.
- **fuzz CRASH** — the compiler aborted or faulted on a mostly-valid program.
  The saved `crash_seed_<S>.mach` is a reproducer; file it.

Neither harness modifies the compiler, `mach.toml`, `dep/mach-std`, or
`examples/`; all builds happen in temp dirs that are cleaned up on exit.
