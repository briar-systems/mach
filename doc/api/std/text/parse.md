# std.text.parse

## tag ParseError

```mach
pub tag ParseError: u8 {
    empty;
    syntax:   usize;
    overflow: usize;
    base:     u8;
}
```

every way a number parse can fail

empty: nothing to parse (an empty string, or only whitespace)
syntax: the byte at the payload offset is not part of a valid number
overflow: the value does not fit; payload is the offset at which the
          overflow was detected
base: the requested base is outside 2..36; payload is that base

## fun parse_u64

```mach
pub fun parse_u64(s: str, base: u8) res[u64, ParseError];
```

parse an unsigned 64-bit integer prefix from a string

stops at the first character that is not a valid digit for the given base,
returning the value parsed so far. use parse_u64_exact to require full
input consumption.

s: string to parse
base: numeric base (2-36), or 0 for auto-detect (0x, 0b, 0o prefixes)
ret: the parsed value, or an error message

## fun parse_u64_exact

```mach
pub fun parse_u64_exact(s: str, base: u8) res[u64, ParseError];
```

parse an unsigned 64-bit integer from a string, requiring full consumption

returns an error if there are any characters remaining after the number.
leading whitespace is also rejected.

s: string to parse
base: numeric base (2-36), or 0 for auto-detect (0x, 0b, 0o prefixes)
ret: the parsed value, or an error message

## fun parse_i64

```mach
pub fun parse_i64(s: str, base: u8) res[i64, ParseError];
```

parse a signed 64-bit integer prefix from a string

delegates to parse_u64 for the unsigned portion, so it inherits the same
prefix-parsing behavior. use parse_i64_exact to require full input
consumption.

s: string to parse
base: numeric base (2-36), or 0 for auto-detect (0x, 0b, 0o prefixes)
ret: the parsed value, or an error message

## fun parse_i64_exact

```mach
pub fun parse_i64_exact(s: str, base: u8) res[i64, ParseError];
```

parse a signed 64-bit integer from a string, requiring full consumption

returns an error if there are any characters remaining after the number.
leading whitespace is also rejected.

s: string to parse
base: numeric base (2-36), or 0 for auto-detect (0x, 0b, 0o prefixes)
ret: the parsed value, or an error message

## fun parse_f64

```mach
pub fun parse_f64(s: str) res[f64, ParseError];
```

stops at the first character that is not part of a valid float. use
parse_f64_exact to require full input consumption, or parse_f64_len for a span
that is not null-terminated.

s: string to parse
ret: the parsed value, or an error message

## fun parse_f64_len

```mach
pub fun parse_f64_len(s: str, len: usize) res[f64, ParseError];
```

parse a 64-bit floating point number prefix from the first `len` bytes of `s`

like parse_f64 but bounded by an explicit length instead of a null terminator,
so it is safe on a byte span embedded in a larger buffer (e.g. a zero-copy
slice into a parse input). stops at the first byte that is not part of a valid
float, or at `len`.

s: the bytes to parse
len: the number of bytes available in s
ret: the parsed value, or an error message

## fun parse_f64_exact

```mach
pub fun parse_f64_exact(s: str) res[f64, ParseError];
```

parse a 64-bit floating point number from a string, requiring full consumption

handles optional sign, integer part, fractional part, and exponent.
also handles special values inf, -inf, nan, +inf, +nan.
returns an error if there are any characters remaining after the number.
leading whitespace is also rejected.

s: string to parse
ret: the parsed value, or an error message

