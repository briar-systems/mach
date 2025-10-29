# Mach Language Specification

Mach is a statically typed, compiled systems language with explicit control over data layout and resource management. The design emphasises predictable semantics, C interoperability, and a minimal core that applications can reason about without hidden behaviour.

> NOTE: This document reflects the current language surface as implemented by the bootstrap compiler (`cmach`). Future self-hosted or third party compilers should use this document as the canonical contract.

---

## Table of contents

- [Mach Language Specification](#mach-language-specification)
  - [Table of contents](#table-of-contents)
  - [Guiding principles](#guiding-principles)
  - [Source form](#source-form)
  - [Lexical structure](#lexical-structure)
    - [Identifiers](#identifiers)
    - [Keywords](#keywords)
    - [Literals](#literals)
    - [Tokens](#tokens)
  - [Types](#types)
    - [Primitive types](#primitive-types)
    - [Typed pointers](#typed-pointers)
    - [Arrays (slices)](#arrays-slices)
    - [Records](#records)
    - [Unions](#unions)
    - [Function types](#function-types)
    - [Type aliases](#type-aliases)
    - [Generics](#generics)
  - [Declarations](#declarations)
    - [Visibility (`pub`)](#visibility-pub)
    - [Module imports (`use`)](#module-imports-use)
    - [External bindings (`ext`)](#external-bindings-ext)
    - [Type aliases (`def`)](#type-aliases-def)
    - [Records (`rec`) and unions (`uni`)](#records-rec-and-unions-uni)
    - [Variables (`var`) and values (`val`)](#variables-var-and-values-val)
    - [Functions (`fun`)](#functions-fun)
    - [Method syntax](#method-syntax)
    - [Inline assembly (`asm`)](#inline-assembly-asm)
  - [Compile-Time System](#compile-time-system)
    - [Evaluation model](#evaluation-model)
    - [Namespaces and constants](#namespaces-and-constants)
    - [Intrinsic functions](#intrinsic-functions)
    - [Interaction with `$if`](#interaction-with-if)
    - [Compile-time attributes](#compile-time-attributes)
  - [Statements](#statements)
    - [Blocks](#blocks)
    - [`if` / `or`](#if--or)
    - [`for`](#for)
    - [`ret`](#ret)
    - [`asm { ... }`](#asm---)
    - [Expression statements](#expression-statements)
  - [Expressions](#expressions)
    - [Literals](#literals-1)
    - [Identifier lookup](#identifier-lookup)
    - [Pointer operators](#pointer-operators)
    - [Arithmetic and bitwise operators](#arithmetic-and-bitwise-operators)
    - [Comparisons](#comparisons)
    - [Logical operators](#logical-operators)
    - [Unary operators](#unary-operators)
    - [Casts (`::`)](#casts-)
    - [Indexing](#indexing)
    - [Field access](#field-access)
    - [Function calls](#function-calls)
    - [Assignments](#assignments)
    - [Inline assembly expressions](#inline-assembly-expressions)
  - [Intrinsics and runtime conventions](#intrinsics-and-runtime-conventions)
    - [Variadic forwarding](#variadic-forwarding)
  - [Modules and projects](#modules-and-projects)
    - [Project configuration (`mach.toml`)](#project-configuration-machtoml)
  - [Design notes and future direction](#design-notes-and-future-direction)


---

## Guiding principles

- **Explicit control.** Mach performs no implicit heap allocation, lifetime management, or type coercion. Code specifies where data lives and how it is manipulated.
- **Predictable semantics.** Expressions have a single, well-defined type. Operations fail at compile time if their operand types do not match.
- **Interoperability with C.** Data layout mirrors C: record field alignment, pointer sizes (64-bit), and calling conventions align with the platform C ABI.
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
- Keywords and builtin types cannot be redefined (see below).

### Keywords

- `asm`: Assembly language inline block.
- `fun`: Function declaration.
- `rec`: Record declaration.
- `uni`: Union declaration.
- `var`: Mutable variable declaration.
- `val`: Immutable value declaration.
- `use`: Import statement.
- `if`: Conditional statement.
- `or`: Else-if / else branch.
- `for`: Loop statement.
- `ret`: Return from function.
- `brk`: Break from loop.
- `cnt`: Continue to next loop iteration.
- `def`: Type alias declaration.
- `ext`: External function declaration.
- `pub`: Public visibility modifier.
- `nil`: Null pointer literal.

### Literals

- **Integer literals**: decimal (`42`), hexadecimal (`0xFF`), binary (`0b1010`). Suffixes are not currently supported; the semantic analyser determines a concrete type based on context (see [Expressions](#expressions)). When no context is available, integer literals default to `i32`.
- **Float literals**: decimal with optional fraction and exponent (`3.14`, `6.02e23`). Always treated as `f64` unless directed otherwise.
- **Character literals**: `'a'`, `'\n'`. Represent unsigned bytes (`u8`). Escape sequences mirror C (`\n`, `\t`, `\\`, `\'`, `\"`, `\xNN`).
- **String literals**: double-quoted UTF-8 sequences with the same escapes as characters. Each literal lowers to a readonly `[]u8` with static storage duration.
- **Null pointer literal**: `nil` represents an untyped pointer to address zero.

### Tokens

**Operators:**

| Operator            | Description                                                          | Example                   |
| ------------------- | -------------------------------------------------------------------- | ------------------------- |
| `+` `-` `*` `/` `%` | Arithmetic: addition, subtraction, multiplication, division, modulo  | `a + b`, `x * y`          |
| `&` `\|` `^` `~`    | Bitwise: AND, OR, XOR, NOT                                           | `flags & mask`, `~bits`   |
| `<<` `>>`           | Bitwise shifts: left shift, right shift                              | `value << 4`              |
| `==` `!=`           | Equality: equal, not equal                                           | `x == y`                  |
| `<` `<=` `>` `>=`   | Relational: less than, less or equal, greater than, greater or equal | `a < b`                   |
| `&&` `\|\|` `!`     | Logical: AND, OR, NOT                                                | `valid && ready`, `!flag` |
| `=`                 | Assignment                                                           | `x = 10`                  |
| `::`                | Type cast                                                            | `value :: u64`            |
| `?`                 | Address-of (pointer from lvalue)                                     | `?variable`               |
| `@`                 | Dereference (lvalue from pointer)                                    | `@pointer`                |

**Punctuation:**

| Symbol | Purpose                                                    |
| ------ | ---------------------------------------------------------- |
| `( )`  | Function calls, grouping expressions, type parameters      |
| `[ ]`  | Array indexing, array type declarations                    |
| `{ }`  | Block statements, record/union bodies, literals            |
| `,`    | Separator for arguments, parameters, and fields            |
| `;`    | Statement terminator                                       |
| `.`    | Field access, module path separator                        |
| `:`    | Type annotations                                           |
| `...`  | Variadic function parameters, variadic argument forwarding |

---

## Types

Mach resolves every expression to a `Type`. The bootstrap compiler caches descriptors so that repeated constructions reuse existing type objects.

### Primitive types

| Name                      | Size             | Notes             |
| ------------------------- | ---------------- | ----------------- |
| `u8`, `u16`, `u32`, `u64` | 1, 2, 4, 8 bytes | Unsigned integers |
| `i8`, `i16`, `i32`, `i64` | 1, 2, 4, 8 bytes | Signed integers   |
| `f16`, `f32`, `f64`       | 2, 4, 8 bytes    | IEEE 754 floats   |
| `ptr`                     | 8 bytes          | Untyped pointer   |

### Typed pointers

`*T` references the location of a value of type `T`.

- Size/alignment: 8 bytes on 64-bit targets.
- `ptr` is a separate built-in representing “pointer to unknown”; `*ptr` therefore corresponds to C’s `void**`.
- Pointer arithmetic is limited to integer offsets and yields typed pointers.
- The address-of operator `?expr` produces a typed pointer when `expr` is an lvalue.

### Arrays (slices)

- Written as `[N]T`, `[_]T`, or `[]T` – the numeric bound is currently advisory.
- Runtime representation: `{ data: *T, len: u64 }`.
- Strings are `[]u8` slices. String literals embed a static buffer and pass slices referencing that buffer.
- Array literals:
  1. `[]T{ expr0, expr1, ... }` – constructs a new buffer with values checked against `T`.
  2. `[]T{ data_ptr, len_expr }` – slice literal; reuses existing memory. The compiler verifies the pointer matches `*T` and the length is integer-compatible with `u64`.
- Type aliases that resolve to arrays honour the same literal forms (`Alias{ ... }`).

### Records

Declared with `rec` blocks.

```mach
pub rec Vec2 {
    x: f32;
    y: f32;
}
```

- Fields require trailing semicolons.
- Layout uses C rules: each field aligned to its natural requirement, total size rounded up to the maximum field alignment.
- Record literals use `Type{ field: expression, ... }`. All fields must be initialised, and order does not matter.

### Unions

Declared with `uni` blocks and share the semantics of C unions.

```mach
uni Value {
    int: i64;
    p:   ptr;
}
```

- All fields begin at offset 0.
- Size = max(field sizes) rounded up to max alignment.
- Literals use named fields; only one field may be initialised.

### Function types

Function signatures appear both as declarations and as types:

```mach
fun main(argc: i32, argv: ptr) i32 { ... }
val fnPtr: fun(i32, ptr) i32 = main;
```

- Parameter list is ordered; each parameter has `name: Type` in declarations and only `Type` in bare function types.
- Variadic functions append `...` as the final parameter (Mach-managed varargs).
- Return type is optional; `ret;` in a function without a return type exits the function.

### Type aliases

`def Name: ExistingType;` creates a distinct type name pointing to another descriptor. Aliases participate in equality checks by identity until resolved (`type_resolve_alias` strips layers when needed).

### Generics

Mach supports generic types and functions using bracket syntax `[T]`. Generic parameters are resolved at compile time based on usage.

**Generic records:**

```mach
pub rec List[T] {
    data: *T;
    len:  u64;
    cap:  u64;
}
```

**Generic functions:**

```mach
pub fun list_new[T](cap: u64) Result[List[T], string] {
    val data: *T = mem.alloc[T](cap);
    // ...
}
```

**Generic instantiation:**

When calling generic functions or constructing generic types, the compiler accepts only explicit type parameters:

```mach
val my_list: List[i32] = list_new[i32](10).unwrap_ok();
```

Generic type parameters can appear in:
- Record and union definitions
- Function signatures (parameters and return types)
- Type aliases
- Method definitions

Type parameter constraints are not yet supported; all generic parameters are unbounded.

---

## Declarations

Mach programs are a sequence of top-level declarations interspersed with `use` statements and optional inline assembly.

### Visibility (`pub`)

- Prepend `pub` to export a symbol from the current module. Supported for `fun`, `rec`, `uni`, `def`, `var`, and `val` declarations.
- `pub` is illegal inside function bodies.

### Module imports (`use`)

```
use std.io.console;
```

- Path segments are dot-separated identifiers.
- Semicolons required.
- During semantic analysis the compiler resolves the path via `mach.toml` and the project’s module map. Public symbols from the target module are injected into the current scope.
- All public symbols from imported modules are available for use.

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

### Records (`rec`) and unions (`uni`)

Defined as shown in the [Types](#types) section. `pub rec`/`pub uni` exports them.

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
- Function bodies are required; there is no support for forward declarations without a body.

### Method syntax

Mach supports method-style function definitions that associate functions with types. Methods use dot notation for the type before the function name:

```mach
pub fun List[T].reserve(this: *List[T], additional: u64) Option[string] {
    // ...
}
```

- The first parameter is typically named `this` and represents the receiver.
- Methods can take the receiver by value (`this: Type`) or by pointer (`this: *Type`).
- Method calls use dot syntax: `my_list.reserve(10)`.
- Methods are desugared to regular functions during compilation; they are syntactic sugar for organizational clarity.
- Methods can be defined for any type, including generic types with their type parameters.

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

## Compile-Time System

Mach exposes build- and target-specific information through a dedicated compile-time subsystem. Expressions prefixed with `$` are evaluated during semantic analysis and replaced with literal values before type checking continues. If resolution fails the compiler emits an error and stops compilation.

### Evaluation model

- Only three expression forms are recognised:
  1. **Identifiers** that name well-known constants (for example `$OS_LINUX`).
  2. **Namespaced fields** of the form `$namespace.member` (for example `$target.pointer_size`).
  3. **Intrinsic function calls** such as `$size_of(Type)`.
- The evaluated value is written back into the AST as a literal. The resulting literal type must satisfy the surrounding context just like any other expression.
- Integer results use Mach unsigned integer types: enumerations are `u8` and sizes/alignment are `u64`. String results are `[]u8` slices containing UTF‑8 data owned by the compiler.
- Compile-time expressions cannot currently perform arithmetic or arbitrary function calls. Attempting to use other expression forms (casts, binary operators, user functions, etc.) produces `unsupported compile-time expression` diagnostics.

### Namespaces and constants

| Form                                                        | Type              | Description                                                                                                                                                             |
| ----------------------------------------------------------- | ----------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `$OS_LINUX`, `$OS_DARWIN`, `$OS_WINDOWS`, `$OS_BSD`         | `u8`              | Enumerated operating system identifiers.                                                                                                                                |
| `$ARCH_X86_64`, `$ARCH_ARM64`, `$ARCH_ARM`, `$ARCH_RISCV64` | `u8`              | Enumerated architecture identifiers.                                                                                                                                    |
| `$target.os`                                                | `u8`              | Active build target OS (matches the constants above).                                                                                                                   |
| `$target.arch`                                              | `u8`              | Active build target architecture (matches the constants above).                                                                                                         |
| `$target.pointer_size`                                      | `u64`             | Pointer size of the target in bytes.                                                                                                                                    |
| `$target.word_size`                                         | `u64`             | Alias for pointer size; provided for readability when dealing with word-sized data.                                                                                     |
| `$target.triple`                                            | `[]u8`            | LLVM target triple string resolved from project configuration or the host when not specified.                                                                           |
| `$build.debug`                                              | `u8`              | `1` when the build is emitting debug information, `0` otherwise.                                                                                                        |
| `$mach.version`                                             | `[]u8`            | Version string of the compiler performing the build.                                                                                                                    |
| `$OS`, `$ARCH`, `$PTR_WIDTH`                                | `u8`, `u8`, `u64` | Backwards-compatible aliases that mirror `$target.os`, `$target.arch`, and the pointer width in **bits** respectively. Prefer the modern `target.*` forms for new code. |

The `target.*` and `build.*` values reflect the target selected in `mach.toml` (or via the CLI). When cross-compiling these values describe the *output* platform, not the host machine running the compiler.

### Intrinsic functions

Four intrinsics are presently available. They are invoked by wrapping the call in a `$` prefix:

```mach
val size_bytes: u64  = $size_of(MyType);
val alignment: u64   = $align_of(MyType);
val field_offset: u64 = $offset_of(MyRecord, value);
val type_id: u64      = $type_of(MyRecord);
```

- `size_of(Type)` – evaluates to the size of `Type` in bytes as a `u64`.
- `align_of(Type)` – evaluates to the ABI alignment requirement of `Type` in bytes as a `u64`.
- `offset_of(RecordType, field)` – yields the byte offset of `field` within `RecordType` (records and unions only).
- `type_of(expr_or_type)` – produces a deterministic `u64` identifier for the resolved type. The value is derived from the type kind, size, and alignment and is intended for comparisons rather than serialization.

All intrinsics accept fully resolved types. The resulting value is constant-folded and can be used anywhere a `u64` literal is expected.

### Interaction with `$if`

`$if` conditions are ordinary compile-time expressions. They typically compare the constants above to select platform-specific code:

```mach
$if ($target.os == $OS_LINUX) {
  use sys: std.system.platform.linux.sys;
}
```

Because only identifiers and field accesses are supported, `$if` conditions must be written in terms of comparisons, conjunctions, disjunctions, and unary negation. Any failure to resolve the expression aborts compilation.

### Compile-time attributes

Symbol metadata can be attached using assignment syntax. The most common attribute is `symbol`, which overrides the exported identifier of the following declaration:

```mach
$main.symbol = "main";
fun main(args: []string) i64 {
    ret 0;
}
```

- Attribute assignments accept either string literals (`"name"`) or integer literals.
- Attributes are applied before semantic analysis so later passes observe the finalised metadata.
- Additional attributes may be added in future releases; unknown attributes currently produce diagnostics.

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

- Integer literals default to `i32` when no type context is available. When a literal appears with an expected integer type, the compiler checks whether the value fits and uses that type.
- Float literals default to `f64`.
- Character literals produce `u8` values.
- String literals produce `[]u8` slices.

### Identifier lookup

- Resolves to the nearest symbol in scope (locals, parameters, module imports, globals).
- Reassignment requires the identifier to be a mutable binding (`var` or pointer dereference).

### Pointer operators

- `?expr` – address-of. Requires `expr` to be an lvalue.
- `@expr` – dereference. `expr` must have a pointer type.

### Arithmetic and bitwise operators

Binary operators:

```
+  -  *  /  %
&  |  ^
<< >>
```

- Both operands must be numeric (`type_is_numeric`).
- For addition (`+`) and subtraction (`-`), pointer arithmetic is allowed: pointer ± integer → pointer. Pointer subtraction (pointer - pointer) is also supported and returns a `u64` representing the difference.
- Operands are promoted based on size and signedness. If operand sizes differ, the larger type is used. For same-sized integer types, unsigned types are preferred over signed types.

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

`expr.field` – `expr` must be a record or pointer to record.

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

| Intrinsic       | Description                                                                                     |
| --------------- | ----------------------------------------------------------------------------------------------- |
| `va_count()`    | Inside Mach variadic functions: returns number of variadic arguments supplied at the call site. |
| `va_arg(index)` | Returns a `ptr` to the variadic argument at `index`. Caller must cast manually.                 |

### Variadic forwarding

- Declare a function as variadic with `fun log(format: []u8, ...)
- `...` as an expression (only inside a Mach-managed variadic function) forwards the current variadic pack to another variadic function.
- Forwarding must appear as the last argument in the call.

---

## Modules and projects

- Modules are files resolved via `use`. The `ModuleManager` consults `mach.toml` and registered search paths to locate sources.
- Imports are transitive: when module A uses module B, B is compiled and its public symbols become available in A.
- Circular dependencies are currently unsupported.

### Project configuration (`mach.toml`)

Projects describe layout, build targets, and dependency mapping in TOML. Every Mach project requires a `mach.toml` file at the root.

**Basic structure:**

```toml
[project]
name = "example"
version = "0.1.0"
src = "src"
target = "native"  # or "all", or a specific target name

[dependencies]
std = "std"

[targets.linux]
triple = "x86_64-pc-linux-gnu"
entrypoint = "main.mach"
artifacts = "out/example/linux"
out = "bin/example"
opt-level = 2
emit-ast = true
emit-ir = true
emit-asm = true
emit-object = true
build-library = false
no-pie = false
```

**`[project]` section** identifies the project and default build configuration:
- `name` (required) – Project name.
- `version` (required) – Semantic version (e.g., `"0.1.0"`).
- `src` (required) – Source directory containing `.mach` files (default: `"src"`).
- `target` (required) – Default target: `"native"` (host platform), `"all"` (all defined targets), or a specific target name.

**`[targets.<name>]` sections** define per-platform build configurations:
- `triple` (required) – LLVM target triple (e.g., `"x86_64-pc-linux-gnu"`).
- `entrypoint` (required) – Main source file relative to `src`.
- `artifacts` (required) – Directory for intermediate build artifacts (AST, IR, ASM, OBJ).
- `out` (required) – Final executable or library output path.
- `opt-level` (required) – Optimization level (0–3).
- `emit-ast`, `emit-ir`, `emit-asm`, `emit-object` (required) – Control which intermediate files are generated.
- `build-library` (required) – Build as library instead of executable.
- `shared` (optional) – Build shared library if `build-library=true`.
- `no-pie` (optional) – Disable position-independent executable.
- `link` (optional, repeatable) – External libraries to link. Specify full paths to library files (`.a`, `.so`, `.dylib`, etc.). Can be repeated multiple times to link multiple libraries.

**Example with external libraries:**
```toml
[targets.linux]
triple = "x86_64-pc-linux-gnu"
entrypoint = "main.mach"
artifacts = "out/app/linux"
out = "bin/app"
opt-level = 2
emit-ast = false
emit-ir = false
emit-asm = false
emit-object = true
build-library = false
no-pie = false
link = "/usr/lib/x86_64-linux-gnu/libm.so.6"
link = "/usr/local/lib/libglfw.so"
link = "dep/custom/libutil.a"
```

**`[dependencies]` section** maps dependency names to their source locations:
```toml
[dependencies]
std = "std"              # points to ./std/
mylib = "dep/mylib/src"  # points to ./dep/mylib/src
```
- Dependencies must point to Mach source directories, not project roots.
- Import paths like `use std.io.console` are resolved via these mappings.

The compiler caches compiled modules under the `artifacts` directory to avoid recompilation.

---

## Design notes and future direction

- **Self-hosted compiler.** This repository (`mach`) will house the future compiler written in Mach. The language surface defined here must remain valid even when the implementation changes.
- **Generics.** Basic generic support is now implemented for types and functions. Future work includes constraints, trait bounds, and specialization.
- **Traits.** Traits (interfaces) are not yet designed and are not part of the current language.
- **Pattern matching and higher-level flow control.** The initial language sticks to `if`, `for`, and direct expressions. Additional constructs will require updates to this document and the semantic checker.
- **Memory safety.** Mach deliberately leaves safety to the programmer. Libraries may offer safe wrappers, but the core language keeps pointer manipulation unrestricted.
- **Toolchain evolution.** While `cmach` (the bootstrap C compiler) is the current compiler, this spec avoids referencing implementation details wherever possible. When the self-hosted version diverges, the spec should be updated to reflect intentional design choices, not incidental behaviour.
