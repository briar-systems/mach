# Contributing to Mach

Thank you for your interest in contributing to Mach. Be respectful,
constructive, and professional: treat Mach like a passion project and its
community like family.

[`doc/project.md`](doc/project.md) describes how the source is laid out and
where a new module goes.


## Building

Mach is self-hosting, so building it needs an existing Mach compiler. Install
the latest [release](https://github.com/briar-systems/mach/releases) for your
host and put `mach` on `PATH`. CI seeds from the release named by `SEED` at the
top of `.github/workflows/ci.yml` and `cd.yml` (see [Fixpoint](#fixpoint)).

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull .
mach build . -o a
./a build .
```

The release builds `a`, and `a` builds the compiler, so the compiler you run
comes from this tree's own code generator. It is written to
`out/<target>/<profile>/bin/mach`, or `bin/mach.exe` on Windows. A default
Linux x86_64 build writes `out/linux-x86_64/debug/bin/mach`.


## Fixpoint

A change to the compiler has to reach the self-host fixpoint: the compiler it
builds must build itself byte for byte. CI does not check it, so run it
locally from the repository root:

```bash
seed=$(sed -n 's/^  SEED: //p' .github/workflows/ci.yml)
MACH_VERSION=$seed MACH_INSTALL_DIR=$PWD/out/seed sh dist/install.sh
out/seed/mach dep pull .
out/seed/mach build . --profile debug -o a
./a dep pull .
./a build . --profile release --threads 2 -o b
./b build . --profile release --threads 3 -o c
cmp b c
```

These are the steps CI runs, plus `c`. The seed is the release `SEED` names,
never whichever `mach` is on `PATH`, and the install script checks its archive
against the release's `SHA256SUMS`. The seed builds `a` at the debug profile,
so no older compiler's optimiser compiles your tree. `a` builds `b` on two
threads and `b` builds `c` on three, so the fixpoint also holds the output to
being the same at every thread count. When `cmp` finds no difference, `c` is
the compiler. Use `--profile debug` for `b` and `c` to check the debug
fixpoint.

On Windows, install the seed with `pwsh -NoProfile -File dist/install.ps1` and
the same two variables, and name the outputs `a.exe`, `b.exe` and `c.exe`. On
a Mac, run in a native shell: Rosetta 2 cannot host the compiler's worker
threads.

The compiler's own source uses only what the seed reads. A language feature
reaches it once a release ships the feature and `SEED` moves to that release
in both workflows.

`-o` names a canonical path inside the project root: relative, `/`-separated,
with no `.` or `..` component. `-o ../a`, `-o ./a` and an absolute path are
refused with `-o must name a canonical path inside the project root`, so keep
the stage outputs in the checkout. `.gitignore` covers `a`, `b` and `c`.


## Formatting and checks

Run the formatter and the checks through the compiler you just built, never
the seed on `PATH`:

```bash
out/linux-x86_64/debug/bin/mach fmt .
out/linux-x86_64/debug/bin/mach check .
```

The compiler's tests are a corpus under `test/`, one program a file, with no
project around any of them but those under `project/`. A file under `run/` builds for every target and
exits 0 where the host can run it. A file under `refuse/` is refused with the
diagnostic kind its first line names. A file under `gpu/` builds for the shader
targets. A directory under `project/` is a small project and its `steps` file,
the commands run in order in one fresh work directory, each with the exit code it
expects and, after `|`, text a line of its output must hold, for what only a
sequence of commands shows. Each file under `target/` names a target's flags and states what
`mink dump` must show of every artifact built for it. One driver runs it all
through the compiler you just built, with the `mink` on `PATH` unless `--mink`
names another:

```bash
python3 test/run.py --mach out/linux-x86_64/debug/bin/mach
```

The compiler carries no unit `test` blocks and no goldens. `mach test` and the
language's `test` blocks are features for the projects Mach builds. CI is the
build: it builds the compiler from the seed release and then with itself, and
nothing else.

The tree is canonical: `mach fmt .` must leave it unchanged before a pull
request is opened (`mach fmt --check .` reports what differs).

The API reference is held the same way: `doc/mach` is what `mach doc .` writes, in one run
over every module either artifact reaches, the editor library's included. A change to a
doc-comment or to the module tree regenerates it in the same pull request.

### Local checks

Run from the repository root with the compiler you built (`$MACH`, the path in
`out/boot/env`, or `out/<host>/debug/bin/mach`). Each command exits non-zero on
failure.

| check | command |
|---|---|
| formatter | `"$MACH" fmt --check .` |
| committed reference | `"$MACH" doc . -o out/docgen -q`, then compare `out/docgen/mach` with `doc/mach`: every page and nothing else |
| editor library | `"$MACH" check . -a editor` |
| documented flags are accepted | `python3 .github/scripts/doc-flags.py "$MACH" doc` |
| doc and source markers cite open issues | `GH_TOKEN=$(gh auth token) GH_REPO=briar-systems/mach python3 .github/scripts/doc-markers.py` |
| cross-build the compiler | `"$MACH" build . --target darwin-x86_64 --profile release -o out/cross-darwin-x86_64` (also `darwin-aarch64`, `linux-riscv64`) |
| fixpoint | see [Fixpoint](#fixpoint) |


## Branching

- `main` holds tagged releases only. It takes integration merges from `dev`.
- `dev` is the integration branch and the target of every pull request.
- `feat/<issue>` and `fix/<issue>` branch off `dev` and return to it.
- `hotfix/<issue>` branches off `main` and merges into both `main` and `dev`.


## Commits

Commits are small, self-contained, and conventional. The issue number is the
scope:

```
fix(#1234): brief description

Longer explanation if needed.
```

The types are `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, `style`,
`ci`, and `perf`. A breaking change marks the type with `!` before the colon:
`feat(#139)!: brief description`. A change with no issue uses `chore: ...` with
no scope, and a release commit is `chore(release): <version>`.

For example, `chore: update dependencies`.


## Pull requests

- Open as a draft targeting `dev`; mark it ready when it is done.
- Link the issue with `Closes #N`.
- Leave `CHANGELOG.md` alone. The changelog is written from the merged
  commits when `dev` is released to `main`.
- Say briefly what changed. Every pull request builds the compiler from the
  seed release on the linux runners. The windows, darwin and release lanes
  run when the CI workflow is dispatched with a `lane`, which is worth doing
  before a windows or darwin change reaches `dev`, and `lane=all` runs green on
  `dev` before every release. Every other check is local, see
  [Local checks](#local-checks).
- Merge with a merge commit. Never rebase or fast-forward.
- When the target is not the default branch, close the linked issue by hand
  after the merge.


## Issues

File issues through the templates. Issues use an orthogonal, faceted tagging system across six sets:

- SemVer magnitude: `patch`, `minor`, `major`
- Kind of work: `feature`, `fix`, `removal`, `chore`, `performance`
- Where (domain or location): `testing`, `tooling`, `doc` (omitted when touching core compiler code)
- Severity and state: `critical`, `blocked`, `parked`, `security`
- Discussion: `discussion` (design proposals, RFCs, and open debates)

Tags mix and match across sets (for example, `patch`, `fix`, `tooling`). When opening an issue, select the applicable tags in the sidebar. There are no milestones. Work in flight is an open draft pull request, `parked` marks an issue deliberately set aside until something changes, and every other open issue is backlog. `blocked` is a tag that records state. Themes are epic issues with native sub-issues. SemVer impact comes from the `major`, `minor` and `patch` tags, never from a milestone.


## Versioning

Mach adheres strictly to [semantic versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):
- `MAJOR`: breaking changes to the language grammar, compiler interface, or supported runtime contracts.
- `MINOR`: backward-compatible new language features, compiler flags, target additions, and optimizations.
- `PATCH`: backward-compatible bug fixes, documentation, and internal refactors.

The standard library is versioned separately in its own repository. SemVer impact is tracked directly on issues and pull requests via conventional commit types and issue tags. A release bump updates both `[project].version` in `mach.toml` and `MACH_VERSION` in `src/base/version.mach`. Tags are created on `main` after the integration merge from `dev`.


## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE).
