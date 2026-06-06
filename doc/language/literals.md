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
`*u8`; it coerces to any pointer type. It is not assignable to a function
type — function/`nil` relationships are expressed through `==` / `!=`
comparison, not assignment.

```mach
var p: *i64 = nil;          # null pointer
val absent: u8 = p == nil;  # nil compares against any pointer
```

## Backticks

Backticks (`` ` ``) are **reserved** as a literal/quote-class character
but have no assigned syntactic role. They are preserved for a future
feature with a concrete need.

## See also

- [types.md](types.md) — what these literals are typed as
- [val-var.md](val-var.md) — using literals as binding initializers
