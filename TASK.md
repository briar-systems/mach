# Description
Update the Mach testing framework and configuration to make `dir_tests` per-target in `mach.toml`, validate the testing harness with smoke tests, and build a comprehensive Mach language test suite (casts, pointers, control flow, generics, edge cases) using the new `test "name" { ... }` blocks.

# Outline
- [x] Make `dir_tests` per-target in `mach.toml`.
  - [x] Inspect current config schema and target handling.
  - [x] Update parsing, validation, and accessors to scope `dir_tests` under each target.
  - [x] Update any defaults/merging behavior and related docs/examples.
- [x] Add and run smoke tests for the testing harness.
  - [x] Create minimal passing tests to validate discovery, build, and execution.
  - [x] Create minimal failing tests to validate reporting and non-zero exit.
  - [x] Run the test command and record results.
- [x] Build a comprehensive Mach language test suite using `test "name" { ... }`.
  - [x] Casts and numeric conversions (signed/unsigned, truncation, widening).
  - [x] Pointers and references (address-of, deref, pointer arithmetic if supported).
  - [x] Control flow (if/else, loops, break/continue, early returns).
  - [x] Records/structs, field access, and passing by value/reference.
  - [x] Generics and edge cases (records passed as pointers to generic methods, type inference).
  - [x] Arrays and indexing (value, nested, pointer indexing).
  - [x] Functions, calling conventions, and ABI-sensitive cases.
  - [ ] Error/diagnostic expectations for invalid programs (if harness supports).
  - [x] Organize tests by feature area and ensure coverage breadth.
- [x] Validate and iterate.
  - [x] Run full suite and note failures or gaps.
  - [x] Create minimal repro tests for each failure cluster.
    - [x] Record by-value copy minimal.
    - [x] Nested arrays/records minimal layout.
    - [x] Generic wrapper get/set/swap minimal.
  - [x] Inspect sema/type sizing and aggregate assignment rules.
  - [x] Inspect lowering/codegen for aggregate copy and generic instantiation.
  - [x] Add targeted debug logging for one failing case.
  - [x] Implement fixes and rerun `cmach test .`.
  - [x] Adjust tests or document compiler issues found.

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

## 2026-01-16 00:03
- Updated task scope to make `dir_tests` per-target, validate the testing harness with smoke tests, and expand Mach language coverage tests.

## 2026-01-16 14:03
- Moved `dir_tests` to per-target config (schema, loader, cmd_test usage) and updated `mach.toml` + docs.
- Added test suite files under `src/tests` with assertion helpers, smoke, casts, pointers, control flow, records, generics, arrays.
- Updated test harness to link using the mangled `_start` entry symbol and to write into per-target test output directories.
- Ran `cmach test .`; smoke/casts/pointers/control_flow passed; failures in records/arrays/generics (copy semantics, nested records/arrays, generic wrap get/set). Tests: 61, failures: 9.

## 2026-01-16 19:26
- Added `src/tests/repros.mach` with minimal repro tests for record copy, nested arrays/records, and generic wrap get/set.

## 2026-01-16 20:45
- Implemented comprehensive fixes for aggregate copying in `mach/boot/src/compiler/masm/lower.c`.
  - Added `emit_aggregate_copy` logic to local variable initialization and assignment.
  - Fixed array literal element copying for aggregates.
  - Updated struct literal field initialization to copy aggregates of all sizes correctly.
  - Refined `RET` and `CALL` lowering to handle small aggregates (<= 8 bytes) consistently: `RET` loads from address to register, `CALL` spills register return to stack address.
- Reran full test suite including repros.
  - All tests passed (67 tests, 0 failures).
  - Fixed issues with nested arrays, array of records, generic wrappers, and record by-value copies.
