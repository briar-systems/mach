# Contributing to Mach

Thank you for your interest in contributing to Mach!

---

## Code of Conduct

Be respectful, constructive, and professional. Treat Mach like a passion project and its community like family.

---

## Getting Started

### Prerequisites

- Git
- An existing `mach` binary. Mach is self-hosting, so building from source needs one. Install the latest [release](https://github.com/briar-systems/mach/releases).

### Building

Mach builds its own source with an existing `mach`:

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull
mach build .
```

The compiler is written to `out/<target>/<profile>/bin/mach`, or `bin/mach.exe`
on Windows. A default Linux x86_64 build writes
`out/linux-x86_64/debug/bin/mach`.

The installed compiler is the seed. Build generation A with the seed, B with
A, and C with B. Release verification requires B and C to be byte-identical.
A can differ from B while the installed seed carries an older code generator.
See the [release shape](doc/design/release-shape.md) for the seed transition.

---

## Branching Strategy

We use a two-branch model:

- **`main`**: Stable branch. Tagged releases only.
- **`dev`**: Development branch. Integration point for features and fixes.

Features and fixes branch off `dev` and return through a pull request. Releases integrate `dev` into `main` and tag the merge commit. A hotfix branches off `main` and merges into both long-lived branches.

### Branch Naming

- `feat/<issue>`: New features
- `fix/<issue>`: Fixes, including documentation corrections
- `hotfix/<issue>`: Urgent release fixes

### Contributing Changes

1. **Fork the repository**
2. **Create a feature branch from `dev`:**
   ```bash
   git checkout dev
   git pull origin dev
   git checkout -b feat/1234
   ```
3. **Make your changes** following the coding standards below
4. **Make small, self-contained conventional commits:**
   ```
   fix(component): brief description

   Longer explanation if needed.
   ```
5. **Push to your fork:**
   ```bash
   git push origin feat/1234
   ```
6. **Open a draft Pull Request** targeting `dev`, then mark it ready when complete

Replace `1234` with the issue number. Use `fix/1234` for a fix.

**Pull Request Guidelines:**

- Target `dev` branch (not `main`)
- Provide clear description of changes
- Link the issue with `Closes #N`
- Verify compiler changes from a detached checkout of the committed tip
- Ensure the compiler builds and reaches the B/C byte-identical fixpoint
- Run the built compiler's test suite in both debug and release profiles
- Follow coding standards
- Merge with a merge commit, without rebasing or fast-forwarding
- Close the linked issue after a merge to a non-default branch

## Versioning

Mach uses [Semantic Versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):

- **MAJOR**: Breaking changes to the supported compiler surface
- **MINOR**: New features and backward-compatible additions
- **PATCH**: Bug fixes, documentation, and internal improvements

Tags are created and pushed on `main` after merging from `dev`. The standard
library is versioned separately in its own repository. The
[4.30.0 / 5.0.0 release shape](doc/design/release-shape.md) records the current
migration window and the compatibility limits of the transition release.

A release bump updates both `[project].version` in `mach.toml` and
`MACH_VERSION` in `src/lang/version.mach`. CI and the tag workflow require the
two values to match.

### Release Flow

```
dev (ongoing work)
 └─► merge to main
      └─► tag vX.Y.Z on main
```

---

## Reporting Issues

**Bugs:**
- Check existing issues first
- Provide minimal reproduction case
- Include version info, OS, error messages
- Attach relevant files if applicable

**Enhancements:**
- Align with Mach's philosophy (simplicity, explicitness, predictability)
- Provide concrete use cases
- Consider if it can be a library instead

---

## Coding Standards

### Mach Code (in `src/` and `dep/std/`)

Mach coding standards are in flux while syntax stabilizes and the userbase grows. Follow existing patterns and refer to the [language reference](doc/language/README.md) for language features.

---

## Project Structure

```
mach/
├── dep/               # dependency realizations
│   └── std/           # standard library, pinned by a committed gitlink
├── doc/               # documentation
├── src/               # self-hosting mach compiler
├── out/               # ignored build output, grouped by target and profile
└── mach.toml          # project configuration
```

The standard library lives in a separate repository
([mach-std](https://github.com/briar-systems/mach-std)). `[dep.std]` declares
its source and selector in `mach.toml`, `.gitmodules` records its submodule
location, and the committed gitlink at `dep/std` pins the exact revision.
`mach dep pull` initializes that pin in place. Builds verify the dependency
without fetching or advancing its selector. See the
[dependency model](doc/design/dependency-model.md).

Building from source uses an existing `mach` (the latest release). See [Getting Started](#getting-started). The original bootstrap seed ([mach-boot](https://github.com/briar-systems/mach-boot)) is no longer part of the build. It remains only as a from-scratch cold-start hatch.

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

---

Thank you for contributing to Mach!
