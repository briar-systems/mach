# Literals

This document describes the literal forms available in Mach and how they are used. Literal tokens are parsed from source text; their types and exact semantics are determined by their usage context.

- [Literals](#literals)
  - [Integer Literals](#integer-literals)
  - [Floating-point Literals](#floating-point-literals)
  - [Character literals](#character-literals)
  - [String literals](#string-literals)
  - [Null literal](#null-literal)
  - [Composite literals](#composite-literals)
    - [Array typed literals](#array-typed-literals)
    - [Array literal short form](#array-literal-short-form)
    - [Record and union typed literals](#record-and-union-typed-literals)
    - [Anonymous composite literals](#anonymous-composite-literals)
    - [Generic typed literals](#generic-typed-literals)
  - [Varargs pack expression](#varargs-pack-expression)
  - [Casting literals](#casting-literals)
  - [Examples](#examples)


## Integer Literals

Integer literals can be specified in several bases using optional prefixes:
- No prefix for decimal (base 10)
- `0b` for binary (base 2)
- `0o` for octal (base 8)
- `0x` for hexadecimal (base 16)

Only digits appropriate to the base are allowed.
Underscores `_` may appear between digits as visual separators.
Prefixed integers are always integers and cannot include a decimal point.

```mach
val a: u64 = 1_000_000;
val b: i32 = 0xff;
val c: u16 = 0b1101_0101;
val d: u32 = 0o644;
```


## Floating-point Literals

Floating-point literals represent real numbers and must include a decimal point.

```mach
val x: f64 = 3.14159;
val y: f32 = 0.0;
val z: f64 = 1_234.567_89;
```


## Character literals

Character literals are enclosed in single quotes: `'a'`

Character literals evaluate to an ASCII code unit with a value of `u8`.

The following escape sequences are allowed:
- `\'` for single quote
- `\"` for double quote
- `\\` for backslash
- `\n` for newline
- `\t` for tab
- `\r` for carriage return
- `\0` for null

```mach
val nl:  u8 = '\n';
val tab: u8 = '\t';
val q:   u8 = '\"';
```


## String literals

String literals are enclosed in double quotes: `"hello"`

String literals evaluate to the built-in `str` type, which is a record with `data: &u8` and a pointer-sized `len` field. Literal data is UTF-8 encoded and lives in read-only memory.

The following escape sequences are allowed:
- `\'` for single quote
- `\"` for double quote
- `\\` for backslash
- `\n` for newline
- `\t` for tab
- `\r` for carriage return
- `\0` for null

```mach
val greeting: str = "Hello, World!\n";
val bytes:   &u8 = greeting.data;
val length:  u64 = greeting.len;
```


## Null literal

`nil` denotes a null value.

It evaluates to type of `ptr` with a value of zero.

Typical usage:
``` 
var p: *u8 = nil;
if (p == nil) {
    # ...
}
```

Use `nil` in pointer-like contexts or where a null value is acceptable.


## Composite literals

Mach supports constructing composite values with brace initializers. The initializer syntax is driven by the type on the left.

### Array typed literals

Typed array literal:
```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
```

Notes:
- Arrays `[N]T` have fixed length `N`.
- Elements are comma-separated; a trailing comma is not required.

Access:
```
val x: u8  = a[0];
var i: u64 = 0;
for (i < 3) {
  # use a[i]
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


## Casting literals

Use `::` to convert literal values to specific types when context alone is insufficient:
```
val a = (1)   :: u32;
val b = (1.0) :: f64;
val p = (?a)  :: *u32;
```


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

Array:
```
val a: [3]u8 = [3]u8{ 1, 2, 3 };
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
