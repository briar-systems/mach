---
name: "mach-language"
description: "Concise Mach language reference: syntax, types, conventions, and short examples for authoring valid Mach code."
---

## Purpose
A compact, actionable summary to help agents and contributors write correct Mach code and use the compiler toolchain for fast validation.

---

## Language basics
- Single-line comments only: `# comment`
- Keywords are summarized below; for full syntax details, see `doc/keywords.md`.
- One file = one module. Module path follows file path (e.g., `src/driver/pipeline.mach` → `project.driver.pipeline`).

---

## Keywords (quick)

| keyword | meaning | common form |
| --- | --- | --- |
| `use` | import another module (optionally under an alias) | `use [alias:] project.module;` |
| `pub` | export a top-level symbol from the current module | `pub val x: i32 = 0;` |
| `ext` | declare an external (FFI) binding | `ext "C:printf" printf: fun(fmt: *u8, ...) i32;` |
| `def` | define a type alias | `def Age: u32;` |
| `rec` | define a record (struct-like) type | `rec Point { x: f32; y: f32; }` |
| `uni` | define a tagged union (optionally generic) | `uni Result[T] { ok: T; err: T; }` |
| `val` | declare an initialized, immutable binding | `val pi: f32 = 3.14;` |
| `var` | declare a mutable binding (initializer optional) | `var i: i32 = 0;` |
| `fun` | declare a function or method | `fun add(a: i32, b: i32) i32 { ret a + b; }` |
| `ret` | return from a function (with an expression if required) | `ret 0;` |
| `if` | conditional branch | `if (cond) { ... }` |
| `or` | else / else-if branch for `if` | `or (cond) { ... } or { ... }` |
| `for` | loop (conditional or infinite) | `for (cond) { ... }` / `for { ... }` |
| `cnt` | continue to next loop iteration | `cnt;` |
| `brk` | break out of the nearest loop | `brk;` |
| `fin` | deferred statement (runs when scope exits) | `fin cleanup();` |
| `masm` | inline masm block (backend/target-specific) | `masm { ... }` |

---

## Types & pointers
- Primitives: `u8` `u16` `u32` `u64` `i8` `i16` `i32` `i64` `f32` `f64` `ptr` `str`.
- Builtin value: `nil` (null pointer).
- Pointers: `*T` (mutable pointer), `&T` (read-only pointer).
- Arrays: `[N]T`.
- Constructs: `rec { ... }` (records), `uni { ... }` (tagged unions).
- Address-of: `?x` — when applied to a `var` it yields a `*T`; when applied to a `val` it yields an `&T`. Deref: `@ptr`.
- Casting: `value::TargetType` is a pure bit reinterpretation; sizes must match exactly (this does not perform numeric conversion).
- No type inference; types must be explicit.
- Literal coercion only happens at declaration.

---

## Expressions (common)
- Field access / module access: `obj.field`
- Indexing: `arr[i]` (arrays and pointer-like values)
- Calls: `func(a, b)` and methods via dot: `obj.method()`
- Generic instantiation/calls: `Type[T]` and `func[T](...)`
- Composite literals: `Type { field: value, ... }` and `Type[T] { ... }`
- Cast: `value::TargetType` (bit reinterpret; same-size only)
- Address-of / deref: `?expr`, `@expr`

---

## Control flow & methods
- Conditional and loops: `if (cond) { .. } or { .. }`, `for (cond) { .. }`, `for { .. }` (infinite).
- Deferred finalizers: `fin stmt;` (deferred in scope).
- Methods with receivers: `fun (this: *T) method() { ... }` — method calls auto-convert between value and pointer receivers as needed.
- Mach does NOT have tertiary (C-style) for loop syntax. `for` is effectively equivalent to `while` in C.
- Mach does NOT have a `while` statement.

---

## Entry point convention
- Use for native executables:

```mach
use std.runtime;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
  ret 0;
}
```
Note the `$main.symbol` assignment to set the entry point symbol (defined as and required by the standard library runtime).

---

## Minimal examples
- Simple `main`:

```mach
use std.runtime;
$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
  ret 0;
}
```

- Pointer & method examples:

```mach
rec Counter { value: i32; }
fun (this: *Counter) inc() { this.value = this.value + 1; }

var n: i32 = 3;
val pn: *i32 = ?n;
val m: i32 = @pn;
```

---

## Documentation
- Mach documentation follows a pattern as seen below:

```mach
# Summary and description of the function, method, record, etc.
# ---
# param:  description of parameter
# param2: description of another parameter
# ret:    description of return value
```

- The `---` line separates the summary from parameter/return docs and is not required where not relevant (e.g no parameters). The same goes for the entirety of the parameter/return section.
- In the above example, `param` and `param2` are placeholder names; use the actual parameter names. `ret` is NOT a placeholder; always use `ret` for return value documentation.
- Attempt to keep parameter descriptions aligned for readability (as shown above with space-padding).
