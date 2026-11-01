# Contributing to Mach

Thank you for your interest in contributing to Mach. Be respectful,
constructive, and professional: treat Mach like a passion project and its
community like family.


## Building

Mach is self-hosting, so building it needs an existing Mach compiler. Install
the latest [release](https://github.com/briar-systems/mach/releases) for your
host and put `mach` on `PATH`; CI seeds from the same archive
(`.github/scripts/bootstrap.sh`, see [Fixpoint](#fixpoint)).

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
bash .github/scripts/seed-build.sh mach a
./a dep pull .
./a build .
```

A 6.x release reads only the previous manifest keys, so it builds `a` through
`seed-build.sh` (see [Fixpoint](#fixpoint)), and `a` builds the compiler. It is
written to `out/<target>/<profile>/bin/mach`, or `bin/mach.exe` on Windows. A
default Linux x86_64 build writes `out/linux-x86_64/debug/bin/mach`.


## Fixpoint

A change to the compiler has to reach the self-host fixpoint: the compiler it
builds must build itself byte for byte. CI does not check it, so run it
locally. `.github/scripts/bootstrap.sh` is the one way: it reads `RUNNER_OS`
and `RUNNER_TEMP` and writes `MACH` to the file `GITHUB_ENV` names, so a local
run names them. From the repository root:

```bash
mkdir -p out/boot
RUNNER_OS=Linux RUNNER_TEMP=$PWD/out/boot GITHUB_ENV=$PWD/out/boot/env \
    bash .github/scripts/bootstrap.sh --fixpoint
```

The script installs the seed release named on the `seed` line of
`.github/bootstrap-chain`, never whichever `mach` is on `PATH`, and walks the
chain through each pinned mach commit. The last step builds `a` from your tree,
`a` builds `b`, and `b` builds `c`. It fails when `cmp b c` differs and
otherwise leaves `c` as the compiler, with its path in `out/boot/env`. Use `RUNNER_OS=macOS` or
`Windows` on those hosts (the outputs are then `a.exe`, `b.exe` and `c.exe` on
Windows), and `--profile release` for the release fixpoint. Without
`--fixpoint` it stops at `b`.

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

The compiler carries no tests of its own: no unit `test` blocks, no goldens
and no corpus. `mach test` and the language's `test` blocks are features for
the projects Mach builds. CI is the build: it walks the bootstrap chain and
builds the compiler with itself, and nothing else.

The tree is canonical: `mach fmt .` must leave it unchanged before a pull
request is opened (`mach fmt --check .` reports what differs).

The API reference is held the same way: `doc/mach` and `doc/README.md` are what `mach doc .`
writes, together with the pages `mach doc . -a editor` adds for the editor library, whose
index entries follow the compiler's. A change to a doc-comment or to the module tree
regenerates them in the same pull request.

### Local checks

Run from the repository root with the compiler you built (`$MACH`, the path in
`out/boot/env`, or `out/<host>/debug/bin/mach`). Each command exits non-zero on
failure.

| check | command |
|---|---|
| formatter | `"$MACH" fmt --check .` |
| committed reference | `"$MACH" doc . -o out/docgen -q && "$MACH" doc . -a editor -o out/docgen -q`, then compare `out/docgen/mach` with `doc/mach` |
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
- Say briefly what changed. Every pull request builds the compiler through the
  bootstrap chain on the linux runners. The windows, darwin and release lanes
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

The standard library is versioned separately in its own repository. SemVer impact is tracked directly on issues and pull requests via conventional commit types and issue tags. A release bump updates both `[project].version` in `mach.toml` and `MACH_VERSION` in `src/lang/version.mach`. Tags are created on `main` after the integration merge from `dev`.


## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE).
