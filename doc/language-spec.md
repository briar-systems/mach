# Mach Language Specification

**Status:** Draft. This document reflects the current language surface as implemented by the bootstrap compiler (`mach-c`). Future self-hosted compilers will use this document as the canonical contract.

Mach is a statically typed, compiled systems language with explicit control over data layout and resource management. The design emphasises predictable semantics, C interoperability, and a minimal core that applications can reason about without hidden behaviour.

---

## Table of contents

1. [Guiding principles](#guiding-principles)
2. [Source form](#source-form)
3. [Lexical structure](#lexical-structure)
4. [Types](#types)
5. [Declarations](#declarations)
6. [Statements](#statements)
7. [Expressions](#expressions)
8. [Intrinsics and runtime conventions](#intrinsics-and-runtime-conventions)
9. [Modules and projects](#modules-and-projects)
10. [Standard library conventions](#standard-library-conventions)
11. [Design notes and future direction](#design-notes-and-future-direction)

---

## Guiding principles

- **Explicit control.** Mach performs no implicit heap allocation, lifetime management, or type coercion. Code specifies where data lives and how it is manipulated.
- **Predictable semantics.** Expressions have a single, well-defined type. Operations fail at compile time if their operand types do not match.
- **Interoperability with C.** Data layout mirrors C: struct field alignment, pointer sizes (64-bit), and calling conventions align with the platform C ABI.
- **Small, orthogonal feature set.** Rather than chasing syntactic sugar, Mach supports a minimal collection of statements and expressions that compose well.
- **Toolability.** The grammar avoids context-sensitive constructs. The compiler is a sequence of simple passes (lex, parse, semantic analysis, codegen).

---

## Source form

- Files use UTF-8 encoding without requiring a BOM. A BOM, if present, is treated as part of the first token.
- File extension: `.mach`.
- Whitespace separates tokens; runs of spaces/tabs/newlines are interchangeable except inside string literals and inline assembly.
- Comments:
  - Line comments start with `#` and continue to end-of-line.
  - No block comment syntax exists today.
- Statements and declarations terminate with `;` unless the construct is inherently block-shaped (`{ ... }`).
- Indentation is not enforced but the canonical style uses four spaces.

---

## Lexical structure

### Identifiers

- Start with `[A-Za-z_]`, followed by zero or more letters, digits, or underscores.
- Case-sensitive.
- Keywords cannot be redefined (see below).

### Keywords

```
as, asm, bool, brk, cnt, def, else, ext, for, fun, i8, i16, i32, i64,
if, import (reserved), nil, or, pub, ret, str, true, false, u8, u16,
u32, u64, uni, use, val, var, ...
```

`true`, `false`, and `nil` behave as literals; treat them as keywords for lexical purposes.

### Literals

- **Integer literals**: decimal (`42`), hexadecimal (`0xFF`), binary (`0b1010`). Suffixes are not currently supported; the semantic analyser determines a concrete type based on context (see [Expressions](#expressions)).
- **Float literals**: decimal with optional fraction and exponent (`3.14`, `6.02e23`). Always treated as `f64` unless directed otherwise.
- **Character literals**: `'a'`, `'\n'`. Represent unsigned bytes (`u8`). Escape sequences mirror C (`\n`, `\t`, `\\`, `\'`, `\"`, `\xNN`).
- **String literals**: double-quoted UTF-8 sequences with the same escapes as characters. Each literal lowers to a readonly `[]u8` with static storage duration.

### Tokens

Operators and punctuation:

```
+  -  *  /  %
&  |  ^  ~
<< >>
== != < <= > >=
=  ::
?  @  ->
( ) [ ] { }
,  ;  .  :
...
```

`::` is used for casts; `?` and `@` handle address-of and dereference; `->` dereferences a pointer then accesses a struct field.

---

## Types

Mach resolves every expression to a `Type`. The bootstrap compiler caches descriptors so that repeated constructions reuse existing type objects.

### Primitive types

| Name | Size | Notes |
|------|------|-------|
| `u8`, `u16`, `u32`, `u64` | 1, 2, 4, 8 bytes | Unsigned integers |
| `i8`, `i16`, `i32`, `i64` | 1, 2, 4, 8 bytes | Signed integers |
| `f16`, `f32`, `f64` | 2, 4, 8 bytes | IEEE 754 floats |
| `ptr` | 8 bytes | Untyped pointer |
| `bool` | Alias for `u8` (provided by `std.types.bool`) |

### Typed pointers

`*T` references the location of a value of type `T`.

- Size/alignment: 8 bytes on 64-bit targets.
- `ptr` is a separate built-in representing “pointer to unknown”; `*ptr` therefore corresponds to C’s `void **`.
- Pointer arithmetic is limited to integer offsets (see [Expressions](#expressions)) and yields typed pointers.
- The address-of operator `?expr` produces a typed pointer when `expr` is an lvalue.

### Arrays (slices)

- Written as `[N]T`, `[_]T`, or `[]T` – the numeric bound is currently advisory.
- Runtime representation: `{ data: *T, len: u64 }`.
- Strings are `[]u8` slices. String literals embed a static buffer and pass slices referencing that buffer.
- Array literals:
  1. `[]T{ expr0, expr1, ... }` – constructs a new buffer with values checked against `T`.
  2. `[]T{ data_ptr, len_expr }` – slice literal; reuses existing memory. The compiler verifies the pointer matches `*T` and the length is integer-compatible with `u64`.
- Type aliases that resolve to arrays honour the same literal forms (`Alias{ ... }`).

### Structs

Declared with `str` blocks.

```mach
pub str Vec2 {
    x: f32;
    y: f32;
}
```

- Fields require trailing semicolons.
- Layout uses C rules: each field aligned to its natural requirement, total size rounded up to the maximum field alignment.
- Struct literals use `Type{ field: expression, ... }`. All fields must be initialised, and order does not matter.

### Unions

Declared with `uni` blocks and share the semantics of C unions.

```mach
uni Value {
    int: i64;
    ptr: *ptr;
}
```

- All fields begin at offset 0.
- Size = max(field sizes) rounded up to max alignment.
- Literals use named fields; only one field may be initialised.

### Function types

Function signatures appear both as declarations and as types:

```mach
fun main(argc: i32, argv: *ptr) i32 { ... }
val fnPtr: fun(i32, *ptr) i32 = main;
```

- Parameter list is ordered; each parameter has `name: Type` in declarations and only `Type` in bare function types.
- Variadic functions append `...` as the final parameter (Mach-managed varargs).
- Return type is optional; `ret;` in a function without a return type exits the function.

### Type aliases

`def Name: ExistingType;` creates a distinct type name pointing to another descriptor. Aliases participate in equality checks by identity until resolved (`type_resolve_alias` strips layers when needed).

---

## Declarations

Mach programs are a sequence of top-level declarations interspersed with `use` statements and optional inline assembly.

### Visibility (`pub`)

- Prepend `pub` to export a symbol from the current module. Supported for `fun`, `str`, `uni`, `def`, `var`, and `val` declarations.
- `pub` is illegal inside function bodies.

### Module imports (`use`)

```
use std.io.console;
```

- Path segments are dot-separated identifiers.
- Semicolons required.
- Aliases (e.g. `use foo.bar as baz;`) are not yet implemented.
- During semantic analysis the compiler resolves the path via `mach.toml` and the project’s module map. Public symbols from the target module are injected into the current scope.
- Imported modules themselves can be referenced as expressions (`module.symbol`).

### External bindings (`ext`)

```
ext "C:puts" puts: fun(*u8) i32;
ext printf: fun(*u8, ...);
```

- Optional string literal of the form `"convention:symbol"`. Convention defaults to `C`; symbol defaults to the Mach identifier.
- Type expression must be a function type.
- No body allowed. Use `pub ext` to re-export.

### Type aliases (`def`)

```
def Size: u64;
```

- Introduces a new type name. Aliases may be public.

### Structs (`str`) and unions (`uni`)

Defined as shown in the [Types](#types) section. `pub str`/`pub uni` exports them.

### Variables (`var`) and values (`val`)

```
val answer: i32 = 42;
var counter: i64;
```

- Require explicit type annotations. In the absence of an initializer the variable is zero-initialised.
- `val` is immutable and must be initialised.
- `var` is mutable; initializer optional.
- Can appear at top level or inside blocks. Top-level declarations may be `pub`.

### Functions (`fun`)

```
pub fun main(args: []string) i64 {
    ret commands_dispatch(args);
}
```

- Parameters use `name: Type`. Default values are not supported.
- Optional return type. Absent return types imply no value (`ret;`).
- Variadics append `...` to the parameter list (no identifier). Forwarding uses the expression `...` as the final argument in a call.
- Nested functions are disallowed.
- Forward declarations are supported by repeating the signature without a body, followed later by the full definition.

### Inline assembly (`asm`)

```
asm {
    ; emitted verbatim into the generated module
}
```

- Available at top level and within blocks.
- `pub asm` is invalid.
- Content between braces is copied as-is into the generated LLVM IR.

---

## Statements

Mach statements appear within blocks (`{ ... }`), function bodies, and control flow constructs.

### Blocks

```
{
    stmt1;
    stmt2;
}
```

- Introduce a new scope. Variables declared inside cease to exist at the closing brace.
- Blocks can appear wherever a statement is expected and as expression statements.

### `if` / `or`

```
if (condition) {
    ...
}
or (condition2) {
    ...
}
or {
    ...
}
```

- Conditions must be “truthy” (numeric or pointer-like types). Non-conforming types trigger a compile-time error.
- The final `or { ... }` branch acts like `else` and does not require a condition.
- Each branch body is analysed in order.

### `for`

Single-clause loop:

```
var i: u64 = 0;
for (i < 10) {
    ...
    i = i + 1;
}
```

- Condition evaluated at the start of each iteration. Omitting the condition is not yet supported (write `for (true) { ... }` for an infinite loop).
- `brk;` exits the loop; `cnt;` skips to the next iteration.

### `ret`

```
ret;
ret value;
```

- Without value: only valid in functions without a declared return type (void functions).
- With value: type must be assignment-compatible with the function’s return type.

### `asm { ... }`

Inline assembly inside statements behaves the same as at top level.

### Expression statements

Any expression followed by `;` is a valid statement if it has side effects (function call, assignment, etc.). Expressions with no effect are flagged as errors to prevent accidental no-ops.

---

## Expressions

Expressions evaluate to values. The compiler tracks whether an expression is an lvalue (assignable) or rvalue (value only) to enforce semantics.

### Literals

- Integer literals default to the smallest unsigned type that fits (`u8`/`u16`/`u32`/`u64`) but are promoted to at least `u32` to avoid unexpected overflow. When a literal appears with an expected integer type, the compiler checks whether the value fits; otherwise it falls back to the inference rule.
- Float literals default to `f64`.
- Character literals produce `u8` values.
- String literals produce `[]u8` slices.

### Identifier lookup

- Resolves to the nearest symbol in scope (locals, parameters, module imports, globals).
- Reassignment requires the identifier to be a mutable binding (`var` or pointer dereference).

### Pointer operators

- `?expr` – address-of. Requires `expr` to be an lvalue.
- `@expr` – dereference. `expr` must have a pointer type.
- `expr->field` – shorthand for `@expr.field` when `expr` is a pointer to a struct.

### Arithmetic and bitwise operators

Binary operators:

```
+  -  *  /  %
&  |  ^
<< >>
```

- Both operands must be numeric (`type_is_numeric`).
- For addition (`+`) and subtraction (`-`), pointer arithmetic is allowed: pointer ± integer → pointer. Pointer differences are not yet defined.
- Operands promote via `type_promote_binary`, which performs float precedence first, then integer rank (with signedness rules). Narrow integer literals are promoted to at least 32 bits to avoid overflow.

### Comparisons

```
== != < <= > >=
```

- Operands must be numeric or pointer-like (for equality). Comparisons return `u8` (`0` or `1`).
- Pointer comparisons are limited to equality/inequality.

### Logical operators

```
&& || !
```

- `&&` and `||` operate on truthy values (numeric or pointer-like), coerce the result to `u8`.
- `!` unary negation works on truthy values.

### Unary operators

- `-expr` – numeric negation.
- `~expr` – bitwise NOT on integers.

### Casts (`::`)

`expr :: TargetType` performs explicit conversions.

- Integer ↔ integer: allowed as long as the target width can represent the value. Narrowing requires caution; values are truncated/wrapped following two’s complement rules.
- Integer ↔ float: allowed.
- Float ↔ float: allowed.
- Pointer ↔ pointer: allowed if types are pointer-like (`ptr` or `*T`).
- Pointer ↔ integer: allowed only via explicit cast.
- Array ↔ pointer: `[]T` to `*T` extracts `data`; the reverse is not implicit.

### Indexing

`expr[index]` – `expr` must be array-like (`[]T`). The result is an lvalue of type `T`.

### Field access

`expr.field` – `expr` must be a struct or pointer to struct.

### Function calls

`func(arg0, arg1, ...)`

- Callee must be a function value (identifier, function pointer, or result of an expression returning function type).
- Arguments are analysed left to right. Each argument may receive a type hint from the corresponding parameter.
- Variadic functions allow additional arguments and expose the `...` forwarding expression (see [Intrinsics](#intrinsics-and-runtime-conventions)).

### Assignments

```
lhs = rhs;
```

- `lhs` must be an lvalue.
- `rhs` must be assignment-compatible with `lhs`’s type.
- The result type of the assignment expression is the type of the left-hand side (mirroring C).

### Inline assembly expressions

`asm { ... }` inside expressions is not currently supported; use statement form.

---

## Intrinsics and runtime conventions

The bootstrap compiler recognises several built-in functions that bypass normal symbol resolution. They reside in the global namespace.

| Intrinsic | Description |
|-----------|-------------|
| `size_of(expr_or_type)` | Returns the size (bytes) of the operand’s type as `u64`. Operand is analysed but not evaluated. |
| `align_of(expr_or_type)` | Returns the alignment (bytes) of the operand’s type as `u64`. |
| `offset_of(typeExpr, fieldName)` | Struct type and field identifier. Returns the byte offset of the named field. |
| `type_of(expr)` | Produces a `u64` identifier representing the expression’s type (implementation detail; used for diagnostics/runtime tagging). |
| `va_count()` | Inside Mach variadic functions: returns number of variadic arguments supplied at the call site. |
| `va_arg(index)` | Returns a `ptr` to the variadic argument at `index`. Caller must cast manually. |

### Variadic forwarding

- Declare a function as variadic with `fun log(format: []u8, ...)
- `...` as an expression (only inside a Mach-managed variadic function) forwards the current variadic pack to another variadic function.
- Forwarding must appear as the last argument in the call.

### Runtime hooks

The bootstrap runtime exposes `mach_panic([]u8)` and `abort()` for fatal errors (bounds checks, assertions). Programs may call them directly.

---

## Modules and projects

- Modules are files resolved via `use`. The `ModuleManager` consults `mach.toml` and registered search paths to locate sources.
- Imports are transitive: when module A uses module B, B is compiled and its public symbols become available in A.
- Circular dependencies are currently unsupported.

### Project configuration (`mach.toml`)

Projects describe layout and dependency mapping in TOML. Example:

```toml
[project]
name = "example"
version = "0.0.1"
entrypoint = "main.mach"

[directories]
src-dir = "src"
dep-dir = "dep"
out-dir = "out"

[deps]
std = { path = "../mach-std", src = "src" }

[modules]
std = "std"
```

- `deps` entries point at repositories or folders containing Mach modules.
- `modules` map import prefixes (left) to dependency names (right). `use std.io.console` is resolved through this map to the `mach-std` source tree.
- The compiler caches compiled modules under `out/` to avoid recompilation.

---

## Standard library conventions

While Mach does not bake in a standard library, `mach-std` ships alongside the compiler and establishes idioms for common tasks.

### Dynamic arrays (`std.types.array`)

- Functions are prefixed with `array_` to keep the public symbol namespace stable (important for future self-hosted builds).
- Arrays maintain hidden capacity metadata directly before the slice data pointer. Functions like `array_append`, `array_reserve`, `array_shrink_to_fit`, `array_clear`, and `array_free` manage growth and lifetime.
- Always reassign the result of these helpers: each call may allocate a new backing buffer.

### Strings (`std.types.string`)

- `string` is an alias for `[]u8`.
- Helpers include equality (`string_equal`), substring operations, and indexing.

### Console I/O (`std.io.console`)

- Provides `print` and `error` formatted output functions. The format syntax mirrors a small subset of C’s `printf` (e.g., `%s`, `%d`, `%u`, `%f`).
- Implementations ultimately delegate to platform-specific write syscalls.

### Memory (`std.system.memory`)

- Thin wrappers around OS allocation primitives (`allocate`, `reallocate`, `deallocate`). Behaviour is platform-specific with Linux using `mmap/munmap`.
- Reallocation semantics mirror `realloc`: allocate new space, copy existing bytes up to the lesser of old/new size, free the old block.

Standard library modules are not special-cased by the compiler; they are ordinary Mach code built against these conventions.

---

## Design notes and future direction

- **Self-hosted compiler.** This repository (`mach`) will house the future compiler written in Mach. The language surface defined here must remain valid even when the implementation changes.
- **Generics and traits.** Not currently implemented. Any future design must preserve explicit type instantiation and keep generated code predictable.
- **Pattern matching and higher-level flow control.** The initial language sticks to `if`, `for`, and direct expressions. Additional constructs will require updates to this document and the semantic checker.
- **Memory safety.** Mach deliberately leaves safety to the programmer. Libraries may offer safe wrappers, but the core language keeps pointer manipulation unrestricted.
- **Toolchain evolution.** While `mach-c` is the current compiler, this spec avoids referencing implementation details wherever possible. When the self-hosted version diverges, the spec should be updated to reflect intentional design choices, not incidental behaviour.

---

## Appendix: Grammar sketch

A simplified grammar (EBNF-like) to guide parser implementations:

```
program      ::= { declaration } EOF

declaration  ::= use_decl | ext_decl | type_decl | var_decl | fun_decl | asm_block
use_decl     ::= "use" module_path ";"
module_path  ::= IDENT { "." IDENT }

ext_decl     ::= [ "pub" ] "ext" [ string_literal ] IDENT ":" func_type ";"

type_decl    ::= [ "pub" ] ( struct_decl | union_decl | alias_decl )
struct_decl  ::= "str" IDENT "{" { field_decl } "}"
field_decl   ::= IDENT ":" type_expr ";"
union_decl   ::= "uni" IDENT "{" { field_decl } "}"
alias_decl   ::= "def" IDENT ":" type_expr ";"

var_decl     ::= [ "pub" ] ( "var" | "val" ) IDENT ":" type_expr [ "=" expression ] ";"

fun_decl     ::= [ "pub" ] "fun" IDENT "(" [ param_list ] ")" [ type_expr ] block
param_list   ::= param { "," param }
param        ::= IDENT ":" type_expr | "..."

asm_block    ::= "asm" block

type_expr    ::= IDENT
               | "*" type_expr
               | "[]" type_expr
               | "[" expression "]" type_expr
               | "fun" "(" [ type_list ] ")" [ type_expr ]
               | "str" "{" { field_decl } "}"
               | "uni" "{" { field_decl } "}"

block        ::= "{" { statement } "}"
statement    ::= var_decl
               | if_stmt
               | for_stmt
               | asm_block
               | "ret" [ expression ] ";"
               | "brk" ";"
               | "cnt" ";"
               | expression ";"

if_stmt      ::= "if" "(" expression ")" block { "or" [ "(" expression ")" ] block }
for_stmt     ::= "for" "(" expression ")" block

expression   ::= assignment
assignment   ::= logical_or [ "=" assignment ]
logical_or   ::= logical_and { "||" logical_and }
logical_and  ::= equality { "&&" equality }
equality     ::= comparison { ( "==" | "!=" ) comparison }
comparison   ::= shift { ( "<" | "<=" | ">" | ">=" ) shift }
shift        ::= sum { ( "<<" | " >>" ) sum }
sum          ::= product { ( "+" | "-" ) product }
product      ::= unary { ( "*" | "/" | "%" ) unary }
unary        ::= ( "-" | "!" | "~" | "?" | "@" ) unary
               | postfix
postfix      ::= primary { postfix_op }
postfix_op   ::= "(" [ arg_list ] ")"
               | "[" expression "]"
               | "." IDENT
               | "->" IDENT
               | "::" type_expr
primary      ::= IDENT
               | literal
               | "(" expression ")"
               | array_literal
               | struct_literal

literal      ::= int_lit | float_lit | string_lit | char_lit | "true" | "false" | "nil"
```

This sketch omits precedence details for brevity but mirrors the current parser implementation. Update it alongside the compiler when syntax evolves.
