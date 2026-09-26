# mach.lang.float

## def FloatWidth

```mach
pub def FloatWidth: u8
```

## val FLOAT_W_NONE

```mach
pub val FLOAT_W_NONE: FloatWidth = 0
```

## val FLOAT_W_32

```mach
pub val FLOAT_W_32:   FloatWidth = 32
```

## val FLOAT_W_64

```mach
pub val FLOAT_W_64:   FloatWidth = 64
```

## fun width_name

```mach
pub fun width_name(w: FloatWidth) str;
```

the float type a width names, as its source spelling

## fun f64_bits

```mach
pub fun f64_bits(f: f64) u64;
```

## fun bits_f64

```mach
pub fun bits_f64(bits: u64) f64;
```

## fun f64_neg

```mach
pub fun f64_neg(f: f64) f64;
```

## fun f64_signbit

```mach
pub fun f64_signbit(f: f64) bool;
```

## fun f64_to_f32_bits

```mach
pub fun f64_to_f32_bits(d: u64) u32;
```

## fun f32_bits_to_f64_bits

```mach
pub fun f32_bits_to_f64_bits(s: u32) u64;
```

## fun round_at

```mach
pub fun round_at(f: f64, w: FloatWidth) f64;
```

## fun widths_agree

```mach
pub fun widths_agree(a: FloatWidth, b: FloatWidth) bool;
```

## fun unify_width

```mach
pub fun unify_width(a: FloatWidth, b: FloatWidth) FloatWidth;
```

## rec Format

```mach
pub rec Format;
```

a binary interchange format: its significand precision, the hidden bit counted, and the
width of its exponent field. every width a float type can take has one row in format_of

## fun format_of

```mach
pub fun format_of(w: FloatWidth) Format;
```

the format a width names; FLOAT_W_NONE is the unsuffixed default, binary64

## def Fit

```mach
pub def Fit: u8
```

## val FIT_EXACT

```mach
pub val FIT_EXACT:     Fit = 0
```

## val FIT_INEXACT

```mach
pub val FIT_INEXACT:   Fit = 1
```

## val FIT_OVERFLOW

```mach
pub val FIT_OVERFLOW:  Fit = 2
```

## val FIT_UNDERFLOW

```mach
pub val FIT_UNDERFLOW: Fit = 3
```

## rec Rounded

```mach
pub rec Rounded;
```

a decimal rounded to one format: the value, carried exactly in an f64, and how it fit.
an overflow carries +inf and an underflow of a nonzero decimal carries +0

## fun round_decimal

```mach
pub fun round_decimal(mant: *bignum.Big, exp10: i64, trunc: bool, w: FloatWidth) Rounded;
```

round mant * 10^exp10 straight to the format of `w`, to nearest with ties to even, in
exact arithmetic. `trunc` says digits past mant were dropped and at least one was nonzero,
so the true value lies just above mant * 10^exp10

mant: the significant digits as an integer
exp10: the decimal exponent of mant's last digit
trunc: a nonzero digit was dropped past mant
w: the width to round to
ret: the rounded value and whether it was exact, inexact, or left the format's range

## val SHORTEST_DIGITS

```mach
pub val SHORTEST_DIGITS: usize = 20
```

## rec Shortest

```mach
pub rec Shortest;
```

the shortest decimal that rounds back to a value in its format: digit values 0..9, and the
exponent that places them, value = 0.d1 d2 .. dn * 10^exp10

## fun shortest

```mach
pub fun shortest(v: f64, w: FloatWidth) Shortest;
```

the shortest round-trip decimal of a positive finite nonzero value representable in the
format of `w`, by exact bignum arithmetic (Steele and White's dragon4) over that format's
own neighbours, ties read the way round-to-nearest-even reads them back

v: the value, carried in an f64
w: the width whose neighbours bound the round trip
ret: its shortest digits and their exponent

## val SHORTEST_TEXT_CAP

```mach
pub val SHORTEST_TEXT_CAP: usize = 48
```

## fun shortest_text

```mach
pub fun shortest_text(s: *Shortest, buf: *u8) str;
```

the shortest digits spelled as a float literal, positional for a modest exponent and in
scientific form otherwise, terminated, in a SHORTEST_TEXT_CAP buffer

