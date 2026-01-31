# Description
Fix the self-hosted Mach compiler to properly bootstrap. The goal is to get the full pipeline `cmach → imach → mach` to complete successfully.

# Outline
- [x] Fix codegen bug for chained pointer field access
- [x] Fix codegen bug for widening casts (ZEXT)
- [x] Fix parser string allocation (names must be null-terminated)
- [x] Fix block statement list storage
- [x] Fix identifier parsing (single ident only, dots handled by postfix)
- [x] Fix `get_child` to handle both single-child and List conventions
- [x] Fix use declaration to store alias text
- [x] **Fix module symbol table scoping**
  - [x] Add `current_table: *symbol.Table` field to Sema struct
  - [x] Update `load_module` to save/restore `current_table` when switching contexts
  - [x] Update `collect_declarations` to use `current_table` instead of `sema.symbols`
  - [x] Update all symbol operations to use `current_table`
  - [x] **Fix struct copy codegen bug** - manual field copy workaround for Table struct
  - [x] Fix method symbol names to use qualified names (e.g., "Option.unwrap")
- [x] **Fix architectural mismatch: lazy vs multi-pass analysis**
  - [x] Implement `ensure_symbol_analyzed` for on-demand symbol analysis
  - [x] Update `check_ident` to use lazy analysis when `resolved_type` is nil
  - [x] Add context save/restore mechanism for cross-module resolution
  - [x] Skip generic records/unions/functions during type resolution (defer until instantiation)
  - [x] Fix cycle detection to avoid false positives in ensure_symbol_analyzed
- [x] **Fix module imports**
  - [x] Add `ModuleImport` struct for tracking unaliased imports
  - [x] Add `imports` field to `SemaModule`
  - [x] Update `collect_use_decl` to add unaliased imports to imports list
  - [x] Update `lookup_type_symbol` to search imported modules
  - [x] Update `check_ident` to search imported modules for symbols
  - [x] Fix `parse_pub` to wrap declarations in NODE_DECL_PUB wrapper
  - [x] Update `lookup_method` to search imported modules for methods
  - [x] Fix `find_method_symbol` to match qualified method names correctly
- [x] **Fix parser operator data storage**
  - [x] Fix binary expression parser to store operator tag in `node.data`
  - [x] Fix unary expression parser to store operator tag in `node.data`
- [x] **Fix call expression AST structure**
  - [x] Fix parser to store callee in `children`, arguments in `data`
  - [x] Previously both were in `children` causing argument count mismatch
- [x] Fix remaining semantic errors (major progress)
  - [x] "undefined identifier" - fixed by defining receiver `this` in method scope
  - [x] "undefined type" - fixed by correcting `def` syntax (colon, not equals) and generic param allocation
  - [x] "undefined field or method" - fixed by ensuring method symbols are lazily analyzed
  - [x] "identifier type not resolved" - fixed by handling generic function calls correctly
  - [x] "cast missing target type" - fixed check_cast to use children[1]
  - [x] "str indexing" - added special handling for str type in check_index
  - [x] "str method lookup" - added special case for TY_STR in lookup_method
- [x] **Implement context-aware integer literal inference** (COMPLETE)
  - [x] Add `check_expr_with_hint` function that takes expected type
  - [x] For integer literals: if expected type is integer, use it; else default to i64
  - [x] For float literals: if expected type is float, use it; else default to f64
  - [x] Update `check_var_decl` to pass declared type to initializer check
  - [x] Update `check_call` to pass parameter types to argument checks
  - [x] Update `check_binary` (assignment) to pass LHS type to RHS check
  - [x] Update `check_return` to pass expected return type to expression check
  - [x] Update `check_unary` to pass expected type through to operand
  - [x] Update `check_struct_lit` to pass field type to value check
  - [x] Update `check_array_lit` to pass element type to subsequent elements
  - [x] Update `check_index` to pass isize hint for index expression
  - [x] Pass expected type through parenthesized expressions
</newtml>

- [x] **Fix "unsupported type syntax" (26 errors)**
  - [x] Parser was creating NODE_DECL_UNI/NODE_DECL_REC for anonymous types in type context
  - [x] Modified `parse_rec` and `parse_uni` to take `is_type` parameter
  - [x] When called from `parse_type()`, now creates NODE_TYPE_UNI/NODE_TYPE_REC instead
