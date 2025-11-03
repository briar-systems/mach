# Literals

This document describes the literal forms available in Mach and how they are used. Literal tokens are parsed from source text; their types and exact semantics are determined by their usage context.

Related topics:
- Lexing and tokens: see Lexical Structure
- Types and casts: see Types and Expressions and Operators
- Composite construction: see Arrays and Slices and Records and Unions

## Overview

Mach supports:
- Integer literals (decimal, hex, binary, octal; digit separators)
- Floating-point literals (decimal with a dot; digit separators)
- Character literals (with standard escapes)
- String literals (with standard escapes; usable with slices and pointers)
- The null literal `nil`
- Composite literals (typed array/slice and record/union initializers)
- The varargs pack expression `...` in varargs contexts

In many cases, the type of a literal is resolved from context. Use an explicit cast with `::` when needed.

---

## Integer literals

Forms:
- Decimal (no prefix): `0`, `42`, `10_000`
- Hexadecimal (prefix `0x` or `0X`): `0xff`, `0XDEAD_BEEF`
- Binary (prefix `0b` or `0B`): `0b1010_0011`
- Octal (prefix `0o` or `0O`): `0o755`

Rules:
- Only digits appropriate to the base are allowed.
- Underscores `_` may appear between digits as visual separators.
- Prefixed integers are always integers and cannot include a decimal point.

Examples:
``` 
val a: u64 = 1_000_000;
val b: i32 = 0xff;
val c: u16 = 0b1101_0101;
val d: u32 = 0o644;
```

Context-driven typing:
``` 
fun f(x: u32) {
    var y: u32 = 10;     # 10 fits u32 by context
    var z: u64 = 10;     # 10 fits u64 by context
    var n: i64 = (10) :: i64;  # explicit cast if desired
}
```

---

## Floating-point literals

Form:
- Decimal with a single dot: `3.14`, `0.0`, `10_000.5`

Rules:
- At least one digit must appear before or after the dot.
- Underscores `_` may appear between digits.
- Exponential notation (e.g., `1e6`) is not part of the literal syntax.
- Base prefixes (`0x`, `0b`, `0o`) are not valid for floats.

Examples:
``` 
val pi: f64 = 3.14159;
val q:  f32 = 0.25;
val r:  f64 = 12_345.678_9;
```

When a floating literal is used without an explicit type, its type is resolved from context. Use `::` to convert explicitly:
``` 
val x: f64 = (1.0) :: f64;
```

---

## Character literals

Form:
- Single quotes: `'a'`, `'\n'`, `'\"'`

Supported escapes:
- `\'` `\"` `\\` `\n` `\t` `\r` `\0`

Examples:
``` 
val nl  = '\n';
val tab = '\t';
val q   = '\"';
```

Character literals are single code units; their final type is resolved by context. Cast explicitly when needed.

---

## String literals

Form:
- Double quotes: `"text"`

Properties:
- May contain any character including newlines.
- Supported escapes: `\'` `\"` `\\` `\n` `\t` `\r` `\0`

Examples:
``` 
val hello = "hello";
val multi = "line1\nline2";
val quote = "quote: \"";
```

Common uses with pointers and slices:
``` 
# function taking a C-style string pointer
ext "C:puts" puts: fun(*u8) i32;

# pass pointer to string data (context decides usage)
val rc: i32 = puts("Hello!\n");

# string literal to slice of u8: []u8
val s: []u8 = []u8{ 'h', 'i' };  # explicit elements
# or directly use string literal in a context expecting []u8
fun use_bytes(bytes: []u8) { /* ... */ }
use_bytes("hi");  # data pointer + length are provided
```

Note: When a string literal is used where a slice `[]T` is expected, the literal provides both the data pointer and length for the slice value.

---

## Null literal

`nil` denotes a null value.

Typical usage:
``` 
var p: *u8 = nil;
if (p == nil) {
    # ...
}
```

Use `nil` in pointer-like contexts or where a null value is acceptable.

---

## Composite literals

Mach supports constructing composite values with brace initializers. The initializer syntax is driven by the type on the left.

### Array and slice typed literals

