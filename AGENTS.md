# AGENTS

This file is for agentic coding assistants working in this repo.
Keep it practical, follow existing patterns, and update docs when behavior changes.

## Hard rules
- do not manually edit anything under `dep/`; use `cmach dep ...` in a consuming project.
- if a change would affect mach language syntax, semantics, behavior, or other core language aspects, pause and discuss before proceeding.
- maintain existing coding style and conventions.
- keep comments lowercase and concise; avoid overcommenting trivial changes.
- avoid compatibility shims; prefer full implementations.
- remove dead code or replaced functionality when a change would otherwise strand it.
- treat `doc/*` as the source of truth for language/tooling behavior.
- if a change is user-visible, update relevant `doc/*` pages and keep examples compiling.
- if you modify bootstrap compiler c code under `boot/`, run `clang-format` when available.

## Repo orientation
- bootstrap compiler (c): `boot/` -> builds `out/bin/cmach` (~27k lines, C23, full feature set).
- self-hosted compiler (mach): `src/` -> builds `out/bin/imach` (~68 .mach files, ~60-70% feature parity).
- standard library lives in a separate repo (`https://github.com/octalide/mach-std`) and cannot be edited here.
- project config: `mach.toml` (targets, source paths, output paths).

## Build, run, test
### Build bootstrap compiler (recommended)
- `make cmach` (clean + build)
- `make cmach-build` (build only)
- output: `out/bin/cmach`

### Build self-hosted stages
- `make imach` or `make imach-build` (uses `cmach`)
- `make mach` or `make mach-build` (uses `imach`)

### Test suite
- `make test` (builds `cmach`, then runs `./out/bin/cmach test`)
- `cmach test .` (run all tests in repo)

### Run a single test or subset
- `cmach test --filter "lexer" .` (filters by module path or test name)
- `cmach test --filter "edge case" .`
- `cmach test --target linux .` (target selection from `mach.toml`)
- `-m` or `--modules` shows module-level progress
- `-v` or `--verbose` shows all test results (not just failures)

### Typical project commands
- `cmach init my-app` - create new project with scaffolding
- `cmach build .` or `cmach build . --target <name>`
- `cmach build . -o out/bin/myapp` - specify output
- `cmach build myfile.mach -I std=/path/to/mach-std/src` - module prefix mapping (C only)
- `cmach run .` - build and execute with argument forwarding
- `cmach dep add <url> --version branch/main` - add remote dependency
- `cmach dep add --local /path/to/lib <name>` - add local dependency
- `cmach dep pull` - update dependencies
- `cmach dep list` - show current deps
- `cmach dep del <name>` - remove dependency
- `cmach dep tidy` - clean orphaned submodules

### Compiler command parity
- `cmach`, `imach`, and `mach` are intended to be interchangeable for MVP.
- when in doubt, default to `cmach` for bootstrap reliability, but keep commands compatible.
- known gaps in Mach (self-hosted):
  - build: `-I` flag not supported (blocks single-file with imports)
  - build: library/archive mode missing
  - test: `--verbose` and `--modules` flags missing
  - test: per-test isolation missing (all tests in file run together)
  - no optimization passes (C has IR + x86_64 peephole)
  - no floating point support
  - incomplete ABI integration for function calls

## MASM (Mach Assembly) Backend Architecture

The compiler uses a two-layer backend called MASM:

### Layer 1: Portable IR
- location: `boot/src/compiler/masm/ir.c` (C), `src/compiler/masm/ir.mach` (Mach)
- 44-47 opcodes in three-operand form (mov, load, store, arithmetic, bitwise, comparisons)
- virtual register allocation (VREG_START = 1024)
- stack slot assignment for spilled registers

### Layer 2: Target-Specific Code Generation
- instruction selection: `boot/src/compiler/masm/isa/x86_64/isel.c`, `src/compiler/masm/isel.mach`
- instruction encoding: `boot/src/compiler/masm/isa/x86_64/encode.c`
- lowers portable IR to x86_64 opcodes (100+ x86_64-specific opcodes)
- handles register allocation, stack frame setup, calling conventions

