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


## Testing and formatting

Run the tests and the formatter through the compiler you just built, never
the seed on `PATH`:

```bash
out/linux-x86_64/debug/bin/mach test .
out/linux-x86_64/debug/bin/mach test . --profile release
out/linux-x86_64/debug/bin/mach fmt .
```

The tree is canonical: `mach fmt .` must leave it unchanged before a pull
request is opened (`mach fmt --check .` reports what differs).

`bash test/run.sh` runs the codegen corpus against the external decoders and
the C reference, and `bash test/run.sh --link` the link cases; see
[test/README.md](test/README.md).


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

The types are `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, and
`style`. A change with no issue uses `chore: ...` with no scope.

If no related issue exists, just supply a type without the scope, e.g:

```
chore: update dependencies
```


## Pull requests

- Open as a draft targeting `dev`; mark it ready when it is done.
- Link the issue with `Closes #N`.
- Add a line to the `## [Unreleased]` section of `CHANGELOG.md` under the
  matching heading (`Added`, `Changed`, `Fixed`, `Removed`) for anything a
  user can observe.
- Put the verification evidence in the body: the fixpoint and the test
  counts in both profiles.
- Merge with a merge commit. Never rebase or fast-forward.
- When the target is not the default branch, close the linked issue by hand
  after the merge.


## Issues

File issues through the templates. Each template names the sidebar fields a
template cannot set: the type label, the `area:*` labels, a `target:*` label
when the work is target-specific, and the milestone.


## Versioning

Mach uses [semantic versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):
MAJOR for breaking changes to the supported compiler surface, MINOR for
backward-compatible additions, PATCH for fixes, documentation, and internal
improvements. The standard library is versioned separately in its own
repository.

A release bump updates both `[project].version` in `mach.toml` and
`MACH_VERSION` in `src/lang/version.mach`; CI and the tag workflow require
the two to match. Tags are created on `main` after the integration merge
from `dev`.


## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE).
