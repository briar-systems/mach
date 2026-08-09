# The cross-compilation and linking suite

One question: **does mach produce a correct image for a target, and link it against
what the platform actually provides?**

That is the question a checksum cannot ask. The codegen corpus one directory up
executes cases and compares them against a C anchor, so it settles what the compiler
computes. Nothing it does looks at an import table, a GOT slot, a load command, a
section type, a `PT_GNU_RELRO` span, an archive member, or an object some other
toolchain produced.

```
test/link/
  run.sh                       driver entry point
  cases/<name>/                one case: mach.toml, case.conf, src/, its goldens
  lib/                         the driver, the producers, and the host C toolchain
```

## What earns a case here

A case belongs here only when its fact is one of these:

- **cross-compilation** — an image in a format this host cannot execute, built and
  read host-side. a PE import table, a Mach-O bind table, a raw flat image.
- **linking** — what the linker resolved, placed, deduplicated or refused. import
  attribution, GOT and IAT slots, archive member selection, `PT_GNU_RELRO`, the
  symbol table, a refusal that comes from the linker rather than the front end.
- **a foreign toolchain** — an object, archive or import library clang, gcc or
  llvm-ar produced. mach's own objects can be self-consistently wrong, and only
  something that does not share mach's opinion can say so.

Everything else is checked by building it and looking, or by a unit test, and never
enters this directory. A front-end diagnostic, a comptime fold, a reflection walk,
an inline-asm body and an optimizer decision are all observable on a developer's own
machine in seconds, and a case for one of them is maintenance with no return.

**Never a case that fetches over the network for anything but its declared
dependencies.** An upstream rename in a sibling repository turned every open pull
request red once already (#2831), and the case that did it could not be defended on
the day it broke.

## Running it

```
run.sh                          # every case, every leg this host serves
run.sh --leg <name>             # one leg (repeatable)
run.sh --case <name>            # one case (repeatable)
run.sh --deps float|pin         # resolve refs fresh (default), or from mach.lock
run.sh --bless                  # rewrite goldens, print diffs, never under CI
run.sh --matrix                 # print the coverage matrix from the last run
```

Nonzero exit on any failure. Two environment variables:

- `MACH_LINK_MACH` is the compiler under test, default the checkout's
  `out/<host>/debug/bin/mach`. The driver never falls back to a `mach` on `PATH`,
  which is usually an older release, and it prints the path and version it measured.
- `MACH_LINK_OUT` is where the run's matrix and dependency record go, default
  `test/link/out`. Cases still build into their own directories, because the output
  path is a manifest key rather than something the command line can redirect, so two
  invocations against one checkout are **not** independent. Use a second checkout.

## Legs and targets

The leg registry is `test/engines.conf`, shared with the corpus, so a target cannot
mean one thing to one suite and something else to the other.

A **leg** is a machine: a row that declares a runner and executes something. A
**target** is what a case builds. Those are separate axes because the whole point of
a cross-compilation case is that they differ — `pe-import-claim` runs on the
`x86_64-linux` leg and builds `x86_64-windows`, and the import table it reads is a
fact no Windows runner is needed to establish.

`engine none` rows (spirv, mos6502, riscv32) are targets and never legs. A link case
reaches them through `target:` in its `case.conf`.

## The case manifest

`case.conf`, one `key: value` per line, `#` opens a comment:

```
legs: x86_64-linux aarch64-linux     # machines this case runs on (default: all)
skip: x86_64-windows                 # a leg subtracted, with the reason in a comment
target: x86_64-windows               # the mach target to build (default: the leg)
profiles: debug release              # (default: both)
run: pe-imports                      # the producer (default: exec)
build-flags: --pie                   # extra flags for the build
self-host: linux-riscv64             # cross-build the compiler and let it compile the case
```

Every producer is catalogued in `lib/produce.sh`'s header, which says what each one
observes. `doc/design/test-observability.md` says what running a program
structurally *cannot* observe and maps each of those classes to the surface that
can. Read it before adding anything here.

`self-host` names a target in the **repo's own** `mach.toml`, which is a different
vocabulary from this suite's legs. It is written out rather than derived: the two
schemes agree on four of six names, so a derivation would be a rule with exceptions.

## Goldens

A golden is the recorded observable, diffed byte for byte. Which file a case uses
depends on what its producer observes:

- `expect.txt` — the observable is target-independent (`exec`, `relro-fault`,
  `debuginfo`, `varloc-fbreg`, `symtab`: their facts hold identically on every ELF
  ISA).
- `expect.<target>.txt` — the observable is format-specific, which is every
  structural producer.
- `expect.<profile>.txt` — the observable is a real function of the active
  profile's own codegen. Only `gdb-session` is, and only since #2779.

`run.sh --bless` rewrites them and prints the diff, and refuses to run when `CI` is
set: a golden nobody read is not a golden.

## Honesty mechanisms

- Every run writes a coverage matrix — case, leg, profile, target, engine, producer,
  result — naming the **engine** that produced each cell, so a qemu result can never
  be read as native ABI evidence. It is written as the run goes, so a run that dies
  partway still leaves what it covered. CI uploads it.
- A leg a case declares away appears as `SKIP` in that matrix rather than vanishing:
  a run a leg was never applicable to and a run it was quietly dropped from used to
  look identical (#2741).
- Before anything is built, the driver checks that every case declares the
  `[target.<t>]` block each leg it runs on will build, and — under `--deps pin` —
  that the repo's own `mach.lock` records every git dep any case declares. Both were
  separate scripts a workflow could stop calling, and #2353 and #2729 are each a year
  of a lane believed to be running that had never once started. They live inside the
  driver now so nothing can reach a build without them holding.
- A run that matched no case exits nonzero rather than reporting a clean sweep.

## qemu is not ABI evidence

The `engine` column decides how a case executes and there is no flag to override it:
a run that says it exercised a leg has to have exercised it.

qemu-user does not fully model kernel memory semantics. It does not fault an
`mprotect` over address space the loader never mapped, and its guest page size need
not match a real kernel's. RELRO and mprotect runtime behaviour is therefore proven
only by a native leg — #1885 was a 64K-aligned aarch64 image that faulted ENOMEM on a
native 4K-page kernel every qemu-aarch64 leg had reported green. The `aarch64-linux`
row is native for that reason.

## Dependency resolution

A case declares `ref = "branch/main"` for mach-std, which tracks its latest release.

- `--deps float` (default) resolves it fresh, which is what catches a downstream
  break on the pull request that would first trip over it. It resolves **once per
  run**, not once per case: before #2619, a suite spanning a mach-std merge compiled
  some cases against one standard library and some against another and still reported
  a single verdict.
- `--deps pin` resolves every case to the commit this repo's own `mach.lock`
  records — the one the compiler in the same checkout was built against — so a
  release-gate run is reproducible and a bisect over mach commits is sound (#2592).

Both modes write `out/deps.txt` naming the commit every case actually compiled
against, because "what did the run that cut this tag build against" is asked after
the log has rotated.