### Lowering (AST -> IR)
- location: `boot/src/compiler/masm/lower.c`, `src/compiler/masm/lower.mach`
- converts AST to portable IR
- handles deferred statements (`fin`)
- variadic function lowering
- virtual register allocation per-function

### Object File Output
- ELF64 relocatable objects (C bootstrap)
- ELF64 executables directly without linker (Mach self-hosted advantage)
- sections: .text, .data, .rodata, .bss, .symtab, .strtab, .rela.text

### Supported Targets
- isa: x86_64 (only fully implemented)
- abi: SysV AMD64 (Linux standard)
- os: Linux (only fully implemented)
- Darwin/macOS and Windows types defined but stubbed
- AArch64 ISA type defined but not implemented

### Optimization Passes (C only)
- IR-level peephole (pre-isel)
- x86_64-specific peephole (post-isel)
- redundant move elimination, push/pop pair optimization
- Mach backend has no optimization infrastructure

## Code style: C (bootstrap compiler in `boot/`)
- language: C23, compiled with `clang`.
- standard library only; avoid non-standard extensions.
- naming:
  - functions: `lowercase_with_underscores`
  - structs/types: `PascalCase`
  - macros/constants: `UPPER_SNAKE_CASE` when needed.
- memory management:
  - use init/dnit pattern for structs; do not free stack-allocated structs.
  - free owned heap allocations in `dnit` or explicit teardown paths.
- error handling:
  - validate pointers/allocations and return early on failure.
  - surface parser/sema errors via existing helpers (e.g., `parser_error`).
  - avoid silent failures; print errors to stderr for CLI commands.
- formatting:
  - match existing brace style (K&R with braces on new line for blocks).
  - keep comments lowercase and concise.
  - run `clang-format` when editing `boot/` (if available).

## Code style: Mach (compiler in `src/`)
- follow existing patterns; language syntax is still evolving.
- imports:
  - group `use` statements by domain (std, project, compiler subsystems).
  - use aligned aliases for readability, e.g. `use print:   std.print;`.
- naming:
  - functions and locals: `lowercase_with_underscores`.
  - record types: `PascalCase`.
  - module paths: dot-separated, map to file paths under `src/`.
- types:
  - prefer explicit types in public APIs and records.
  - use `val` for immutable bindings, `var` for mutable.
  - avoid implicit conversions; use explicit casts where needed.
- error handling:
  - check `Result`/`Option` before use; handle `is_err()`/`is_some()`.
  - return early on error; keep control flow linear.
  - print user-facing errors via `std.print` helpers.
- control flow:
  - use `ret` consistently; avoid falling through when clarity matters.
  - use `cnt`/`brk` for loop control.
- comments:
  - lowercase, short, explain non-obvious intent only.
- documentation format:
  ```mach
  # summary and description
  # ---
  # param:  description
  # ret:    description of return value
  ```

## Tests and modules
- tests live alongside code in `.mach` files using `test "name" { ... }`.
- `cmach test` scans `dir_src` from `mach.toml`, builds each test as a separate executable, and runs them in isolated processes.
- passing tests are silent by default; failures and crashes are reported per test.
- `--filter` matches both module path and test name substrings.
- `-m` or `--modules` shows module-level progress during execution.
- test binaries are emitted under `out/<target>/tests/...` and cleaned per run.

## Mach syntax and idioms
- entry points use `$main.symbol = "main";` and a C-like `fun main(argc: i64, argv: &&u8) i64` signature.
- `use` imports modules; use aligned aliases for readability (e.g. `use print:   std.print;`).
- `val` is immutable, `var` is mutable; prefer explicit types in public APIs.
- control flow uses `ret`, `cnt`, and `brk` instead of fallthrough.
- pointer operators are explicit: `?expr` (address-of) and `@expr` (dereference).
- casts are explicit (`value::Type`); avoid implicit conversions.
- null pointer is `nil` (not `null` or `NULL`).
- tests return 1 to pass, 0 to fail (harness appends `ret 1` if omitted).
- single-line comments only: `# comment`
- one file = one module. module path follows file path.