- [x] **Fix "undefined type" for AllocError (37 errors)**
  - [x] Issue: generic function instantiation wasn't switching to origin module context
  - [x] When resolving return type `Result[*T, AllocError]` from std.allocator method,
        the unqualified `AllocError` was looked up in caller's module context
  - [x] Added module context switching in `instantiate_generic_function`
  - [x] Now saves caller context, switches to function's origin module, resolves types, restores
- [x] **Fix `str` <-> `&u8` type compatibility**
  - [x] `str` is an alias for `&u8`, so allow str -> &u8 and &u8 -> str assignments
  - [x] Allow nil -> str assignments (str is a pointer type alias)
  - [x] Add str casting rules: str <-> ptr, str <-> usize/u64, str <-> pointer
- [x] **Fix circular type dependency detection**
  - [x] When a type is being resolved, if we already have a shell type, return it
  - [x] This allows self-referential structures via pointers (e.g., `rec Type { inner: *Type }`)
- [ ] Other remaining errors
  - [ ] "undefined identifier" (34)
  - [ ] "duplicate definition" (20)
  - [ ] "unsupported expression" (6)
  - [ ] "not a struct type" (6)
  - [ ] "function may not return a value" (6)

# Progress Summary

## Session 11 Results (Current)

Starting errors: 447
Current errors: 76 (83% reduction this session, 99.5% from project start)

### Error Distribution After All Session 11 Fixes
| Error Type | Session Start | Final | Change |
|------------|--------------|-------|--------|
| type mismatch in assignment | 112 | 0 | **-112** |
| argument type mismatch | 77 | 0 | **-77** |
| invalid cast | 61 | 0 | **-61** |
| return type mismatch | 38 | 0 | **-38** |
| undefined identifier | 34 | 34 | 0 |
| initializer type mismatch | 31 | 2 | **-29** |
| field access requires record or union type | 28 | 0 | **-28** |
| duplicate definition | 20 | 20 | 0 |
| circular type dependency detected | 17 | 0 | **-17** |
| unsupported expression | 6 | 6 | 0 |
| not a struct type | 6 | 6 | 0 |
| function may not return a value | 6 | 6 | 0 |
| break/continue outside loop | 2 | 2 | 0 |

## Session 10 Results

Starting errors: 906
Final errors: 447 (51% reduction)

# Log

## 2026-01-30 (Session 3)
- Traced crashes in symbol.Table.define
- Identified complex expression codegen bug
- Applied workarounds for pointer arithmetic

## 2026-01-31 (Session 4)
- Fixed codegen bug: widening casts now use ZEXT instead of MOV
- All 481 tests pass

## 2026-02-01 (Session 5)
- Fixed chained pointer field access codegen bug
- Fixed parser string allocation
- Fixed block/identifier/use parsing
- Implemented partial module system

## 2026-01-30 (Session 6)
- Verified cmach builds, 481 tests pass, imach builds
- Ran `make mach` - 2708 semantic errors initially
- Identified module scoping issue: `collect_declarations` using `sema.symbols` instead of `current_table`
- Added `current_table` field to Sema and updated all symbol operations
- Fixed method symbol naming to use qualified names ("Type.method")
- **CRITICAL FIX:** Discovered struct copy codegen bug where Table's `current`/`root` pointers were zeroed during copy
- Applied workaround: manually copy Table fields instead of struct assignment
- Reduced "duplicate definition" errors from 1393 to 26
- Current status: 15,658 errors remaining, different types needing different fixes

## 2026-01-30 (Session 7)
- Analyzed bootstrap vs self-hosted compiler architecture
- **KEY DISCOVERY:** Bootstrap uses lazy on-demand analysis, self-hosted uses strict multi-pass
- Implemented lazy analysis in self-hosted to match bootstrap behavior
- Fixed parser to store val/var type annotation in `extra` field
- Added skip for generic types during resolution (defer until instantiation)
- Fixed cycle detection false positives in lazy analysis
- Added ModuleImport tracking for unaliased imports
- Updated type/identifier/method lookup to search imported modules
- Fixed parse_pub to create proper NODE_DECL_PUB wrapper
- Fixed find_method_symbol to match qualified method names
- Fixed binary/unary expression parsers to store operator tags
- Fixed call expression to store callee in children, args in data
- Reduced total errors from ~15,658 to ~2,679

## 2026-01-31 (Session 8)
- Fixed method receiver: added `define_receiver` function to define `this` in method scope
  - Reduced "undefined identifier" from 560 to 73
