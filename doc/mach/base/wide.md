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
| [`Wide`](#wide) | `rec` | a 128-bit integer, read signed or unsigned by the operation |
| [`QuotRem`](#quotrem) | `rec` | a quotient and its remainder |

| On `Wide` | |
| :--- | :--- |
| [`zero`](#zero) | the value 0 |
| [`make`](#make) | the value whose low and high limbs are `lo` and `hi` |
| [`from_u64`](#from_u64) | `v` zero-extended |
| [`from_i64`](#from_i64) | `v` sign-extended |
| [`is_zero`](#is_zero) | whether the value is 0 |
| [`eq`](#eq) | whether two values hold the same bits |
| [`lt_u`](#lt_u) | whether `a` is below `b`, both read unsigned |
| [`lt_s`](#lt_s) | whether `a` is below `b`, both read signed |
| [`is_negative`](#is_negative) | whether the top bit is set, the value read signed being negative |
| [`add`](#add) | a + b, wrapping |
| [`sub`](#sub) | a - b, wrapping |
| [`neg`](#neg) | -a, wrapping, so the signed minimum stays itself |
| [`max_u`](#max_u) | the largest unsigned value, all ones |
| [`min_s`](#min_s) | the smallest signed value, only the top bit set |
| [`not`](#not) | every bit of `a` flipped |
| [`and`](#and) | the bitwise and of `a` and `b` |
| [`or`](#or) | the bitwise or of `a` and `b` |
| [`xor`](#xor) | the bitwise exclusive or of `a` and `b` |
| [`mul_u64`](#mul_u64) | the full 128-bit product of two u64s |
| [`mul`](#mul) | a * b, wrapping |
| [`quot_rem_u`](#quot_rem_u) | a / b and a % b unsigned |
| [`quot_rem_s`](#quot_rem_s) | a / b and a % b signed, rounding toward zero, total over a nonzero `b` as quot_rem_u is |
| [`shl`](#shl) | `a` shifted left by `n` bits, 0 for `n` of 128 or more |
| [`shr_u`](#shr_u) | `a` shifted right by `n` bits, zero-filled, 0 for `n` of 128 or more |
| [`shr_s`](#shr_s) | `a` shifted right by `n` bits, sign-filled, all sign bits for `n` of 128 or more |
| [`trunc`](#trunc) | the low `bits` bits of `a`, `bits` in 1..128 |
| [`sext`](#sext) | the low `bits` bits of `a` sign-extended from bit `bits` - 1, `bits` in 1..128 |
| [`fits_unsigned`](#fits_unsigned) | whether `a`, read unsigned, fits in `bits` bits |
| [`fits_signed`](#fits_signed) | whether `a`, read as signed, lies in the range of a signed `bits`-wide integer, `bits` in 1..128 |
| [`bit_length`](#bit_length) | how many bits `a` takes read unsigned, 0 for 0 |
| [`trailing_zeros`](#trailing_zeros) | how many low zero bits `a` has, 128 for 0 |
| [`is_pow2`](#is_pow2) | whether `a`, read unsigned, is a power of two |
| [`mul_add_small`](#mul_add_small) | a * m + d, unsigned, flagging overflow |
| [`format_u`](#format_u) | the decimal digits of `a` read unsigned, unterminated |
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

a 128-bit integer, read signed or unsigned by the operation

| Field | Type | |
| :--- | :--- | :--- |
| `lo` | `u64` | the low 64 bits |
| `hi` | `u64` | the high 64 bits |

### zero

```mach
pub fun zero() Wide
```

the value 0

### make

```mach
pub fun make(lo: u64, hi: u64) Wide
```

the value whose low and high limbs are `lo` and `hi`

### from_u64

```mach
pub fun from_u64(v: u64) Wide
```

`v` zero-extended

### from_i64

```mach
pub fun from_i64(v: i64) Wide
```

`v` sign-extended

### is_zero

```mach
pub fun is_zero(a: Wide) bool
```

whether the value is 0

### eq

```mach
pub fun eq(a: Wide, b: Wide) bool
```

whether two values hold the same bits

### lt_u

```mach
pub fun lt_u(a: Wide, b: Wide) bool
```

whether `a` is below `b`, both read unsigned

### lt_s

```mach
pub fun lt_s(a: Wide, b: Wide) bool
```

whether `a` is below `b`, both read signed

### is_negative

```mach
pub fun is_negative(a: Wide) bool
```

whether the top bit is set, the value read signed being negative

### add

```mach
pub fun add(a: Wide, b: Wide) Wide
```

a + b, wrapping

### sub

```mach
pub fun sub(a: Wide, b: Wide) Wide
```

a - b, wrapping

### neg

```mach
pub fun neg(a: Wide) Wide
```

-a, wrapping, so the signed minimum stays itself

### max_u

```mach
pub fun max_u() Wide
```

the largest unsigned value, all ones

### min_s

```mach
pub fun min_s() Wide
```

the smallest signed value, only the top bit set

### not

```mach
pub fun not(a: Wide) Wide
```

every bit of `a` flipped

### and

```mach
pub fun and(a: Wide, b: Wide) Wide
```

the bitwise and of `a` and `b`

### or

```mach
pub fun or (a: Wide, b: Wide) Wide
```

the bitwise or of `a` and `b`

### xor

```mach
pub fun xor(a: Wide, b: Wide) Wide
```

the bitwise exclusive or of `a` and `b`

### mul_u64

```mach
pub fun mul_u64(a: u64, b: u64) Wide
```

the full 128-bit product of two u64s

### mul

```mach
pub fun mul(a: Wide, b: Wide) Wide
```

a * b, wrapping

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

`a` shifted left by `n` bits, 0 for `n` of 128 or more

### shr_u

```mach
pub fun shr_u(a: Wide, n: u32) Wide
```

`a` shifted right by `n` bits, zero-filled, 0 for `n` of 128 or more

### shr_s

```mach
pub fun shr_s(a: Wide, n: u32) Wide
```

`a` shifted right by `n` bits, sign-filled, all sign bits for `n` of 128 or more

### trunc

```mach
pub fun trunc(a: Wide, bits: u32) Wide
```

the low `bits` bits of `a`, `bits` in 1..128

### sext

```mach
pub fun sext(a: Wide, bits: u32) Wide
```

the low `bits` bits of `a` sign-extended from bit `bits` - 1, `bits` in 1..128

### fits_unsigned

```mach
pub fun fits_unsigned(a: Wide, bits: u32) bool
```

whether `a`, read unsigned, fits in `bits` bits

### fits_signed

```mach
pub fun fits_signed(a: Wide, bits: u32) bool
```

whether `a`, read as signed, lies in the range of a signed `bits`-wide integer, `bits` in 1..128

### bit_length

```mach
pub fun bit_length(a: Wide) u32
```

how many bits `a` takes read unsigned, 0 for 0

### trailing_zeros

```mach
pub fun trailing_zeros(a: Wide) u32
```

how many low zero bits `a` has, 128 for 0

### is_pow2

```mach
pub fun is_pow2(a: Wide) bool
```

whether `a`, read unsigned, is a power of two

### mul_add_small

```mach
pub fun mul_add_small(a: Wide, m: u64, d: u64, overflow: *bool) Wide
```

a * m + d, unsigned, flagging overflow

| Parameter | |
| :--- | :--- |
| `a` | the value |
| `m` | the multiplier |
| `d` | the addend |
| `overflow` | set to whether the result left 128 bits |
| **returns** | the result, 0 when the product overflowed and the wrapped sum when only the add did |

### format_u

```mach
pub fun format_u(a: Wide, buf: *u8, cap: usize) usize
```

the decimal digits of `a` read unsigned, unterminated

| Parameter | |
| :--- | :--- |
| `a` | the value |
| `buf` | where the digits go |
| `cap` | the bytes at `buf`, at least 39 |
| **returns** | how many digits were written, 0 when `cap` is below 39 |

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

