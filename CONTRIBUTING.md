# Contributing to Mach

Thank you for your interest in contributing to Mach!

---

## Code of Conduct

Be respectful, constructive, and professional. Treat Mach like a passion project and its community like family.

---

## Getting Started

### Prerequisites
- GNU Make
- Git
- curl (for auto-downloading the bootstrap compiler)

### Building

Mach is self-hosting — the compiler is written in Mach and compiles its own source. `make` runs a 4-stage bootstrap build chain; the seed compiler (`cmach`) is automatically downloaded from [mach-boot](https://github.com/octalide/mach-boot) releases.

1. **cmach** — Seed compiler (pre-built, auto-downloaded)
2. **imach** — Intermediate compiler: Mach source compiled by cmach
3. **smach** — Self-hosted compiler: Mach source compiled by imach
4. **mach** — Final compiler: Mach source compiled by smach

```bash
git clone --recurse-submodules https://github.com/octalide/mach.git
cd mach
make    # downloads cmach and builds all stages
```

The four binaries are written to `out/bin/`; the final compiler is `out/bin/mach`. The bootstrap reaches a byte-identical fixpoint — recompiling the source with `mach` reproduces `mach` exactly. `make clean` wipes `out/`.

To use a custom cmach build:

```bash
CMACH=/path/to/cmach make
```

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
- Ensure code builds with `make`
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
├── dep/
│   └── mach-std/      # standard library (git submodule)
├── doc/               # documentation
├── examples/          # example projects and syntax fixtures
├── src/               # self-hosting mach compiler
├── out/               # build output (git-ignored); final compiler at out/bin/mach
├── Makefile           # build system (auto-downloads cmach)
└── mach.toml          # project configuration
```

The standard library lives in a separate repository ([mach-std](https://github.com/octalide/mach-std)) and is included as a git submodule under `dep/mach-std/`.

The bootstrap compiler lives in a separate repository ([mach-boot](https://github.com/octalide/mach-boot)) and is automatically downloaded during the build.

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

---

Thank you for contributing to Mach!
