# Description

Continue development of the self-hosted Mach compiler (`imach`). The compiler infrastructure is scaffolded with lexer, parser, AST, type system, semantic analysis, and codegen modules. The lexer is functional; remaining components are stubbed and need implementation.

# Outline

## Phase 1: Parser Implementation
- [x] Fix token kind comparison in parser
  - [x] Implement proper union tag comparison for TokenKind (using tag field)
  - [x] Update `Parser.check()` to work correctly
  - [x] Update `Parser.match_tag()` and `Parser.consume()`
- [x] Implement `parse_program()` - top-level declarations
  - [x] Parse `use` statements
  - [x] Parse `pub` modifier
  - [x] Parse `ext` declarations
  - [x] Parse `def` type aliases
  - [x] Parse `rec` record definitions
  - [x] Parse `uni` union definitions
  - [x] Parse `val` constants
  - [x] Parse `var` global variables
  - [x] Parse `fun` function definitions
  - [x] Parse `test` blocks
  - [x] Parse comptime statements (`$if`, `$error`) - partial, attributes work
- [x] Implement `parse_stmt()` - statements
  - [x] Parse `if`/`or` conditionals
  - [x] Parse `for` loops
  - [x] Parse `ret` return
  - [x] Parse `brk` break
  - [x] Parse `cnt` continue
  - [x] Parse `fin` defer
  - [x] Parse block statements
  - [x] Parse `val`/`var` local declarations
  - [x] Parse expression statements
  - [x] Parse inline `asm` blocks
- [x] Implement `parse_expr()` - expressions with Pratt parsing
  - [x] Define operator precedence table
  - [x] Parse binary operators
  - [x] Parse prefix operators (!, -, ~, ?, @)
  - [x] Parse postfix operators (calls, indexing, field access, casts)
  - [x] Parse primary expressions (literals, identifiers, parenthesized)
  - [x] Parse array/struct literals
- [x] Implement `parse_type()` - type expressions
  - [x] Parse pointer types (*T, &T)
  - [x] Parse array types ([N]T)
  - [x] Parse function types
  - [x] Parse named types (identifiers)
  - [x] Parse generic instantiation (T[U, V])
  - [x] Parse anonymous rec/uni

## Phase 2: Semantic Analysis

### 2.1 Foundation - Type System Enhancements
- [x] Enhance `type.mach` with complete type checking utilities
  - [x] Implement `is_integer()` - check TypeKind against integer variants
  - [x] Implement `is_float()` - check TypeKind against f32/f64
  - [x] Implement `is_pointer()` - check for ty_pointer and ty_ptr
  - [x] Implement `types_equal()` - structural type comparison
  - [x] Implement `can_assign()` - check if source type assignable to target
  - [x] Add primitive type caching (singleton types for u8, i32, etc.)
  - [x] Add size/alignment calculation for each TypeKind
  - [x] Add `def TypeKind: u8` type alias for cleaner code
  - [x] Add `can_cast()` for cast validation
  - [x] Add `is_bool()`, `is_void()`, `is_array()`, `is_record()`, `is_union()`, etc.
  - [x] Add `get_field()` for field lookup in records/unions
  - [x] Add `kind_name()` and `primitive_kind_from_name()` utilities

### 2.2 Type Resolution (`resolve_type`)
- [x] Implement `resolve_type` in `sema.mach`
  - [x] Handle `type_name` nodes - lookup in symbol table
    - [x] Check for primitive type names (u8, i32, bool, str, etc.)
    - [x] Look up user-defined types in symbol table
    - [ ] Handle qualified names (module.Type) - deferred
  - [x] Handle `type_ptr` nodes - resolve inner, wrap as mutable pointer
  - [x] Handle `type_ref` nodes - resolve inner, wrap as immutable pointer
  - [x] Handle `type_array` nodes - resolve element type, create array type
  - [x] Handle `type_fun` nodes - resolve params and return type
  - [ ] Handle `type_rec` nodes (anonymous records) - deferred
  - [ ] Handle `type_uni` nodes (anonymous unions) - deferred
  - [ ] Handle generic type instantiation (T[U, V]) - deferred
  - [ ] Cache resolved types to avoid duplication - deferred

