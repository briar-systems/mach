# Contributing to Mach

Thank you for your interest in contributing to Mach!

---

## Code of Conduct

Be respectful, constructive, and professional. Treat Mach like a passion project and its community like family.

---

## Getting Started

### Prerequisites
- C compiler: `clang` with C23 support
- GNU Make
- Git

### Building

Mach uses a three-stage bootstrap build chain:

1. **cmach** — Bootstrap compiler written in C (`boot/`)
2. **imach** — Intermediate compiler: Mach source compiled by cmach
3. **mach** — Final compiler: Mach source compiled by imach (self-hosting)

```bash
git clone --recurse-submodules https://github.com/octalide/mach.git
cd mach
make cmach    # build the C bootstrap compiler
make imach    # build the intermediate compiler
make mach     # build the self-hosting compiler
```

To run the test suite:

```bash
make test     # runs unit tests via cmach
```

---

## Branch Workflow

We use a two-branch model:

- **`main`** — Stable branch. Production-ready code only.
- **`dev`** — Development branch. Integration point for features.

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
- Ensure code builds with `make cmach imach`
- Follow coding standards

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

### C Code (Bootstrap Compiler in `boot/`)

**Standards:**
- Target C23 using `clang`
- Only use C standard library

**Naming:**
- Functions: `lowercase_with_underscores`
- Structs: `PascalCase`

**Memory Management:**
```c
// structs use init/dnit pattern
void ast_node_init(ASTNode *node);  // initialize fields, don't allocate struct
void ast_node_dnit(ASTNode *node);  // clean up internals, don't free struct

// usage
ASTNode node;
ast_node_init(&node);
// use node
ast_node_dnit(&node);
```

### Mach Code (in `src/` and `dep/mach-std/`)

Mach coding standards are in flux while syntax stabilizes and the userbase grows. Follow existing patterns and refer to the [language specification](doc/language-spec.md) for language features.

---

## Project Structure

```
mach/
├── boot/              # bootstrap C compiler (stage 1)
├── dep/
│   └── mach-std/      # standard library (git submodule)
├── doc/               # documentation
├── src/               # self-hosting mach compiler (stages 2 & 3)
├── Makefile           # build system
└── mach.toml          # project configuration
```

The standard library lives in a separate repository ([mach-std](https://github.com/octalide/mach-std)) and is included as a git submodule under `dep/mach-std/`.

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

---

Thank you for contributing to Mach!
