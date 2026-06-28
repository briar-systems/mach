# Contributing to Mach

Thank you for your interest in contributing to Mach!

---

## Code of Conduct

Be respectful, constructive, and professional. Treat Mach like a passion project and its community like family.

---

## Getting Started

### Prerequisites
- Git
- An existing `mach` binary — Mach is self-hosting, so building from source needs one. Install the latest [release](https://github.com/briar-systems/mach/releases).

### Building

Mach builds its own source with an existing `mach`:

```bash
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull
mach build .
```

The compiler is written to `out/<target>/bin/mach`. Because the language is self-hosting, the build reaches a byte-identical fixpoint — recompiling the source with the result reproduces it exactly.

---

## Branching Strategy

We use a two-branch model:

- **`main`** — Stable branch. Tagged releases only.
- **`dev`** — Development branch. Integration point for features and fixes.

All work happens on feature branches off `dev`. When `dev` reaches a stable milestone, it is merged into `main` and tagged.

### Branch Naming

- `feat/<name>` — New features
- `fix/<name>` — Bug fixes
- `doc/<name>` — Documentation changes

### Contributing Changes

1. **Fork the repository**
2. **Create a feature branch from `dev`:**
   ```bash
   git checkout dev
   git pull origin dev
   git checkout -b feat/your-feature-name
   ```
3. **Make your changes** following the coding standards below
4. **Commit with clear messages:**
   ```
   component: brief description

   Longer explanation if needed.
   ```
5. **Push to your fork:**
   ```bash
   git push origin feat/your-feature-name
   ```
6. **Open a Pull Request** targeting the `dev` branch

**Pull Request Guidelines:**
- Target `dev` branch (not `main`)
- Provide clear description of changes
- Link related issues
- Ensure the compiler builds and reaches the byte-identical fixpoint
- Run the tests with `mach test .`
- Follow coding standards

## Versioning

Mach uses [Semantic Versioning](https://semver.org/) (`vMAJOR.MINOR.PATCH`):

- **MAJOR** — Breaking changes to the language or standard library
- **MINOR** — New features, backward-compatible additions
- **PATCH** — Bug fixes, documentation, internal improvements

Tags are created on `main` after merging from `dev`. The current pre-1.0 convention treats minor bumps as potentially breaking while the language stabilizes.

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

### Mach Code (in `src/` and `dep/mach-std/`)

Mach coding standards are in flux while syntax stabilizes and the userbase grows. Follow existing patterns and refer to the [language reference](doc/language/README.md) for language features.

---

## Project Structure

```
mach/
├── dep/                # dep checkouts realized by `mach dep pull` (git-ignored)
│   └── mach-std/       # standard library
├── doc/               # documentation
├── src/               # self-hosting mach compiler
├── out/               # build output (git-ignored); compiler at out/<target>/bin/mach
└── mach.toml          # project configuration
```

The standard library lives in a separate repository ([mach-std](https://github.com/briar-systems/mach-std)) and is realized into `dep/mach-std/` by `mach dep pull` from the pin in `mach.toml`. The dep is currently frozen at an immutable `commit/<sha>` (the v1.7 self-host seed freeze, [#1486](https://github.com/briar-systems/mach/issues/1486)); it reverts to `branch/main` after the post-v1.7 re-seed.

Building from source uses an existing `mach` (the latest release) — see [Getting Started](#getting-started). The original bootstrap seed ([mach-boot](https://github.com/briar-systems/mach-boot)) is no longer part of the build; it remains only as a from-scratch cold-start hatch.

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

---

Thank you for contributing to Mach!