### 2.3 Symbol Collection (Pass 1)
- [x] Implement `collect_declarations()` for first pass
  - [ ] Handle `decl_use` - register imported modules - deferred
  - [x] Handle `decl_def` - register type aliases
  - [x] Handle `decl_rec` - register record types
  - [x] Handle `decl_uni` - register union types
  - [x] Handle `decl_val` - register constants
  - [x] Handle `decl_var` - register global variables
  - [x] Handle `decl_fun` - register functions
  - [x] Handle `decl_ext` - register external symbols
  - [ ] Handle `decl_test` - register test blocks - skipped (not symbols)
  - [ ] Track public visibility from `decl_pub` - deferred
  - [x] Add `def SymbolKind: u8` type alias
  - [x] Add `is_type_symbol()`, `is_value_symbol()`, `is_callable()` helpers

### 2.4 Type Resolution Pass (Pass 2)
- [x] Implement `resolve_types()` for second pass
  - [x] Resolve record types (basic, without fields)
  - [x] Resolve union types (basic, without fields)
  - [ ] Resolve field types in records/unions - deferred
  - [ ] Resolve function parameter and return types - deferred
  - [ ] Resolve variable/constant types - deferred
  - [ ] Resolve type alias definitions - deferred
  - [ ] Detect and report circular type dependencies - deferred

### 2.5 Expression Type Checking (`check_expr`)
- [x] Implement `check_expr` in `sema.mach`
  - [x] `expr_lit_int` - return i64
  - [x] `expr_lit_float` - return f64
  - [x] `expr_lit_char` - return u8
  - [x] `expr_lit_string` - return str
  - [x] `expr_lit_nil` - return nil type
  - [x] `expr_lit_bool` - return bool
  - [x] `expr_ident` - lookup in symbol table, return symbol's type
  - [x] `expr_binary` - check operand types, determine result type
    - [x] Arithmetic: +, -, *, /, % (numeric operands)
    - [x] Comparison: ==, !=, <, >, <=, >= (return bool)
    - [x] Logical: &&, || (bool operands, return bool)
    - [x] Bitwise: &, |, ^, <<, >> (integer operands)
    - [x] Assignment: = (type compatibility check)
  - [x] `expr_unary` - check operand, determine result type
    - [x] Negation: - (numeric)
    - [x] Logical not: ! (bool)
    - [x] Bitwise not: ~ (integer)
    - [ ] Try: ? (unwrap Result/Option) - deferred
    - [x] Address-of: @ (create pointer to lvalue)
    - [x] Dereference: * (as prefix operator)
  - [x] `expr_call` - resolve function, check return type
  - [x] `expr_field` - check base has field, return field type
  - [x] `expr_index` - check base is array/pointer, return element type
  - [x] `expr_cast` - validate cast legality, return target type
  - [x] `expr_addr` - check operand, return pointer type
  - [x] `expr_deref` - check operand is pointer, return inner type
  - [x] `expr_paren` - check inner expression
  - [ ] `expr_array_lit` - deferred
  - [ ] `expr_struct_lit` - deferred

### 2.6 Statement Type Checking (`check_stmt`)
- [x] Implement `check_stmt` in `sema.mach`
  - [x] `decl_val` - check initializer type matches declared type
  - [x] `decl_var` - check initializer type, allow type inference
  - [x] `stmt_if` - check condition is bool, check branches
  - [x] `stmt_for` - check condition is bool (if present), check body
  - [x] `stmt_ret` - check return value matches enclosing function's return type
  - [x] `stmt_brk` - verify inside loop
  - [x] `stmt_cnt` - verify inside loop
  - [x] `stmt_fin` - check deferred statement
  - [x] `stmt_block` - push scope, check statements, pop scope
  - [x] `stmt_expr` - check expression
  - [x] `stmt_masm` - skip (no type checking for inline asm)

### 2.7 Control Flow Validation (Pass 4)
- [x] Implement control flow analysis (partial)
  - [x] Track loop nesting for break/continue validation
  - [x] Track function return type for return validation
  - [ ] Detect missing returns in non-void functions - deferred
  - [ ] Detect unreachable code after return/break/continue - deferred

### 2.8 Multi-Pass Analysis Entry Point
- [x] Implement `analyze()` orchestration
  - [x] Pass 1: Call `collect_declarations()` on program
  - [x] Pass 2: Call `resolve_types()`
  - [x] Pass 3: Call `check_declarations()` on each declaration body
  - [ ] Pass 4: Full control flow validation - deferred
  - [x] Collect errors with position info
  - [ ] Error printing with source context - deferred

### 2.9 AST Enhancements
- [x] Add `def NodeKind: u8` type alias
- [x] Add NODE_* constants for all node kinds
- [x] Add `is_decl()`, `is_stmt()`, `is_expr()`, `is_type()`, `is_literal()` helpers
- [x] Add `get_name()`, `get_list()`, `get_child()` accessor functions
- [x] Add `get_int_value()`, `get_float_value()`, `get_bool_value()` accessors
- [x] Add `kind_name()` for debugging

