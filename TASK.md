# Description
Implement a Mach testing framework with top-level `test "name" { ... }` blocks, zero dependencies in user code (including stdlib), per-module test binaries, `mach test` execution over all tests (no filters), stdout-only reporting, and all generated outputs controlled by `mach.toml` output settings (including a `tests` output directory for binaries). Ensure tests only run for projects with `mach.toml` and use hard-fail checks while still running all modules and aggregating pass/fail.

# Outline
- [x] Review existing compiler, CLI, and config handling for build/run/output paths.
  - [x] Locate lexer/parser/sema hooks for new `test` block syntax.
  - [x] Locate `mach.toml` output settings usage and add `tests` output directory support.
- [x] Implement compiler support for top-level `test` blocks.
  - [x] Add lexer/parser/AST updates for `test "name" { ... }` blocks with a metadata slot.
  - [x] Add semantic checks and codegen for test entry points.
- [x] Implement `mach test` command.
  - [x] Discover all test modules for the project.
  - [x] Build per-module test binaries into the configured `tests` output directory.
  - [x] Run all test binaries, aggregate pass/fail, and exit non-zero on failures.
- [x] Update documentation and examples.
  - [x] Document syntax and `mach test` behavior in `doc/*`.
  - [x] Ensure examples compile.
- [x] Validate and iterate.
  - [x] Build/test locally and note any limitations.

# Log
## 2026-01-16 00:00
- Initialized TASK.md with task description and initial outline.

## 2026-01-16 00:01
- Updated task description and outline with clarified requirements (hard-fail, top-level tests, tests output directory).

## 2026-01-16 00:02
- Added `test` keyword support in lexer/parser/AST and reserved space for metadata.
- Implemented `mach test` command with per-module test binaries and stdout-only reporting.
- Added `dir_tests` output setting to config parsing and updated docs and example configs.
- Built `cmach` to validate changes.
