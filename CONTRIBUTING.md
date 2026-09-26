# Contributing to Mach

Thank you for your interest in contributing to Mach. Be respectful,
constructive, and professional: treat Mach like a passion project and its
community like family.


## Building

Mach is self-hosting, so building it needs an existing Mach compiler. Install
the latest [release](https://github.com/briar-systems/mach/releases) for your
host and put `mach` on `PATH`; CI seeds from the same archive
(`.github/actions/seed-mach`).

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull .
mach build .
```

The compiler is written to `out/<target>/<profile>/bin/mach`, or
`bin/mach.exe` on Windows. A default Linux x86_64 build writes
`out/linux-x86_64/debug/bin/mach`.


## Fixpoint

A change to the compiler has to reach the self-host fixpoint: the compiler it
builds must build itself byte for byte. The stages start from a seed, a
published release fetched and checked against the release's `SHA256SUMS`, as
CI does in `.github/actions/seed-mach`. The seed is the tag that action pins
(its `default: v...` line), not whichever `mach` happens to be on `PATH`. From
the repository root on x86_64 Linux:

```bash
v=$(sed -n 's/^ *default: v//p' .github/actions/seed-mach/action.yml)
t=x86_64-linux
gh release download "v$v" -R briar-systems/mach -p "mach-$v-$t.tar.gz" -p SHA256SUMS -D ../mach-seed
(cd ../mach-seed && grep " mach-$v-$t.tar.gz\$" SHA256SUMS | sha256sum -c - && tar -xzf "mach-$v-$t.tar.gz" mach)
../mach-seed/mach dep pull .
../mach-seed/mach build . --bin mach -o a
./a build . -o b
./b build . -o c
cmp b c
```

The seed builds `a` from your tree. `a` builds `b`, and `b` builds `c`. `cmp`
prints nothing and exits 0 when `b` and `c` are identical. CI runs the
fixpoint on every pull request that touches the compiler, so run it locally
only when a change needs it. The seed builds the `mach` binary and nothing
else. Run the unit suites with `c`, as in [Testing and
formatting](#testing-and-formatting), because the test code may use language
the seed release predates. A release newer than the pin builds `a` too, and an
older one is not supported. On macOS use `aarch64-darwin` or `x86_64-darwin`
and `shasum -a 256 -c`. On Windows the archive is `mach-$v-x86_64-windows.zip`
holding `mach.exe`, the seed builds `--bin mach-windows`, and the outputs are
`a.exe`, `b.exe` and `c.exe`. Add `--profile release` to every build for the
release fixpoint.

`-o` names a canonical path inside the project root: relative, `/`-separated,
with no `.` or `..` component. `-o ../a`, `-o ./a` and an absolute path are
refused with `-o must name a canonical path inside the project root`, so keep
the stage outputs in the checkout. `.gitignore` covers `a`, `b` and `c`.


## Testing and formatting

Run the tests and the formatter through the compiler you just built, never
the seed on `PATH`:

```bash
out/linux-x86_64/debug/bin/mach test .
out/linux-x86_64/debug/bin/mach test . --profile release
out/linux-x86_64/debug/bin/mach test . --lib tests
out/linux-x86_64/debug/bin/mach test . --lib tests --profile release
out/linux-x86_64/debug/bin/mach fmt .
```

`mach test .` runs the tests in the compiler's own closure. The suites that
live in modules the compiler never reaches (`src/lang/driver/tests/`, the
codegen runtime probes and the rest) are reached by the `tests` library
artifact, whose entry `src/lib/tests.mach` `use`s each of them, so a new
test-only module is added there.

The tree is canonical: `mach fmt .` must leave it unchanged before a pull
request is opened (`mach fmt --check .` reports what differs). The same holds
for the API reference: `doc/mach` and `doc/README.md` are what `mach doc .`
writes, so a change to a doc-comment or to the module tree regenerates them in
the same pull request, and CI fails when the committed pages differ from a
fresh generation.

`bash test/run.sh` runs the codegen corpus, a differential of each case against
the same program written in C, and `bash test/run.sh --link` the link cases; see
[test/README.md](test/README.md).

### Test policy

What earns a test, and where it sits, is set by the
[test policy](doc/language/test.md#test-policy).


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

If no related issue exists, just supply a type without the scope, e.g:

```
chore: update dependencies
```


## Pull requests

- Open as a draft targeting `dev`; mark it ready when it is done.
- Link the issue with `Closes #N`.
- Leave `CHANGELOG.md` alone. The changelog is written from the merged
  commits when `dev` is released to `main`.
- Say briefly what changed and which tests you ran. CI runs the legs the
  changed paths select (`.github/scripts/ci-legs.sh`), and the pull request
  from `dev` to `main` runs everything.
- Merge with a merge commit. Never rebase or fast-forward.
- When the target is not the default branch, close the linked issue by hand
  after the merge.


## Issues

File issues through the templates. Issues use an orthogonal, faceted tagging system across five sets:

- SemVer magnitude: `patch`, `minor`, `major`
- Kind of work: `feature`, `fix`, `removal`, `chore`, `performance`
- Where (domain or location): `testing`, `tooling`, `doc` (omitted when touching core compiler code)
- Severity and state: `critical`, `blocked`, `security`
- Discussion: `discussion` (design proposals, RFCs, and open debates)

Tags mix and match across sets (for example, `patch`, `fix`, `tooling`). When opening an issue, select the applicable tags in the sidebar. Milestones record where an issue stands in the current plan and change freely, so the list on GitHub is the reference. There are three: `active` is the current slice, in flight or queued next, `deferred` is planned for after it, and `parked` is deliberately set aside until something changes. An issue with no milestone is backlog. An issue moves to `active` when work on it starts. `blocked` is a tag that records state, never a milestone. Themes are epic issues with native sub-issues. SemVer impact comes from the `major`, `minor` and `patch` tags, never from a milestone.


## Versioning

Mach adheres strictly to [semantic versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):
- `MAJOR`: breaking changes to the language grammar, compiler interface, or supported runtime contracts.
- `MINOR`: backward-compatible new language features, compiler flags, target additions, and optimizations.
- `PATCH`: backward-compatible bug fixes, documentation, and internal refactors.

The standard library is versioned separately in its own repository. SemVer impact is tracked directly on issues and pull requests via conventional commit types and issue tags. A release bump updates both `[project].version` in `mach.toml` and `MACH_VERSION` in `src/lang/version.mach`. Tags are created on `main` after the integration merge from `dev`.


## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE).