## Phase 3: Code Generation (MASM)
- [ ] Implement AST to IR lowering (`lower.mach`)
  - [ ] Lower function declarations
  - [ ] Lower statements to IR instructions
  - [ ] Lower expressions to IR values
  - [ ] Handle lvalue computation for assignments
- [ ] Implement object file emission (`emit.mach`)
  - [ ] ELF header generation
  - [ ] Section layout
  - [ ] Symbol table generation
  - [ ] Relocation generation
- [ ] Implement linking
  - [ ] Invoke system linker
  - [ ] Handle static libraries

## Phase 4: Build Command Integration
- [ ] Implement `build` command
  - [ ] Parse command-line options (-o, -m, -I, --target)
  - [ ] Detect project vs single file input
  - [ ] Load mach.toml configuration
  - [ ] Invoke full compiler pipeline
  - [ ] Report errors with source locations
- [ ] Implement `run` command
  - [ ] Parse --target flag
  - [ ] Resolve binary path
  - [ ] Execute with arguments
- [ ] Implement `test` command
  - [ ] Parse options (--target, --filter)
  - [ ] Find and compile test files
  - [ ] Run tests and report results

## Phase 5: Project Management Commands
- [ ] Implement `init` command
  - [ ] Create directory structure
  - [ ] Generate mach.toml
  - [ ] Generate main.mach template
  - [ ] Initialize git repo
- [ ] Implement `dep` subcommands
  - [ ] `dep list`
  - [ ] `dep info`
  - [ ] `dep add`
  - [ ] `dep del`
  - [ ] `dep pull`
  - [ ] `dep tidy`

## Documentation & Housekeeping
- [x] Document the bootstrap test runner system (external runner architecture)
  - [x] Update `doc/testing.md` with new architecture details
  - [x] Document per-test isolation and crash detection
  - [x] Document `std.runtime` integration for portable tests
- [ ] Add any user-specified items below

## User-Added Items
(Add items here as needed)

# Log

## 2026-01-22T00:00 UTC
- Initialized TASK.md for self-hosted compiler continuation
- Investigated current state of `src/` (imach):
  - Lexer: functional, has tests passing
  - Token: complete (kind constants, creation helpers)
  - Parser: skeleton only, core methods stubbed
  - AST: type definitions complete, allocation works
  - Symbol table: mostly complete (scope management, lookup)
  - Type system: skeleton (type kinds defined, creation works, checking stubbed)
  - Sema: skeleton (context defined, analysis stubbed)
  - MASM/IR: skeleton (types defined, lowering/emit stubbed)
  - Commands: all stubbed except help
- Key blocker: parser needs token kind comparison to work (union tag handling)
- Next step: implement Parser.check() with proper union tag comparison

## 2026-01-22T01:00 UTC
- Implemented full parser in `src/compiler/parser.mach` (~2950 lines)
- Key changes:
  - `Parser.check(tag: u8)` now uses tag-based comparison (not union comparison)
  - Added `check2()`, `check3()` for multiple tag checks
  - Added `match_tag()` and updated `consume()` to use tags
  - Added `source` field to Parser for token text extraction
  - Added `current_text()`, `previous_text()`, `get_token_text()` helpers
  - Added `add_error()` to properly append errors to error list
  - Implemented error recovery with `synchronize()`
- Implemented all top-level declaration parsing:
  - `parse_use()` - use statements with optional alias
  - `parse_pub()` - pub modifier dispatch
  - `parse_ext()` - external declarations (fun, val, var)
  - `parse_def()` - type aliases
  - `parse_rec()` - record definitions with fields
  - `parse_uni()` - union definitions
  - `parse_val()` - immutable value declarations
  - `parse_var()` - mutable variable declarations
  - `parse_fun()` - function declarations with receiver support
  - `parse_test()` - test blocks
  - `parse_comptime()` - $if and attributes (partial)
- Implemented all statement parsing:
  - `parse_if()` - if/or conditionals
  - `parse_for()` - for loops (condition and infinite)
  - `parse_ret()` - return statements
  - `parse_brk()`, `parse_cnt()` - break/continue
  - `parse_fin()` - defer statements
  - `parse_asm()` - inline assembly (skips content)
  - `parse_block()` - block statements
  - `parse_expr_stmt()` - expression statements
