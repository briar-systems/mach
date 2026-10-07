# mach.lang.fe.literal

scanning and decoding a literal's spelling: the value an integer, float, character or
string literal names and the suffix it carries, or why it does not scan

## rec Int

```mach
pub rec Int;
```

## rec Float

```mach
pub rec Float;
```

## rec Refusal

```mach
pub rec Refusal;
```

why a literal does not scan: the diagnostic it reports and what it says

## val SUFFIX_RANGE_MSG

```mach
pub val SUFFIX_RANGE_MSG: str =
"integer literal is out of range for the type its suffix declares"
```

## fun int_scan

```mach
pub fun int_scan(source: str, span: lang_source.Span) res[Int, Refusal];
```

## fun float_scan

```mach
pub fun float_scan(source: str, span: lang_source.Span) res[Float, Refusal];
```

a literal's value at the width its suffix names, binary64 when it has none

## fun float_at

```mach
pub fun float_at(source: str, span: lang_source.Span, w: float.FloatWidth) res[float.Rounded, fail.Fail];
```

a literal rounded once its type is known: at its suffix's width when it has one, else at
`w`, the width its context gives it. every consumer of a literal's value, the parser, the
comptime evaluator, lowering, and the rule sema applies, reads it from here

source: the text the span indexes
span: the literal's token
w: the width of the literal's type
ret: the rounded value and how it fit, or the scan's refusal

## fun float_spells

```mach
pub fun float_spells(source: str, span: lang_source.Span, s: *float.Shortest) bool;
```

true when the literal's significant digits, leading and trailing zeros and exponent
spelling aside, are exactly the shortest decimal that rounds back to `s`

## fun char_scan

```mach
pub fun char_scan(source: str, span: lang_source.Span) res[u8, Refusal];
```

the byte a character literal names

## fun str_scan

```mach
pub fun str_scan(source: str, span: lang_source.Span, out: *u8) res[usize, Refusal];
```

the bytes a string literal names, its escapes decoded, written to `out`

source: the text the span indexes
span: the literal's token
out: room for at least span.len bytes
ret: how many bytes were written, or why the literal does not scan

## fun escape_decode

```mach
pub fun escape_decode(p: *u8, len: usize, out_consumed: *usize) res[u8, fail.Fail];
```

