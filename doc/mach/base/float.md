[mach](../README.md) / [base](README.md) / **float**

# mach.base.float

binary floating point as the compiler computes it: formats, rounding, NaNs and decimals

comptime folds and float literals are computed in f64 and rounded once to the format of their
width, which is exact for every narrower format. a NaN is carried bit-exactly, and what a target
makes of one is its `Nans` rule in mach.target.facts. a decimal is parsed with its fit reported,
and a value is printed back as the shortest decimal that rounds to it in its own format.

Source: [`src/base/float.mach`](../../../src/base/float.mach)

| Type | | |
| :--- | :--- | :--- |
| [`FloatWidth`](#floatwidth) | `def` |  |
| [`Format`](#format) | `rec` | a binary interchange format: its significand precision, the hidden bit counted, and the width of its exponent field |
| [`Fit`](#fit) | `def` |  |
| [`Rounded`](#rounded) | `rec` | a decimal rounded to one format: the value, carried exactly in an f64, and how it fit |
| [`Shortest`](#shortest-1) | `rec` | the shortest decimal that rounds back to a value in its format: digit values 0..9, and the exponent that places them, value = 0.d1 d2 . |

| On `FloatWidth` | |
| :--- | :--- |
| [`FLOAT_W_NONE`](#floatwidth), [`FLOAT_W_16`](#floatwidth), [`FLOAT_W_32`](#floatwidth), [`FLOAT_W_64`](#floatwidth) | |
| [`width_name`](#width_name) | the float type a width names, as its source spelling |
| [`width_of_bits`](#width_of_bits) | the width of a float `bits` wide, FLOAT_W_NONE for a width no float type has |
| [`round_at`](#round_at) | `f` rounded once, to nearest with ties to even, to the format of `w`, and carried back exactly in an f64 |
| [`bits_at`](#bits_at) | the encoding at `w` of `v`, a value already representable at `w`: the bits `:~` reads |
| [`value_of_bits`](#value_of_bits) | the value an encoding at `w` holds, carried exactly in an f64: the value `:~` makes |
| [`widths_agree`](#widths_agree) | whether two widths can meet: equal, or one of them FLOAT_W_NONE |
| [`unify_width`](#unify_width) | the width two agreeing widths meet at: `a`, or `b` when `a` is FLOAT_W_NONE |
| [`parse_decimal`](#parse_decimal) | the decimal `s`, its first `len` bytes, rounded once to nearest with ties to even at `w` by std's parse and carried exactly in an f64, none when the parse refuses the text |
| [`shortest`](#shortest) | the shortest round-trip decimal of a positive finite nonzero value representable in the format of `w`, by exact bignum arithmetic (Steele and White's dragon4) over that format's own neighbours, ties read the way round-to-nearest-even reads them back |

| On `Fit` | |
| :--- | :--- |
| [`FIT_EXACT`](#fit), [`FIT_INEXACT`](#fit), [`FIT_OVERFLOW`](#fit), [`FIT_UNDERFLOW`](#fit) | |

| On `Rounded` | |
| :--- | :--- |
| [`fit_of`](#fit_of) | how the decimal mant * 10^exp10 fit `value`, the value it was rounded to: exact when the two are equal, an overflow when it rounded to infinity and an underflow when a nonzero decimal rounded to zero |

| On `Shortest` | |
| :--- | :--- |
| [`shortest_text`](#shortest_text) | the shortest digits spelled as a float literal, positional for a modest exponent and in scientific form otherwise, terminated, in a SHORTEST_TEXT_CAP buffer |

| Functions | |
| :--- | :--- |
| [`f64_bits`](#f64_bits) | the binary64 encoding of `f` |
| [`bits_f64`](#bits_f64) | the f64 a binary64 encoding holds |
| [`f64_neg`](#f64_neg) | `f` with its sign bit flipped, a NaN and a zero included |
| [`f64_signbit`](#f64_signbit) | whether the sign bit of `f` is set, as it is for -0 and a negative NaN |
| [`f64_to_f32_bits`](#f64_to_f32_bits) | the binary32 encoding of binary64 bits that carry a binary32 value: rounded to nearest even, a NaN keeping every bit binary32 has |
| [`f32_bits_to_f64_bits`](#f32_bits_to_f64_bits) | a binary32 encoding as the binary64 bits of the same value: exact, a subnormal normalised and a NaN keeping every bit it has |

| Constants | |
| :--- | :--- |
| [`SHORTEST_TEXT_CAP`](#constants) | |

---

## FloatWidth

```mach
pub def FloatWidth: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `FLOAT_W_NONE` | `0` |  |
| `FLOAT_W_16` | `16` |  |
| `FLOAT_W_32` | `32` |  |
| `FLOAT_W_64` | `64` |  |

### width_name

```mach
pub fun width_name(w: FloatWidth) str
```

the float type a width names, as its source spelling

### width_of_bits

```mach
pub fun width_of_bits(bits: u32) FloatWidth
```

the width of a float `bits` wide, FLOAT_W_NONE for a width no float type has

### round_at

```mach
pub fun round_at(f: f64, w: FloatWidth) f64
```

`f` rounded once, to nearest with ties to even, to the format of `w`, and carried back
exactly in an f64. binary64 holds every narrower format exactly with more than twice its
precision plus two bits, so a `+ - * /` of two values at `w` computed in f64 and rounded
here is the correctly rounded result at `w`

### bits_at

```mach
pub fun bits_at(v: f64, w: FloatWidth) u64
```

the encoding at `w` of `v`, a value already representable at `w`: the bits `:~` reads

### value_of_bits

```mach
pub fun value_of_bits(bits: u64, w: FloatWidth) f64
```

the value an encoding at `w` holds, carried exactly in an f64: the value `:~` makes

### widths_agree

```mach
pub fun widths_agree(a: FloatWidth, b: FloatWidth) bool
```

whether two widths can meet: equal, or one of them FLOAT_W_NONE

### unify_width

```mach
pub fun unify_width(a: FloatWidth, b: FloatWidth) FloatWidth
```

the width two agreeing widths meet at: `a`, or `b` when `a` is FLOAT_W_NONE

### parse_decimal

```mach
pub fun parse_decimal(s: str, len: usize, w: FloatWidth) opt[f64]
```

the decimal `s`, its first `len` bytes, rounded once to nearest with ties to even at `w` by
std's parse and carried exactly in an f64, none when the parse refuses the text

### shortest

```mach
pub fun shortest(v: f64, w: FloatWidth) Shortest
```

the shortest round-trip decimal of a positive finite nonzero value representable in the
format of `w`, by exact bignum arithmetic (Steele and White's dragon4) over that format's
own neighbours, ties read the way round-to-nearest-even reads them back

| Parameter | |
| :--- | :--- |
| `v` | the value, carried in an f64 |
| `w` | the width whose neighbours bound the round trip |
| **returns** | its shortest digits and their exponent |

## Format

```mach
pub rec Format {
    precision: u32;
    exp_bits:  u32;
}
```

a binary interchange format: its significand precision, the hidden bit counted, and the
width of its exponent field. every width a float type can take has one row in format_of

## Fit

```mach
pub def Fit: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `FIT_EXACT` | `0` |  |
| `FIT_INEXACT` | `1` |  |
| `FIT_OVERFLOW` | `2` |  |
| `FIT_UNDERFLOW` | `3` |  |

## Rounded

```mach
pub rec Rounded {
    value: f64;
    fit:   Fit;
}
```

a decimal rounded to one format: the value, carried exactly in an f64, and how it fit.
an overflow carries +inf and an underflow of a nonzero decimal carries +0

### fit_of

```mach
pub fun fit_of(value: f64, mant: *std.math.bignum.Big, exp10: i64, trunc: bool) Rounded
```

how the decimal mant * 10^exp10 fit `value`, the value it was rounded to: exact when the two are
equal, an overflow when it rounded to infinity and an underflow when a nonzero decimal rounded to
zero. `trunc` says a nonzero digit past mant was dropped, so the decimal is never exact

## Shortest

```mach
pub rec Shortest {
    digits: [20]u8;
    len:    usize;
    exp10:  i64;
}
```

the shortest decimal that rounds back to a value in its format: digit values 0..9, and the
exponent that places them, value = 0.d1 d2 .. dn * 10^exp10

### shortest_text

```mach
pub fun shortest_text(s: *Shortest, buf: *u8) str
```

the shortest digits spelled as a float literal, positional for a modest exponent and in
scientific form otherwise, terminated, in a SHORTEST_TEXT_CAP buffer

## Functions

### f64_bits

```mach
pub fun f64_bits(f: f64) u64
```

the binary64 encoding of `f`

### bits_f64

```mach
pub fun bits_f64(bits: u64) f64
```

the f64 a binary64 encoding holds

### f64_neg

```mach
pub fun f64_neg(f: f64) f64
```

`f` with its sign bit flipped, a NaN and a zero included

### f64_signbit

```mach
pub fun f64_signbit(f: f64) bool
```

whether the sign bit of `f` is set, as it is for -0 and a negative NaN

### f64_to_f32_bits

```mach
pub fun f64_to_f32_bits(d: u64) u32
```

the binary32 encoding of binary64 bits that carry a binary32 value: rounded to
nearest even, a NaN keeping every bit binary32 has

### f32_bits_to_f64_bits

```mach
pub fun f32_bits_to_f64_bits(s: u32) u64
```

a binary32 encoding as the binary64 bits of the same value: exact, a subnormal normalised
and a NaN keeping every bit it has

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SHORTEST_TEXT_CAP` | `usize` | `48` |  |

