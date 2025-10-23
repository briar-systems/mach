# Contributing to Mach

Thank you for your interest in contributing to Mach!

- [Contributing to Mach](#contributing-to-mach)
  - [Code of Conduct](#code-of-conduct)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Building](#building)
  - [Branch Workflow](#branch-workflow)
    - [Contributing Changes](#contributing-changes)
  - [Reporting Issues](#reporting-issues)
  - [Coding Standards](#coding-standards)
    - [C Code (Bootstrap Compiler in `boot/`)](#c-code-bootstrap-compiler-in-boot)
    - [Mach Code (in `src/` and `std/`)](#mach-code-in-src-and-std)
  - [Project Structure](#project-structure)
  - [License](#license)

---

## Code of Conduct

Be respectful, constructive, and professional. Treat Mach like a passion project and its community like family.

---

## Getting Started

### Prerequisites
- C compiler: `clang` with C23 support
- LLVM 14+ with development headers
- GNU Make
- Git

### Building

```bash
git clone https://github.com/octalide/mach.git
cd mach
make full
```

---

## Branch Workflow

We use a two-branch model:

- **`main`** - Stable branch. Production-ready code only.
- **`dev`** - Development branch. Integration point for features.

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
- Ensure code builds and works
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

### Mach Code (in `src/` and `std/`)

Mach coding standards are in flux while syntax stabalizes and the userbase grows. Follow existing patterns and refer to the [language specification](doc/language-spec.md) for language features.

---

## Project Structure

```
mach/
├── boot/              # bootstrap C compiler
├── doc/               # documentation
├── src/               # mach compiler (written in mach)
├── std/               # standard library
├── Makefile           # build system
└── mach.toml          # project configuration
```

---

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).

---

Thank you for contributing to Mach!
