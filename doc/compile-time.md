# Compile-time Features

This document describes Mach’s compile-time facilities introduced by the `$` prefix:
- Compile-time expressions usable anywhere an expression is allowed
- Conditional compilation with `$if ... or ...`
- Built-in compile-time constants and categories
- Introspection intrinsics: `$size_of`, `$align_of`, `$offset_of`, `$type_of`
- Symbol attributes via `$Name.attribute = value`

Related topics:
- Expressions and Operators
- Types
- Records and Unions
- Modules and Visibility

## Overview

- Prefix any expression with `$` to have it evaluated at compile time. The resulting value is embedded into the program as a constant.
- Use `$if (condition) { ... } or (condition) { ... } or { ... }` to include exactly one branch at compile time.
- Use the provided compile-time constants and categories for target, build, and language information.
- Use compile-time intrinsics to query sizes, alignments, and field offsets of types.
- Set attributes on symbols (such as an exported name) with a simple assignment form.

Compile-time values can be of the following kinds:
- Integers (u64, u8)
- Strings

Notes and constraints:
- `$if` is allowed at the top level and in statement positions inside blocks and functions.
- `$if` is not allowed inside record/union field lists.
- Only expressions that can be fully determined at compile time are permitted in `$` contexts.

---

## Compile-time expressions ($expr)

Any occurrence of `$` followed by an expression is evaluated by the compiler during compilation. The result replaces the expression.

Examples:
```
val ptr_width_bits: u64 = $target.word_size;     # e.g., 64
val size_u64:       u64 = $size_of(u64);         # 8 on many targets
val triple:         []u8 = []u8{ };              # see strings below
```

- In statement position, a `$` expression may appear as its own statement (often used for attributes; see later).
- Expressions may include arithmetic, comparisons, logical operators, and the intrinsics listed below, as long as all inputs are compile-time known.

Strings:
- Some compile-time categories produce strings (e.g., `$target.triple`, `$mach.version`). Use them where a string is expected (for example, pass to a function expecting `*u8` or `[]u8` as appropriate).

---

## Conditional compilation ($if)

Syntax:
```
$if (condition) {
    # compiled only if condition is true
}
or (other_condition) {
    # compiled if first is false and this is true
}
or {
    # final else (no condition)
}
```

Key points:
- Exactly one branch is selected at compile time. Non-selected branches are discarded and never compiled or emitted.
- The `or` chain mirrors runtime `if/or` syntax but is strictly compile-time.
- Conditions must be compile-time evaluable. They commonly use constants (see next section) and intrinsics.

Examples:
```
# Choose implementation based on OS
$if ($target.os == $OS_WINDOWS) {
    ext "C:__imp_MessageBoxA" MessageBoxA: fun(*u8, *u8, *u8, u32) i32;
}
or ($target.os == $OS_LINUX) {
    ext "C:puts" puts: fun(*u8) i32;
}
or {
    # fallback or unsupported platform
}

# Enable code paths only in debug builds
$if ($build.debug) {
    val DBG: u64 = 1;
}
or {
    val DBG: u64 = 0;
}
```

Where allowed:
- Top-level: the chosen branch’s statements are spliced into the module’s top-level.
- Inside blocks/functions: the chosen branch’s statements appear in the generated function or block; others are removed.
- Not allowed inside `rec { ... }` or `uni { ... }` field lists.

---

## Compile-time constants and categories

You can refer to target, build, and language information via `$` identifiers. Two forms exist:
- Enum-like constants: `$OS_LINUX`, `$ARCH_X86_64`, etc.
- Category fields: `$target.os`, `$build.debug`, etc.

Available OS constants (u8):
- `$OS_LINUX`, `$OS_DARWIN`, `$OS_WINDOWS`, `$OS_BSD`

Available ARCH constants (u8):
- `$ARCH_X86_64`, `$ARCH_ARM64`, `$ARCH_ARM`, `$ARCH_RISCV64`

Target information:
- `$target.os` (u8) — one of the `$OS_*` values
- `$target.arch` (u8) — one of the `$ARCH_*` values
- `$target.pointer_size` (u64) — in bytes
- `$target.word_size` (u64) — typically equals `pointer_size`
- `$target.triple` (string) — target triple (e.g., `"x86_64-unknown-linux"`)

Build configuration:
- `$build.debug` (u8) — 1 when building in debug configuration, else 0

Language information:
- `$mach.version` (string) — language version string

