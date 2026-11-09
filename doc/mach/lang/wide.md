# mach.lang.wide

128-bit integer carrier over two u64 limbs

the compiler cannot yet spell u128 in its own source, so this module is the
exact arithmetic behind 128-bit literals, comptime evaluation, constant
folding and IR constants. every operation is total: wrapping where the name
says so, flagged where the caller must know, never undefined.

## rec Wide

```mach
pub rec Wide;
```

## fun zero

```mach
pub fun zero() Wide;
```

## fun make

```mach
pub fun make(lo: u64, hi: u64) Wide;
```

## fun from_u64

```mach
pub fun from_u64(v: u64) Wide;
```

## fun from_i64

```mach
pub fun from_i64(v: i64) Wide;
```

## fun is_zero

```mach
pub fun is_zero(a: Wide) bool;
```

## fun eq

```mach
pub fun eq(a: Wide, b: Wide) bool;
```

## fun lt_u

```mach
pub fun lt_u(a: Wide, b: Wide) bool;
```

## fun lt_s

```mach
pub fun lt_s(a: Wide, b: Wide) bool;
```

## fun is_negative

```mach
pub fun is_negative(a: Wide) bool;
```

## fun add

```mach
pub fun add(a: Wide, b: Wide) Wide;
```

## fun sub

```mach
pub fun sub(a: Wide, b: Wide) Wide;
```

## fun neg

```mach
pub fun neg(a: Wide) Wide;
```

## fun max_u

```mach
pub fun max_u() Wide;
```

## fun min_s

```mach
pub fun min_s() Wide;
```

## fun not

```mach
pub fun not(a: Wide) Wide;
```

## fun and

```mach
pub fun and(a: Wide, b: Wide) Wide;
```

## fun or

```mach
pub fun or (a: Wide, b: Wide) Wide;
```

## fun xor

```mach
pub fun xor(a: Wide, b: Wide) Wide;
```

## fun mul_u64

```mach
pub fun mul_u64(a: u64, b: u64) Wide;
```

## fun mul

```mach
pub fun mul(a: Wide, b: Wide) Wide;
```

## rec QuotRem

```mach
pub rec QuotRem;
```

a quotient and its remainder

## fun divmod_u

```mach
pub fun divmod_u(a: Wide, b: Wide) opt[QuotRem];
```

a / b and a % b unsigned, none when b is zero

## fun divmod_s

```mach
pub fun divmod_s(a: Wide, b: Wide) opt[QuotRem];
```

a / b and a % b signed, rounding toward zero, none when b is zero

## fun quot_rem_u

```mach
pub fun quot_rem_u(a: Wide, b: Wide) QuotRem;
```

a / b and a % b unsigned. total over a nonzero `b`: a zero divisor is a
broken precondition of the caller, and it stops the compiler

## fun quot_rem_s

```mach
pub fun quot_rem_s(a: Wide, b: Wide) QuotRem;
```

a / b and a % b signed, rounding toward zero, total over a nonzero `b` as
quot_rem_u is

## fun shl

```mach
pub fun shl(a: Wide, n: u32) Wide;
```

## fun shr_u

```mach
pub fun shr_u(a: Wide, n: u32) Wide;
```

## fun shr_s

```mach
pub fun shr_s(a: Wide, n: u32) Wide;
```

## fun trunc

```mach
pub fun trunc(a: Wide, bits: u32) Wide;
```

## fun sext

```mach
pub fun sext(a: Wide, bits: u32) Wide;
```

## fun fits_unsigned

```mach
pub fun fits_unsigned(a: Wide, bits: u32) bool;
```

## fun fits_signed

```mach
pub fun fits_signed(a: Wide, bits: u32) bool;
```

whether `a`, read as signed, lies in the range of a signed `bits`-wide integer, `bits` in 1..128

## fun bit_length

```mach
pub fun bit_length(a: Wide) u32;
```

## fun trailing_zeros

```mach
pub fun trailing_zeros(a: Wide) u32;
```

## fun is_pow2

```mach
pub fun is_pow2(a: Wide) bool;
```

## fun mul_add_small

```mach
pub fun mul_add_small(a: Wide, m: u64, d: u64, overflow: *bool) Wide;
```

## fun format_u

```mach
pub fun format_u(a: Wide, buf: *u8, cap: usize) usize;
```

## val FORMAT_CAP

```mach
pub val FORMAT_CAP: usize = 41
```

room for any formatted value: a sign, 39 decimal digits and the terminator

## fun text_u

```mach
pub fun text_u(a: Wide, buf: *u8) str;
```

the decimal text of an unsigned value, terminated, in a FORMAT_CAP buffer

## fun text_s

```mach
pub fun text_s(a: Wide, buf: *u8) str;
```

the decimal text of a signed value, terminated, in a FORMAT_CAP buffer

