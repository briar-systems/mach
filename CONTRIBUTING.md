# Contributing to Mach

Thank you for your interest in contributing to Mach. Be respectful,
constructive, and professional: treat Mach like a passion project and its
community like family.

## What you need

- Git.
- A Mach 5 compiler. Mach is self-hosting, and the development source is Mach
  5 source pinned to std 2.0.0, which no 4.x release can read: a compiler
  must understand a new source form before its own source and standard
  library can adopt it, so a breaking syntax change cannot be built by the
  compiler that preceded it. Install a 5.0
  [release](https://github.com/briar-systems/mach/releases), or build one from
  the published 4.26.5 seed through the pinned bootstrap chain.

The chain is `.github/actions/setup-mach/bootstrap.py`, and `STAGES` in it is
the list of pins; CI runs it on every native host. The published 4.26.5
compiler builds a pinned bridge; the bridge builds the audited 4.30 source to
a fixpoint; that compiler builds the v5 migration stage to a fixpoint; the
stage compiler builds the development tree. Every source commit it names is
reachable from `main`, and no binary other than the 4.26.5 seed enters it.
The 4.30.0 public release and tag were withdrawn, so the version a stage
compiler prints is the version of its source, not evidence of a release.
Run the script, or transcribe its stages by hand: clone each pinned commit
with `--no-checkout`, check it out detached, initialize its submodule, and
build it with the previous stage's output using `--profile debug` (the old
seed's memory use stays within runner limits that way), comparing the last
two generations of each fixpoint stage byte for byte.

The standard library lives in its own repository
([mach-std](https://github.com/briar-systems/mach-std)). `[dep.std]` in
`mach.toml` declares its source and selector, `.gitmodules` records the
submodule, and the committed gitlink at `dep/std` pins the exact revision.
`mach dep pull .` checks that pin out in place; a build verifies it and never
fetches or moves it. `mach dep update` is the one command that moves a pin.
See [doc/language/manifest.md](doc/language/manifest.md#depid) and `mach help dep`.

## Building

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull .
mach build .
```

The compiler is written to `out/<target>/<profile>/bin/mach`, or
`bin/mach.exe` on Windows. A default Linux x86_64 build writes
`out/linux-x86_64/debug/bin/mach`.

The installed compiler is the seed. Generation A is built by the seed, B by
A, and C by B. B and C must be byte-identical; A may differ from B while the
seed carries an older code generator. Verify a compiler change from a
detached checkout of the committed tip, not from a working tree with
uncommitted edits.

## Checks

Run every check through the compiler you just built, never the seed on
`PATH`: an older release reads the tree differently or not at all.

| check | command |
| --- | --- |
| unit suite, both profiles | `out/<host>/debug/bin/mach test .` and `mach test . --profile release` |
| structural censuses | `sh test/census.sh` |
| formatting | `mach fmt --check .` (the tree is canonical; run it before every pull request) |
| reference agrees with the binary | `MACH_DOC_MACH=<abs path> python3 test/doc-agreement.py` |
| language examples compile | `MACH_DOC_MACH=<abs path> python3 test/doc-examples.py` |
| changelog shape | `bash .github/scripts/check-changelog.sh` |
| codegen corpus and link suite | `test/run.sh` and `test/link/run.sh`; see [test/README.md](test/README.md) |

`doc-agreement.py` holds `doc/language/manifest.md`, the `mach init`
scaffolds and the grammar's keyword list to the compiler under test, and
`doc-examples.py` compiles every exercised example of the language reference.
Both read `MACH_DOC_MACH` or the checkout's `out/<host>/debug/bin/mach`. CI
runs all of them on every pull request.

## Branching

- `main` holds tagged releases only. It takes integration merges from `dev`.
- `dev` is the integration branch and the target of every pull request.
- `feat/<issue>` and `fix/<issue>` branch off `dev` and return to it.
- `hotfix/<issue>` branches off `main` and merges into both `main` and `dev`.

```bash
git checkout dev
git pull origin dev
git checkout -b feat/1234
```

## Commits

Commits are small, self-contained, and conventional. The issue number is the
scope:

```
fix(#1234): brief description

Longer explanation if needed.
```

The types are `feat`, `fix`, `docs`, `refactor`, `test`, `chore` and
`style`. A change with no issue uses `chore: ...` with no scope. Never add a
`Co-Authored-By` trailer.

## Pull requests

- Open as a draft targeting `dev`; mark it ready when it is done.
- Link the issue with `Closes #N`.
- Put the verification evidence in the body: the fixpoint, the suite counts
  in both profiles, and every check above that the change touches.
- Merge with a merge commit. Never rebase or fast-forward.
- When the target is not the default branch, close the linked issue by hand
  after the merge.

## Code

Follow the existing patterns and the [language reference](doc/language/README.md).
Fix the actual cause of a defect rather than its symptom, and keep the
structure right: clear ownership of state, data flowing one direction, and a
contract at every axis the compiler grows along.

## Documentation

Every fact has one home. A docstring on a declaration says what it is and
how it is used, and a module's docstring states the contract the module
holds its callers to; the language reference under `doc/language/` (the
language, `mach.toml` and the docstring form) says what the user sees; and
`CHANGELOG.md` says when something changed. `doc/` holds nothing else:
`mach --help` and `mach help <command>` are the command-line reference, and
`mach doc .` renders the docstrings to `doc/api/`, which is generated and
never committed. The docstring form is in
[doc/language/documentation.md](doc/language/documentation.md).

Docstring coverage is a property of the supported surface, not a quota. The
supported, source-stable surface is the editor API (`mach.lang.editor`), the
command line and the manifest schema; every `pub` declaration of that
surface carries a docstring stating its ownership, lifetime and error
contract (who frees what, how long a borrowed product stays valid, which
outcome case means what). Everything else under `src/` is internal: a `pub`
there is documented when its contract is not evident from its signature and
its callers, and a comment explains a genuine invariant, never restates the
line below it. No check counts docstrings; `mach doc .` renders whatever is
there.

## Issues

File issues through the templates. Each template names the sidebar fields a
template cannot set: the type label (bug, feature, chore, problem, rfc,
epic), one or more `area:*` labels, a `target:*` label when the work is
target-specific, and the milestone. A bug report carries a minimal
reproduction, the compiler version, the OS and the exact diagnostic. A
feature request carries concrete use cases and says whether a library could
do it instead.

## Versioning

Mach uses [semantic versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):
MAJOR for breaking changes to the supported compiler surface, MINOR for
backward-compatible additions, PATCH for fixes, documentation and internal
improvements. The standard library is versioned separately in its own
repository.

A release bump updates both `[project].version` in `mach.toml` and
`MACH_VERSION` in `src/lang/version.mach`; CI and the tag workflow require
the two to match. Tags are created on `main` after the integration merge
from `dev`.

## Project structure

```
mach/
├── dep/std/           # standard library, pinned by a committed gitlink
├── doc/language/      # the language reference, mach.toml included
├── src/               # the self-hosting compiler
├── test/              # censuses, the codegen corpus, the link suite, doc checks
├── out/               # ignored build output, grouped by target and profile
└── mach.toml          # project manifest
```

The original bootstrap seed ([mach-boot](https://github.com/briar-systems/mach-boot))
is no longer part of the build. It remains only as a from-scratch cold-start
hatch.

## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE).