- Fixed generic parameter name allocation: parser used `current_text()` instead of `alloc_current_text_or_nil()`
  - Generic param names were garbage like "T, E] {" instead of "T"
  - Reduced "undefined type" significantly
- Fixed `def` declaration syntax: parser expected `=` but actual syntax uses `:` 
  - Fixed `parse_def` to use `TAG_COLON` instead of `TAG_EQ`
  - Eliminated all 317 "undefined type" errors
- Added skip for generic function/method checking and type resolution
  - Methods on generic types (like Option[T].unwrap) are now skipped until instantiation
- Added aliased module search in `lookup_method`
  - Methods from aliased imports (like `use allocator: std.allocator;`) are now found
- Added `check_generic_method_call` to handle calls like `a.alloc[T](count)`
  - Previously only direct generic function calls were supported
  - Now generic method calls on non-generic receivers work
- Reduced "undefined field or method" from 564 to 513

## 2026-01-31 (Session 9)
- **Fixed method lookup returning nil even when method found**
  - `lookup_method` was returning `method_sym.resolved_type` which was nil for cross-module methods
  - Changed to call `ensure_symbol_analyzed(sema, method_sym)` to lazily resolve method types
  - Eliminated all 513 "undefined field or method" errors
- **Fixed generic function lookup in check_call**
  - Added search through imported modules (unaliased and aliased) for generic function calls
  - Previously only searched `current_table`
- **Fixed check_ident for generic functions**
  - Generic functions don't have resolved_type until instantiation
  - Now returns nil without error for generic symbols, allowing call site to handle instantiation
  - Eliminated 152 "identifier type not resolved" errors
- **Fixed check_cast to get target type from correct location**
  - Parser stores cast target type in `children[1]`, not `extra`
  - Fixed `check_cast` to use `expr.get_child(1)` instead of `expr.extra`
  - Eliminated 161 "cast missing target type" errors
- **Fixed parse_ident_expr consuming generic args prematurely**
  - Parser was consuming `[T,E]` in `parse_ident_expr` before precedence loop could handle it
  - Removed `[...]` handling from `parse_ident_expr` - now handled by precedence loop's `should_parse_type_args`
  - Fixed 557 generic function calls that were incorrectly parsed
  - Reduced "missing return value" from 160 to 7
- **Fixed str indexing**
  - `str` type (kind=20, alias for `&u8`) wasn't recognized as indexable
  - Added special handling in `check_index` to allow indexing str and return u8
  - Reduced "cannot index non-array/pointer type" from 91 to 3
- **Fixed str method lookup**
  - `str` type (TY_STR) had nil name, so `lookup_method` couldn't find methods
  - Added special case to use `"str"` as type name for method lookup
  - Reduced "field access requires record or union type" from 91 to 28

Final error count: 906

## 2026-01-31 (Session 10)
- **Implemented context-aware integer literal inference**
  - Analyzed bootstrap compiler's approach (two-phase: analyze then infer)
  - Implemented simpler one-phase approach: pass expected type into `check_expr`
  - Added `check_expr_with_hint(sema, expr, expected)` function
  - For integer literals: if expected is integer type, use it; else default to i64
  - For float literals: if expected is float type, use it; else default to f64
- **Updated all key call sites to use `check_expr_with_hint`:**
  - `check_var_decl`: pass declared_type as hint to initializer
  - `check_call`: pass param_type as hint to each argument
  - `check_binary` (assignment): pass left_type as hint to right operand
  - `check_binary` (arithmetic): added `check_binary_with_hint` to pass expected type to both operands
  - `check_return`: pass current_fn_ret as hint to return expression
  - `check_unary`: added `check_unary_with_hint` to pass expected type through to operand
  - `check_struct_lit`: pass field.ty as hint to field initializer
  - `check_array_lit`: pass first element type as hint to subsequent elements
  - `check_index`: pass isize as hint for index expression
  - Parenthesized expressions: pass expected type through
- **Results after literal inference:** Reduced errors from 906 to 547 (359 fewer, 40% reduction)
  - argument type mismatch: 221 → 77 (-144)
  - initializer type mismatch: 202 → 68 (-134)
  - type mismatch in assignment: 185 → 112 (-73)
  - return type mismatch: 44 → 38 (-6)
- **Fixed "unsupported type syntax" (26 errors)**
  - Parser issue: `parse_rec` and `parse_uni` always created declaration nodes
  - When called from `parse_type()` for anonymous types, should create type nodes
  - Added `is_type` parameter to both functions
  - Call sites in `parse_type()` now pass `is_type=true`