- Implemented Pratt expression parser:
  - Precedence table in `get_precedence()`
  - `parse_precedence()` for precedence climbing
  - All binary operators (arithmetic, comparison, logical, bitwise)
  - All unary operators (!, -, ~, ?, @, &, *)
  - Postfix operators via `parse_postfix()`:
    - `parse_call()` - function calls
    - `parse_index()` - array indexing
    - `parse_field()` - field access
    - `parse_cast()` - type casts (::)
  - Primary expressions:
    - Integer, float, char, string literals
    - nil, true, false
    - Identifiers (with dotted path support)
    - Parenthesized expressions
    - Array literals
    - Struct literals
- Implemented type parsing:
  - `parse_ptr_type()` - *T and &T
  - `parse_array_type()` - [N]T
  - `parse_fun_type()` - fun(...) T
  - `parse_named_type()` - named types with generic args
- All 478 existing tests still pass
- Note: Parser module causes segfault when imported into test files
  - Build works correctly for imach binary
  - Issue appears to be bootstrap compiler limitation with large modules in test context
  - Parser correctness to be verified via integration testing when build command is implemented

## 2026-01-22T02:00 UTC
- Updated `doc/testing.md` with new external test runner architecture
- Key documentation changes:
  - Added `--filter`, `-m`, `-h` command-line options
  - Documented per-test isolation (each test runs in own process)
  - Documented crash detection (exit codes, signal handling)
  - Documented platform portability via `std.runtime`
  - Updated "How it works" section with new per-test compilation flow
  - Documented test binary location (`.tests` hidden directory)
  - Removed outdated `dir_tests` configuration section (now hardcoded to `.tests`)

## 2026-01-22T03:00 UTC
- Fixed segfault when analyzing modules with recursive type definitions
- Root cause: `sema_analyze_rec`/`sema_analyze_uni` would re-enter analysis for the same symbol when resolving recursive field types
- Fix implemented in bootstrap compiler:
  - Added `is_being_analyzed` flag to Symbol struct (symbol.h, symbol.c)
  - Modified `sema_maybe_analyze_symbol_decl_in_module` to check/set flag and prevent re-entry
  - Refactored `sema_analyze_rec` and `sema_analyze_uni`:
    - Create Type object first with placeholder NULL field types
    - Assign `sym->type` before resolving field types
    - Resolve field types afterwards (recursive refs find sym->type already set)
    - Recalculate size/alignment after all fields resolved
  - Fixed `type_create_union` to handle NULL field types during initial creation
- Added regression tests in `src/tests/parser_tests.mach`:
  - Self-referential record (TestNode with *TestNode child)
  - Mutually recursive types (TypeA <-> TypeB)
  - Doubly-linked list (LinkedList with *LinkedList next/prev)
- All 481 tests pass (478 original + 3 new recursive type tests)
- Committed fix to feat/sh branch: b4977b8

## 2026-01-22T04:00 UTC
- Started Phase 2: Semantic Analysis implementation
- Enhanced type system (`type.mach`):
  - Added `def TypeKind: u8` type alias for cleaner code
  - Implemented all type checking utilities: is_integer, is_float, is_pointer, is_bool, etc.
  - Implemented types_equal() with structural comparison
  - Implemented can_assign() for assignment compatibility
  - Implemented can_cast() for cast validation
  - Added primitive type caching in TypeContext
  - Added Field record for record/union fields
  - Added get_field() for field lookup
  - Added record_type(), union_type(), function_type(), generic_type() constructors
  - Added kind_name() and primitive_kind_from_name() utilities
- Enhanced symbol table (`symbol.mach`):
  - Added `def SymbolKind: u8` type alias
  - Added resolved_type and is_mutable fields to Symbol
  - Added kind_name(), is_type_symbol(), is_value_symbol(), is_callable() helpers
- Enhanced AST module (`ast.mach`):
  - Added `def NodeKind: u8` type alias
  - Added NODE_* constants for all 50 node kinds
  - Added classification helpers: is_decl, is_stmt, is_expr, is_type, is_literal
  - Added data accessors: get_name, get_list, get_child
  - Added literal accessors: get_int_value, get_float_value, get_bool_value
  - Added kind_name() for debugging
- Implemented semantic analyzer (`sema.mach`):
  - Multi-pass analysis: collect_declarations -> resolve_types -> check_declarations
  - Type resolution for primitives, pointers, refs, arrays, function types
  - Expression type checking for all literal types
  - Expression type checking for identifiers, binary, unary operators
  - Expression type checking for calls, field access, indexing, casts
  - Statement checking for val/var declarations with type inference
  - Statement checking for if/for/ret/brk/cnt/fin/block
  - Function and test block checking with return type validation
  - Loop depth tracking for break/continue validation
  - Error reporting with source positions
- All 481 tests still pass
- Build succeeds for imach binary