### Language constructs
- `use [alias:] project.module;` - import with optional alias
- `pub` - export symbol
- `ext "C:name"` - external/FFI binding
- `def Name: Type;` - type alias
- `rec Name { field: Type; }` - record (struct)
- `uni Name[T] { variant: Type; }` - tagged union (optionally generic)
- `val name: Type = value;` - immutable binding
- `var name: Type = value;` - mutable binding
- `fun name(params) RetType { body }` - function
- `fun (self: *Type) method() { }` - method with receiver
- `if (cond) { } or { }` - conditional with else
- `for (cond) { }` or `for { }` - loop (no C-style for)
- `fin stmt;` - deferred execution (defer)
- `asm { ... }` - inline assembly
- generics: `Type[T]`, `fun[T](arg: T) T`
- pointers: `*T` (mutable), `&T` (immutable/ref)
- arrays: `[N]T`

## Dependency management
- never edit `dep/` directly.
- use `cmach dep ...` to add/update dependencies in consuming projects.
- version formats: `branch/<name>`, `commit/<hash>`, `^1.2.3`, `~1.2.3`, `1.2.3`
- to use local stdlib for testing:
  ```sh
  cmach dep add --local /ABS/PATH/TO/mach-std mach-std
  # test changes...
  cmach dep del mach-std  # revert before commit
  ```
- workspace note: prefer editing the `mach-std/` repository directly rather than the vendored copy under `dep/mach-std/`.

## Documentation expectations
- `doc/*` is the source of truth for language/tooling behavior.
- update `doc/*` when behavior changes (syntax, flags, outputs, semantics).
- keep examples compiling after updates.
- update `CHANGELOG.md` for user-visible changes (new features, fixes, breaking changes) between releases.
- changelog format: keep a changelog style with added/changed/fixed/removed sections.
- version in `CHANGELOG.md` should match `mach.toml` version.

## Safety and scope
- avoid destructive git actions.
- do not add compatibility shims; implement the full behavior instead.
- remove dead code when replacing functionality.

## Reference docs
- `doc/README.md` (language reference index)
- `doc/testing.md` (test runner, filtering, output semantics)
- `doc/cheatsheet.md` (syntax and toolchain commands)
- `doc/getting-started.md` (build setup and prerequisites)

## Self-Hosting Status (Current)

### Build Chain
```
cmach (C bootstrap) → imach (Mach compiled by C) → mach (Mach compiled by Mach)
      ↓ works              ↓ works                    ↓ CRASHES (SIGILL)
```

### Current Issue: Code Corruption
The final `mach` binary crashes with "Illegal instruction" - malformed machine code.
- Crash shows zeroed bytes (`00 00 00 00`) where instructions should be
- Likely cause: instruction encoding or ELF section writing bugs

### AST Field Mappings (Critical Reference)
Parser stores data in these fields - mismatches cause silent bugs:

| Node Type | `data` | `extra` | `children` |
|-----------|--------|---------|------------|
| VAR/VAL decl | name (str) | type node | initializer |
| CALL expr | arguments (*List) | type args | callee |
| FIELD access | field name (str) | - | base expr |

### Key Files for Debugging
- `src/compiler/masm/isa/x86_64/encode.mach` - instruction encoding
- `src/compiler/masm/isel.mach` - instruction selection
- `src/compiler/masm/of/elf.mach` - ELF output
- `src/compiler/masm/lower.mach` - IR lowering

### Comparison Commands
```bash
# Compare working (imach) vs broken (mach)
objdump -d ./out/bin/imach > /tmp/imach.asm
objdump -d ./out/linux/bin/mach > /tmp/mach.asm
diff /tmp/imach.asm /tmp/mach.asm | head -200
```
