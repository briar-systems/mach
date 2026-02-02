# Goal

Bootstrap the self-hosted Mach compiler: achieve full `cmach → imach → mach` pipeline where the self-hosted compiler can compile itself.

# Scope

**In scope:**
- Parser parity: self-hosted parser must produce identical AST structure to bootstrap
- Sema parity: semantic analysis must match bootstrap behavior exactly
- Codegen parity: code generation must produce equivalent output
- Comptime parity: compile-time evaluation must handle all $if/$or/intrinsics correctly
- Interaction parity: user interface (errors, warnings, and all messages) must match bootstrap

**Out of scope:**
- New language features not in bootstrap
- Performance optimizations beyond correctness
- Cross-compilation (focus on host-to-host first)

# Architecture

```
cmach (C bootstrap)     →  imach (intermediate)  →  mach (final)
boot/src/compiler/*.c      out/bin/imach            out/bin/mach
```

Key compiler components (must be parity-matched):
- `ast.{c,mach}` - AST node definitions and operations
- `parser.{c,mach}` - lexer + parser producing AST
- `sema.{c,mach}` - semantic analysis and type checking
- `comptime.{c,mach}` - compile-time evaluation
- `type.{c,mach}` - type system
- `symbol.{c,mach}` - symbol table
- `masm/*.{c,mach}` - code generation (lowering + emit)

# Milestones

- [x] cmach builds successfully
- [x] cmach → imach builds successfully
- [x] **imach semantic analysis passes**
- [x] **imach → mach builds successfully** (produces ELF binary with real code)
- [x] **Multi-module lowering implemented** (text section: 184KB vs 18 bytes)
- [x] **mach produces working binaries** (`_start` symbol now emitted correctly)
- [x] **imach runs correctly** (bootstrap → self-hosted intermediate works)
- [ ] mach can compile itself (full bootstrap)
- [ ] Test suite passes on mach-compiled mach

# Recent Fixes (Session 18)

- **Fixed `_start` symbol missing**: Module buffer in `cmd_build.c` was limited to 64 entries, but project has 66+ modules
- `std.runtime` and `std.runtime.linux.x86_64` were being truncated from lowering list
- Increased `SemaLoadedModule` buffer from 64 to 256 in `cmd_build.c` and `cmd_test.c`
- Bootstrap-compiled mach now runs correctly with proper `_start` entry point
- imach (intermediate self-hosted) also runs correctly

# Session 17 Fixes

- **Multi-module lowering**: self-hosted compiler now lowers all imported modules (64+), not just main
- Added `LoadedModule` record and `get_loaded_modules()` to `sema.mach`
- Added `lower_all_modules()` and `lower_decls_into()` to `lower.mach`
- Fixed function params retrieval: `node.extra` not `node.data` (parser stores params in extra)
- Text section grew from 18 bytes to 184KB

# Session 16 Fixes

- Fixed bootstrap `lower.c`: small aggregate call results incorrectly dereferenced when passed as arguments
- Fixed self-hosted `lower.mach`: function body retrieved from wrong AST field (`node.extra` → `node.children`)
- Expanded isel: added opcode dispatch, handlers for MOV/LOAD/STORE/arithmetic/shifts/comparisons/jumps/setcc
- Fixed `select_mov`: now handles physical register destinations (previously only vreg)

# Key Principles

1. **Match the spec** - bootstrap compiler is the reference implementation
2. **No workarounds** - fix root causes, not symptoms
3. **AST parity first** - node kinds, structure, and layout must match exactly
4. **Comptime correctness** - $if/$or evaluation must select exactly one branch
