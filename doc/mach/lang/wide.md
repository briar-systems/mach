# mach.lang.wide

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

## fun add_carry

```mach
pub fun add_carry(a: Wide, b: Wide, carry_out: *bool) Wide;
```

## fun sub

```mach
pub fun sub(a: Wide, b: Wide) Wide;
```

## fun sub_borrow

```mach
pub fun sub_borrow(a: Wide, b: Wide, borrow_out: *bool) Wide;
```

## fun add_signed

```mach
pub fun add_signed(a: Wide, b: Wide, overflow_out: *bool) Wide;
```

the two's complement sum, flagging a signed overflow: both operands share a
sign the sum does not

## fun sub_signed

```mach
pub fun sub_signed(a: Wide, b: Wide, overflow_out: *bool) Wide;
```

the two's complement difference, flagging a signed overflow: the operands
differ in sign and the difference does not share the minuend's

## fun neg

```mach
pub fun neg(a: Wide) Wide;
```

## fun max_u

```mach
pub fun max_u() Wide;
```

## fun max_s

```mach
pub fun max_s() Wide;
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

## fun mul_overflows_u

```mach
pub fun mul_overflows_u(a: Wide, b: Wide) bool;
```

## fun mul_overflows_s

```mach
pub fun mul_overflows_s(a: Wide, b: Wide) bool;
```

## fun mul_high_s64

```mach
pub fun mul_high_s64(a: i64, b: i64) i64;
```

## fun divmod_u

```mach
pub fun divmod_u(a: Wide, b: Wide, q: *Wide, r: *Wide) bool;
```

## fun divmod_s

```mach
pub fun divmod_s(a: Wide, b: Wide, q: *Wide, r: *Wide) bool;
```

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

## fun to_f64_u

```mach
pub fun to_f64_u(a: Wide) f64;
```

## fun to_f64_s

```mach
pub fun to_f64_s(a: Wide) f64;
```

## fun from_f64_u

```mach
pub fun from_f64_u(f: f64) Wide;
```

## fun from_f64_s

```mach
pub fun from_f64_s(f: f64) Wide;
```

## fun format_u

```mach
pub fun format_u(a: Wide, buf: *u8, cap: usize) usize;
```

## fun format_s

```mach
pub fun format_s(a: Wide, buf: *u8, cap: usize) usize;
```

## fun format_hex

```mach
pub fun format_hex(a: Wide, buf: *u8, cap: usize) usize;
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

## fun parse_u

```mach
pub fun parse_u(text: *u8, len: usize, base: u32, out: *Wide) bool;
```

