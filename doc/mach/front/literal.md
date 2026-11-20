[mach](../README.md) / [front](README.md) / **literal**

# mach.front.literal

scanning and decoding a literal's spelling: the value an integer, float, character or
string literal names and the suffix it carries, or why it does not scan

Source: [`src/front/literal.mach`](../../../src/front/literal.mach)

Uses types from [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.float`](../base/float.md), [`mach.base.source`](../base/source.md), [`mach.base.wide`](../base/wide.md), [`mach.type.kind`](../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Int`](#int) | `rec` | an integer literal's value and suffix |
| [`Float`](#float) | `rec` | a float literal's value and suffix |
| [`Refusal`](#refusal) | `rec` | why a literal does not scan: the diagnostic it reports and what it says |
| [`MultilineLine`](#multilineline) | `rec` | one line of a multiline string literal |

| On `Int` | |
| :--- | :--- |
| [`int_scan`](#int_scan) | the value and suffix of an integer literal, decimal or prefixed `0x`, `0b` or `0o`, with `_` separators skipped |

| On `Float` | |
| :--- | :--- |
| [`float_scan`](#float_scan) | a literal's value at the width its suffix names, binary64 when it has none |

| On `Refusal` | |
| :--- | :--- |
| [`char_scan`](#char_scan) | the byte a character literal names |
| [`str_scan`](#str_scan) | the bytes a string literal names, its escapes decoded, written to `out` |

| On `MultilineLine` | |
| :--- | :--- |
| [`multiline_line`](#multiline_line) | the line of the multiline string literal `text` whose `\\` starts at `at` |

| Functions | |
| :--- | :--- |
| [`float_at`](#float_at) | a literal rounded once its type is known: at its suffix's width when it has one, else at `w`, the width its context gives it |
| [`float_spells`](#float_spells) | true when the literal's significant digits, leading and trailing zeros and exponent spelling aside, are exactly the shortest decimal that rounds back to `s` |
| [`str_multiline`](#str_multiline) | whether the string literal `text` is the multiline form: lines each led by `\\` |

| Constants | |
| :--- | :--- |
| [`SUFFIX_RANGE_MSG`](#constants) | |

---

## Int

```mach
pub rec Int {
    value:  mach.base.wide.Wide;
    suffix: mach.type.kind.TypeKind;
}
```

an integer literal's value and suffix

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `mach.base.wide.Wide` | the value its digits name, held in 128 bits |
| `suffix` | `mach.type.kind.TypeKind` | the type its suffix names, LIT_SUFFIX_NONE when it has none |

### int_scan

```mach
pub fun int_scan(source: str, span: mach.base.source.Span) res[Int, Refusal]
```

the value and suffix of an integer literal, decimal or prefixed `0x`, `0b` or `0o`, with `_` separators skipped

| Parameter | |
| :--- | :--- |
| `source` | the text the span indexes |
| `span` | the literal's token |
| **returns** | the value and suffix, or a refusal for no digits, a digit outside the base, a value past 128 bits or an unknown suffix |

## Float

```mach
pub rec Float {
    value:  f64;
    suffix: mach.type.kind.TypeKind;
}
```

a float literal's value and suffix

| Field | Type | |
| :--- | :--- | :--- |
| `value` | `f64` | the value rounded at its suffix's width, binary64 when it has none |
| `suffix` | `mach.type.kind.TypeKind` | the type its suffix names, LIT_SUFFIX_NONE when it has none |

### float_scan

```mach
pub fun float_scan(source: str, span: mach.base.source.Span) res[Float, Refusal]
```

a literal's value at the width its suffix names, binary64 when it has none

## Refusal

```mach
pub rec Refusal {
    diag:    mach.base.diagnostic.kind.Kind;
    message: str;
}
```

why a literal does not scan: the diagnostic it reports and what it says

### char_scan

```mach
pub fun char_scan(source: str, span: mach.base.source.Span) res[u8, Refusal]
```

the byte a character literal names

### str_scan

```mach
pub fun str_scan(source: str, span: mach.base.source.Span, out: *u8) res[usize, Refusal]
```

the bytes a string literal names, its escapes decoded, written to `out`

| Parameter | |
| :--- | :--- |
| `source` | the text the span indexes |
| `span` | the literal's token |
| `out` | room for at least span.len bytes |
| **returns** | how many bytes were written, or why the literal does not scan |

## MultilineLine

```mach
pub rec MultilineLine {
    start: usize;
    len:   usize;
    next:  usize;
}
```

one line of a multiline string literal

| Field | Type | |
| :--- | :--- | :--- |
| `start` | `usize` | the first byte of its text, past its `\\` |
| `len` | `usize` | how many bytes its text holds, up to the line break |
| `next` | `usize` | where the next line's `\\` starts, the literal's length after the last line |

### multiline_line

```mach
pub fun multiline_line(text: View, at: usize) MultilineLine
```

the line of the multiline string literal `text` whose `\\` starts at `at`

## Functions

### float_at

```mach
pub fun float_at(source: str, span: mach.base.source.Span, w: mach.base.float.FloatWidth) res[mach.base.float.Rounded, mach.base.fail.Fail]
```

a literal rounded once its type is known: at its suffix's width when it has one, else at
`w`, the width its context gives it. every consumer of a literal's value, the parser, the
comptime evaluator, lowering, and the rule sema applies, reads it from here

| Parameter | |
| :--- | :--- |
| `source` | the text the span indexes |
| `span` | the literal's token |
| `w` | the width of the literal's type |
| **returns** | the rounded value and how it fit, or the scan's refusal |

### float_spells

```mach
pub fun float_spells(source: str, span: mach.base.source.Span, s: *mach.base.float.Shortest) bool
```

true when the literal's significant digits, leading and trailing zeros and exponent
spelling aside, are exactly the shortest decimal that rounds back to `s`

### str_multiline

```mach
pub fun str_multiline(text: View) bool
```

whether the string literal `text` is the multiline form: lines each led by `\\`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SUFFIX_RANGE_MSG` | `str` | `"integer literal is out of range for the type its suffix declares"` |  |

