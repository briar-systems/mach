# Literals

## Numeric

| Form | Example |
|---|---|
| Decimal | `42` |
| Hex | `0xDEAD` |
| Binary | `0b1010` |
| Octal | `0o755` |
| Underscores | `1_000_000` |
| Scientific | `1.5e10` |
| Typed suffix | `7i64`, `255u8`, `2.5f64` |

Numeric literals without a suffix are untyped until they flow into a
binding or expression context that constrains their type. With no such
context an integer literal is `i64` and a float literal is `f64`.

### Typed suffixes

A suffix is the spelling of the primitive type the literal has: `u8`,
`u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64` on an integer literal, and
`f32` or `f64` on a float literal. It works with every radix and with
digit separators (`0xFFu8`, `0b1010u16`, `1_000_000u32`, `1.5e2f32`).

A suffixed literal *is* that type, in the same way a variable of that type
is. It is not a hint, and it is never silently retyped:

```mach
val a: u32 = 7u32;      # fine
val b: u64 = 7u32;      # error: type mismatch, expected u64, found u32
val c: f64 = 1.5f32;    # error: type mismatch, expected f64, found f32
val d: u32 = 7u32 + 1;  # fine: the unsuffixed 1 takes u32 from the other operand
val e: u32 = 7u32 + 8u64;  # error: operands are different types
```

Because a suffixed literal is already typed, it is what types the elements
of a pack tail, where nothing else constrains them:

```mach
fun sink(va: ...) u32 { var acc: u32 = 0; $each a in va { acc = acc + a; } ret acc; }
val total: u32 = sink(7u32, 8u32);
```

A literal outside the range of the type its suffix declares is rejected,
with the range named. A leading `-` is part of the range check, so the
most negative value of a signed type is written the way it reads:

```mach
val a: i8  = -128i8;   # fine
val b: i8  = 128i8;    # error: literal 128 is out of range for i8 (-128..127)
val c: u8  = 256u8;    # error: literal 256 is out of range for u8 (0..255)
val d: u32 = -1u32;    # error: literal -1 is out of range for u32 (0..4294967295)
```

A float suffix needs a float literal: the fractional part or the exponent
is what makes it one, so `1f32` is an error and `1.0f32` and `1e0f32` are
the ways to write it. Any other suffix spelling is an error naming the
suffixes that exist.

Every phase agrees on the type a suffix declares: compile-time evaluation,
type checking, constant folding, and the emitted code all read the same
type and the same value.

## Char

A single character in single quotes, typed as `u8`:

```mach
val c: u8 = 'M';
```

Char escapes: `\n` `\t` `\r` `\\` `\'` `\"` `\0` `\xHH`.

This set is deliberately minimal — there is no `\a` `\b` `\f` `\v`. Any other
byte, control characters included, is written `\xHH` (e.g. `\x08` for
backspace, `\x07` for bell).

## String

A sequence of characters in double quotes, producing a `*u8` pointing at
null-terminated bytes in the data segment:

```mach
val msg: *u8 = "hello, mach\n";
```

String escapes: the same set.

A string literal is a single line. There is no multi-line string syntax —
long content uses `\n` escapes or lives in external files.

## `nil`

The `nil` keyword is the null-address literal. With no context it types as
`*u8`; it coerces to any pointer-like type — a raw `ptr`, a typed `*T`, or a
function type `fun(...)` (a function value is a code address, so the null
address is a valid null callback). The coercion is uniform across every
position a value flows into: globals, locals, record fields, array elements,
call arguments, and return slots.

```mach
def F: fun(u32);
var p: *i64 = nil;          # null pointer
var cb: fun(u32) = nil;     # null function pointer
var k:  F = nil::F;         # the cast spelling works too
val absent: u8 = p == nil;  # nil compares against any pointer-like value
```

nil coerces only to pointer-like targets; assigning it to a non-pointer slot
(`var x: u32 = nil;`) is a type error.

## Backticks

The backtick (`` ` ``) is a real token with no grammar production. It
delimited decorators through v2.3.0, so a backtick at decorator position is
a migration error naming `#[...]`; anywhere else it is a syntax error. See
[grammar.md](grammar.md#unexpected-characters).

## See also

- [types.md](types.md) — what these literals are typed as
- [val-var.md](val-var.md) — using literals as binding initializers
