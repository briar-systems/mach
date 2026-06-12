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
binding or expression context that constrains their type. To force a
specific type, use a typed suffix.

## Char

A single character in single quotes, typed as `u8`:

```mach
val c: u8 = 'M';
```

Char escapes: `\n` `\t` `\r` `\\` `\'` `\0` `\xHH`.

## String

A sequence of characters in double quotes, producing a `*u8` pointing at
null-terminated bytes in the data segment:

```mach
val msg: *u8 = "hello, mach\n";
```

String escapes: the char escape set plus `\"`.

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

Backticks (`` ` ``) are **reserved** as a literal/quote-class character
but have no assigned syntactic role. They are preserved for a future
feature with a concrete need.

## See also

- [types.md](types.md) — what these literals are typed as
- [val-var.md](val-var.md) — using literals as binding initializers
