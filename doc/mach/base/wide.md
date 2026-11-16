[mach](../README.md) / [base](README.md) / **wide**

# mach.base.wide

128-bit integer carrier over two u64 limbs

the compiler cannot yet spell u128 in its own source, so this module is the
exact arithmetic behind 128-bit literals, comptime evaluation, constant
folding and IR constants. every operation is total: wrapping where the name
says so, flagged where the caller must know, never undefined.

Source: [`src/base/wide.mach`](../../../src/base/wide.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Wide`](#wide) | `rec` |  |
| [`QuotRem`](#quotrem) | `rec` | a quotient and its remainder |

| On `Wide` | |
| :--- | :--- |
| [`zero`](#zero), [`make`](#make), [`from_u64`](#from_u64), [`from_i64`](#from_i64), [`is_zero`](#is_zero), [`eq`](#eq), [`lt_u`](#lt_u), [`lt_s`](#lt_s), [`is_negative`](#is_negative), [`add`](#add), [`sub`](#sub), [`neg`](#neg), [`max_u`](#max_u), [`min_s`](#min_s), [`not`](#not), [`and`](#and), [`or`](#or), [`xor`](#xor), [`mul_u64`](#mul_u64), [`mul`](#mul) | |
| [`divmod_u`](#divmod_u) | a / b and a % b unsigned, none when b is zero |
| [`divmod_s`](#divmod_s) | a / b and a % b signed, rounding toward zero, none when b is zero |
| [`quot_rem_u`](#quot_rem_u) | a / b and a % b unsigned |
| [`quot_rem_s`](#quot_rem_s) | a / b and a % b signed, rounding toward zero, total over a nonzero `b` as quot_rem_u is |
| [`shl`](#shl), [`shr_u`](#shr_u), [`shr_s`](#shr_s), [`trunc`](#trunc), [`sext`](#sext), [`fits_unsigned`](#fits_unsigned) | |
| [`fits_signed`](#fits_signed) | whether `a`, read as signed, lies in the range of a signed `bits`-wide integer, `bits` in 1..128 |
| [`bit_length`](#bit_length), [`trailing_zeros`](#trailing_zeros), [`is_pow2`](#is_pow2), [`mul_add_small`](#mul_add_small), [`format_u`](#format_u) | |
| [`text_u`](#text_u) | the decimal text of an unsigned value, terminated, in a FORMAT_CAP buffer |
| [`text_s`](#text_s) | the decimal text of a signed value, terminated, in a FORMAT_CAP buffer |

| Constants | |
| :--- | :--- |
| [`FORMAT_CAP`](#constants) | room for any formatted value: a sign, 39 decimal digits and the terminator |

---

## Wide

```mach
pub rec Wide {
    lo: u64;
    hi: u64;
}
```

### zero

```mach
pub fun zero() Wide
```

### make

```mach
pub fun make(lo: u64, hi: u64) Wide
```

### from_u64

```mach
pub fun from_u64(v: u64) Wide
```

### from_i64

```mach
pub fun from_i64(v: i64) Wide
```

### is_zero

```mach
pub fun is_zero(a: Wide) bool
```

### eq

```mach
pub fun eq(a: Wide, b: Wide) bool
```

### lt_u

```mach
pub fun lt_u(a: Wide, b: Wide) bool
```

### lt_s

```mach
pub fun lt_s(a: Wide, b: Wide) bool
```

### is_negative

```mach
pub fun is_negative(a: Wide) bool
```

### add

```mach
pub fun add(a: Wide, b: Wide) Wide
```

### sub

```mach
pub fun sub(a: Wide, b: Wide) Wide
```

### neg

```mach
pub fun neg(a: Wide) Wide
```

### max_u

```mach
pub fun max_u() Wide
```

### min_s

```mach
pub fun min_s() Wide
```

### not

```mach
pub fun not(a: Wide) Wide
```

### and

```mach
pub fun and(a: Wide, b: Wide) Wide
```

### or

```mach
pub fun or (a: Wide, b: Wide) Wide
```

### xor

```mach
pub fun xor(a: Wide, b: Wide) Wide
```

### mul_u64

```mach
pub fun mul_u64(a: u64, b: u64) Wide
```

### mul

```mach
pub fun mul(a: Wide, b: Wide) Wide
```

### divmod_u

```mach
pub fun divmod_u(a: Wide, b: Wide) opt[QuotRem]
```

a / b and a % b unsigned, none when b is zero

### divmod_s

```mach
pub fun divmod_s(a: Wide, b: Wide) opt[QuotRem]
```

a / b and a % b signed, rounding toward zero, none when b is zero

### quot_rem_u

```mach
pub fun quot_rem_u(a: Wide, b: Wide) QuotRem
```

a / b and a % b unsigned. total over a nonzero `b`: a zero divisor is a
broken precondition of the caller, and it stops the compiler

### quot_rem_s

```mach
pub fun quot_rem_s(a: Wide, b: Wide) QuotRem
```

a / b and a % b signed, rounding toward zero, total over a nonzero `b` as
quot_rem_u is

### shl

```mach
pub fun shl(a: Wide, n: u32) Wide
```

### shr_u

```mach
pub fun shr_u(a: Wide, n: u32) Wide
```

### shr_s

```mach
pub fun shr_s(a: Wide, n: u32) Wide
```

### trunc

```mach
pub fun trunc(a: Wide, bits: u32) Wide
```

### sext

```mach
pub fun sext(a: Wide, bits: u32) Wide
```

### fits_unsigned

```mach
pub fun fits_unsigned(a: Wide, bits: u32) bool
```

### fits_signed

```mach
pub fun fits_signed(a: Wide, bits: u32) bool
```

whether `a`, read as signed, lies in the range of a signed `bits`-wide integer, `bits` in 1..128

### bit_length

```mach
pub fun bit_length(a: Wide) u32
```

### trailing_zeros

```mach
pub fun trailing_zeros(a: Wide) u32
```

### is_pow2

```mach
pub fun is_pow2(a: Wide) bool
```

### mul_add_small

```mach
pub fun mul_add_small(a: Wide, m: u64, d: u64, overflow: *bool) Wide
```

### format_u

```mach
pub fun format_u(a: Wide, buf: *u8, cap: usize) usize
```

### text_u

```mach
pub fun text_u(a: Wide, buf: *u8) str
```

the decimal text of an unsigned value, terminated, in a FORMAT_CAP buffer

### text_s

```mach
pub fun text_s(a: Wide, buf: *u8) str
```

the decimal text of a signed value, terminated, in a FORMAT_CAP buffer

## QuotRem

```mach
pub rec QuotRem {
    q: Wide;
    r: Wide;
}
```

a quotient and its remainder

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FORMAT_CAP` | `usize` | `41` | room for any formatted value: a sign, 39 decimal digits and the terminator |