Legacy constants:
- `$OS` (u8) — same as `$target.os`
- `$ARCH` (u8) — same as `$target.arch`
- `$PTR_WIDTH` (u64) — pointer width in bits

Examples:
```
# OS/arch check
$if ($target.os == $OS_LINUX && $target.arch == $ARCH_X86_64) {
    # linux/x86_64 specific
}

# pointer sizes
val ptr_bytes: u64 = $target.pointer_size;
val ptr_bits:  u64 = $PTR_WIDTH;

# build/debug guard
$if ($build.debug) {
    # assertion helpers
}

# strings
ext "C:puts" puts: fun(*u8) i32;
puts($target.triple);
```

---

## Intrinsics

All intrinsics return compile-time constants (typically u64) and are available as `$`-prefixed calls.

### $size_of(Type) -> u64
Returns the size in bytes of a type.

```
val s1: u64 = $size_of(u64);          # 8 on 64-bit
val s2: u64 = $size_of(*u8);          # pointer size in bytes
val s3: u64 = $size_of(rec { x: u8; y: u64; });
```

Rules:
- Expects exactly one argument.
- Argument must be a type: a type literal (e.g., `*u8`, `rec { ... }`) or an identifier resolvable as a type.

### $align_of(Type) -> u64
Returns the required alignment in bytes of a type.

```
val a1: u64 = $align_of(u64);
val a2: u64 = $align_of(rec { x: u8; y: u64; });
```

Rules:
- Expects exactly one type argument (same forms as `$size_of`).

### $offset_of(Type, field) -> u64
Returns the byte offset of `field` within a record or union type.

```
rec Header { tag: u32; len: u32; }
val off_len: u64 = $offset_of(Header, len);   # typically 4
```

Rules:
- The first argument must resolve to a record or union type (named or anonymous).
- The second argument must be an identifier naming a field on that type.

### $type_of(X) -> u64 (opaque)
Returns an opaque identifier for the type of `X`.

```
val t1: u64 = $type_of(u64);      # using a type directly
val t2: u64 = $type_of(42);       # using an expression
```

Notes:
- The numeric value is an implementation detail. Use for compile-time equality checks or diagnostic tables if needed, not as a runtime ABI.

---

## Symbol attributes

You can attach attributes to symbols using a compile-time assignment:

```
$SymbolName.attribute = value;
```

Where:
- `SymbolName` is the name of a declared, top-level symbol (e.g., a function or method name).
- `attribute` is an attribute key (identifier).
- `value` is a compile-time literal (string or integer).

Standard attribute:
- `symbol` (string): overrides the emitted (extern/mangled) symbol name for a function or method.

Examples:
```
# Set program entry symbol on a function
$main.symbol = "main";

# Override a method’s symbol name
$MyType__do_work.symbol = "do_work_impl";
```

Notes:
- Attribute values must be string or integer literals.
- Attributes are processed at compile time. Unknown attributes may be ignored if not recognized by the implementation.
- Use this feature sparingly; prefer default mangling unless integration requires a specific symbol name.

---

## Examples

Conditional APIs by platform:
```
$if ($target.os == $OS_WINDOWS) {
    ext "C:__imp_MessageBoxA" MessageBoxA: fun(*u8, *u8, *u8, u32) i32;
}
or ($target.os == $OS_LINUX) {
    ext "C:puts" puts: fun(*u8) i32;
}
or {
    # common fallback
}
```

Compile-time layout checks:
```
rec P { x: u8; y: u64; }

val size_P: u64 = $size_of(P);
val align_P: u64 = $align_of(P);

$if ($offset_of(P, y) == 8) {
    # expected layout
}
or {
    # adjust code or report incompatibility
}
```

Symbol override:
```
# Ensure native entry point name matches host conventions
$main.symbol = "main";

fun main() i64 {
    ret 0;
}
```

Debug-only code paths:
```
$if ($build.debug) {
    fun log(msg: *u8) {
        ext "C:puts" puts: fun(*u8) i32;
        puts(msg);
    }
}
```

---

## Rules and limitations

- `$if` is not allowed inside `rec { ... }` or `uni { ... }` field lists.
- Intrinsic arguments must match their expected forms (types for `size_of`/`align_of`, type and field identifier for `offset_of`).
- `$type_of` returns an opaque numeric identifier intended for compile-time reasoning, not as a stable runtime ABI.
- Attribute assignments accept only string or integer literals as values.

Use compile-time features to keep runtime code minimal and portable, selecting implementations and parameters according to the target and build configuration.
