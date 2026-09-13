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