Typed array literal:
```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
```

Typed slice (runtime-sized) literal:
```
val b: []u8 = []u8{ 10, 20, 30, 40 };
```

Notes:
- Arrays `[N]T` have fixed length `N`.
- Slices `[]T` (runtime-sized) are fat pointers with `.data` and `.len`.
- Elements are comma-separated; a trailing comma is not required.

Access:
```
val x: u8  = a[0];
var i: u64 = 0;
for (i < b.len) {
    # use b[i]
    i = i + 1;
}
```

### Array literal short form

You can also begin an array literal with the array type syntax:
```
val c = [3]u8{ 7, 8, 9 };
```

This is equivalent to providing the type on the left-hand side in a declaration.

### Record and union typed literals

Construct a record or union value with named field initializers:
```
rec Point { x: f64; y: f64; }

val origin: Point = Point{ x: 0.0, y: 0.0 };

uni Value { u: u64; f: f64; }
val v: Value = Value{ f: 3.14 };
```

Rules:
- Use `name: value` pairs inside `{ ... }`.
- Fields are comma-separated; a trailing comma is not required.
- Only named field initializers are used (no positional initializers).

### Anonymous composite literals

Anonymous record:
```
val tmp = rec { x: f64, y: f64 }{ x: 1.0, y: 2.0 };
```

Anonymous union:
```
val u = uni { i: i32, f: f32 }{ i: 123 };
```

Alternatively, for expressions that start with `rec`/`uni`, you can write:
```
val tmp_rec = rec { x: 1.0, y: 2.0 };
val tmp_uni = uni { f: 3.14 };
```
These are anonymous composite literals with named field initializers.

### Generic typed literals

Supply type arguments before the initializer:
```
rec Box[T] { value: T; }
val b: Box[u64] = Box[u64]{ value: 42 };
```

---

## Varargs pack expression

The ellipsis `...` is a token with two roles:
- In function parameter lists: marks a function as variadic when used as the last parameter.
- As an expression in argument lists: participates in varargs passing.

Example (declaration):
```
ext "C:printf" printf: fun(*u8, ...) i32;
```

Example (call):
```
# Passing values to a variadic function; the exact calling convention is target-dependent.
printf("val=%d\n", 42);
```

---

## Casting literals

Use `::` to convert literal values to specific types when context alone is insufficient:
```
val a = (1)   :: u32;
val b = (1.0) :: f64;
val p = (?a)  :: *u32;
```

---

## Examples

Integer and float:
```
val dec: u64 = 123;
val hex: u64 = 0xFF_FF;
val bin: u32 = 0b1010_0011;
val oct: u32 = 0o755;
val flt: f64 = 12_345.678_9;
```

Chars and strings:
```
val c1 = 'a';
val c2 = '\n';
val s1 = "hello";
val s2 = "line1\nline2";
```

Null and pointers:
```
var p: *u8 = nil;
if (p == nil) { /* ... */ }
```

Array and slice:
```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
val s: []u8  = []u8{ 10, 20, 30, 40 };
val first: u8 = a[0];
```

Record and union:
```
rec Point { x: f64; y: f64; }
val p0: Point = Point{ x: 0.0, y: 0.0 };

uni Value { u: u64; f: f64; }
val v: Value = Value{ f: 3.14 };
```

Anonymous composite and generics:
```
val tmp = rec { x: f64, y: f64 }{ x: 1.0, y: 2.0 };

rec Box[T] { value: T; }
val b: Box[u64] = Box[u64]{ value: 42 };
```

---

## Summary

- Choose the appropriate literal form for numbers, chars, and strings; use `_` for readability in numeric literals.
- Use `nil` for null in pointer-like contexts.
- Build arrays and slices with typed brace initializers; slices expose `.data` and `.len`.
- Construct records and unions with named fields in `{ ... }`, including anonymous forms.
- Provide explicit type arguments for generic typed literals and use `::` to cast when context is ambiguous.

For deeper details on syntax and tokenization, see Lexical Structure. For how literals participate in expressions and type conversion, see Expressions and Operators. For composite types, see Arrays and Slices and Records and Unions.