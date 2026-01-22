# Description
Phase 7: Machification Pass - Review and refactor the self-hosted compiler and standard library to use idiomatic Mach patterns including readonly pointers, generics, and method syntax.

# Outline
## Phase 7: Machification Pass
- [ ] Review entire self-hosted codebase for idiomatic Mach patterns
- [ ] Convert appropriate patterns to use readonly pointers:
  - [ ] Identify functions taking `*T` that do not modify data or should not claim ownership
  - [ ] Change to `&T` where applicable (convention: `*T` = mutable/ownership, `&T` = readonly)
  - [ ] Update call sites accordingly
- [ ] Convert appropriate patterns to use generics:
  - [ ] Identify repeated code that differs only by type
  - [ ] Replace with generic functions/types where beneficial
  - [ ] Ensure generic instantiation works correctly
- [ ] Convert appropriate patterns to use methods:
  - [ ] Identify functions that operate on a primary type (e.g., `foo_bar(foo: *Foo, ...)`)
  - [ ] Convert to method syntax `fun (this: *Foo) bar(...)`
  - [ ] Update call sites to use method call syntax
- [ ] Review and clean up:
  - [ ] Remove redundant helper functions
  - [ ] Consolidate duplicate utility code
  - [ ] Improve naming consistency
  - [ ] Ensure documentation comments are complete
- [ ] Final testing:
  - [ ] Ensure all tests pass after machification
  - [ ] Verify bootstrap compiler still works
  - [ ] Verify self-hosted compiler can compile itself
- [ ] Apply the above to the standard library as well

# Log
## 2025-01-22
- Reset TASK.md for Phase 7 after completing Phase 6 fixes
- Fixed compilation issues in the self-hosted compiler:
  - Replaced union literal syntax with tagged record constructors throughout parser.mach, lower.mach
  - Fixed `usize` type resolution by adding missing `std.types.size` imports
  - Fixed `allocator.init()` -> `allocator.default()` in all command files
  - Fixed token field access (`tok.kind.tag` -> `tok.tag`)
  - Fixed symbol lookup to handle `Option[*Symbol]` return type
  - Fixed readonly/mutable pointer mismatches in config.mach and elf.mach
  - Added missing `ty` field to ast.Node for resolved types
  - Added missing `std.types.bool` imports in mach-std platform files
  - Renamed duplicate `parse_field` method to `parse_field_access`
  - Fixed various other type mismatches and undefined symbol references
- All 481 tests passing
- Compiler builds successfully (imach-build completes)