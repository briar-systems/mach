# std.math.bignum

## rec Big

```mach
pub rec Big;
```

a fixed-capacity unsigned big integer

## fun zero

```mach
pub fun zero(b: *Big);
```

set b to zero

## fun norm

```mach
pub fun norm(b: *Big);
```

drop leading-zero limbs so n is exact

## fun from_u64

```mach
pub fun from_u64(b: *Big, v: u64);
```

set b to v

## fun is_zero

```mach
pub fun is_zero(b: *Big) bool;
```

true iff b == 0

## fun to_u64

```mach
pub fun to_u64(b: *Big) u64;
```

low 64 bits of b (exact when b < 2^64)

## fun copy

```mach
pub fun copy(dst: *Big, src: *Big);
```

dst = src

## fun cmp

```mach
pub fun cmp(a: *Big, b: *Big) i64;
```

-1 if a < b, 0 if equal, 1 if a > b

## fun mul_small

```mach
pub fun mul_small(b: *Big, m: u32);
```

b = b * m

## fun shl

```mach
pub fun shl(b: *Big, bits: usize);
```

b = b << bits (multiply by 2^bits)

## fun add

```mach
pub fun add(a: *Big, b: *Big);
```

a = a + b

## fun sub

```mach
pub fun sub(a: *Big, b: *Big);
```

a = a - b, requires a >= b

## fun add_small

```mach
pub fun add_small(b: *Big, x: u32);
```

b = b + x

## fun mul_pow10

```mach
pub fun mul_pow10(b: *Big, k: usize);
```

b = b * 10^k

## fun bitlen

```mach
pub fun bitlen(b: *Big) usize;
```

number of significant bits (0 when b == 0)