- **Fixed "undefined type" for AllocError (37 errors)**
  - Root cause: `instantiate_generic_function` didn't switch to origin module context
  - When resolving function parameter/return types from the declaration AST,
    we need to be in the function's origin module for unqualified type names to resolve
  - Example: `Result[*T, AllocError]` in std.allocator uses unqualified `AllocError`
  - When instantiating from mach.compiler.sema, the lookup failed because
    `AllocError` doesn't exist in sema's module context
  - Fix: save caller context, switch to `generic_sym.module_path`, resolve types, restore
- **Final results:** Reduced errors from 906 to 447 (459 fewer, 51% reduction this session)
  - "undefined type" errors: 37 → 0 (eliminated)
  - "unsupported type syntax" errors: 26 → 0 (eliminated)
  - "initializer type mismatch" errors: 68 → 31 (37 more fixed by correct type resolution)

## 2026-01-31 (Session 11 - Current)
- **Fixed `str` <-> `&u8` type compatibility**
  - `str` type (kind=TY_STR) is supposed to be an alias for `&u8`
  - Added to `can_assign`: str -> &u8 and &u8 -> str assignments allowed
  - Added nil -> str assignment (str is a pointer type alias)
  - This eliminated 87 type mismatch errors (46 argument + 41 assignment)
- **Fixed `str` casting rules**
  - Added to `can_cast`: str <-> ptr, str <-> usize/u64, str <-> pointer
  - This eliminated 49 invalid cast errors
- **Fixed circular type dependency detection**
  - Root cause: when resolving `*Type` inside `Type`'s field list, `is_being_resolved` was true
  - The type shell was already created but we were erroring instead of returning it
  - Fix: when type is being resolved AND we have a shell (`resolved_type != nil`), return the shell
  - This allows self-referential types like `rec Node { children: *Node }` to work
  - Eliminated all 17 circular dependency errors
  - Fixed massive cascade of downstream errors:
    - 60 type mismatch in assignment → 0
    - 25 argument type mismatch → 0
    - 12 invalid cast → 0
    - 28 field access requires record or union type → 0
    - 29 initializer type mismatch → 2
- **Added debug output for remaining issues:**
  - Undefined identifiers: `m` (7), `section_count` (4), `ctx` (4), `i` (3), `this` (2)
  - Duplicate definitions: `usize` (3), `isize` (3), various local vars
- **Final error count: 76** (down from 447, 83% reduction)

# Next Steps

1. Debug remaining "undefined identifier" (34) - investigate scope visibility for local variables
   - Common undefined: `m` (7), `section_count` (4), `ctx` (4), `i` (3), `this` (2)

2. Fix "duplicate definition" (20) - likely type imports being registered multiple times
   - `usize` and `isize` duplicates suggest primitive types being re-registered

3. Investigate "unsupported expression" (6), "not a struct type" (6), "function may not return a value" (6)

4. Fix remaining "initializer type mismatch" (2)

# Summary

## Overall Progress
- **Session 6 start:** ~15,658 errors
- **Session 9 end:** 906 errors (94% reduction)
- **Session 10 end:** 447 errors (97% reduction from start)
- **Session 11 end:** 76 errors (99.5% reduction from start, 83% reduction this session)

Key fixes in Session 11:
1. **Fix `str` <-> `&u8` type compatibility** - str is an alias for &u8
   - Eliminated 87 type mismatches (46 arg + 41 assign) for str/&u8 conversions
   - Added nil -> str assignment support
2. **Fix `str` casting rules** - allow str <-> ptr, str <-> usize/u64, str <-> pointer
   - Eliminated 49 invalid cast errors for str conversions
3. **Fix circular type dependency handling** - return shell type for self-references
   - Eliminated all 17 circular dependency errors
   - This also fixed cascading errors: 60 assignment, 25 argument, 12 cast, 28 field access errors
   - Self-referential types via pointers (e.g., `*Type` inside `Type`) now work correctly

Key fixes in Session 10:
1. **Context-aware integer literal inference** - pass expected type hints through expression checking
   - Reduced 359 type mismatch errors by allowing numeric literals to adopt expected type
2. **Fix anonymous union/record type parsing** - use correct node kinds in type context
   - Eliminated 26 "unsupported type syntax" errors
3. **Fix generic function instantiation module context** - switch to origin module when resolving types
   - Eliminated 37 "undefined type" errors for AllocError and similar cross-module types