# mach.lang.target.isa.common

## fun is_mir_compare

```mach
pub fun is_mir_compare(op: mir.MirOpcode) bool;
```

## fun is_mir_divide

```mach
pub fun is_mir_divide(op: mir.MirOpcode) bool;
```

## fun is_mir_float_conv

```mach
pub fun is_mir_float_conv(op: mir.MirOpcode) bool;
```

## fun is_float_width

```mach
pub fun is_float_width(w: u8) bool;
```

the byte widths a scalar float takes

## fun popcount32

```mach
pub fun popcount32(m: u32) u32;
```

## fun u64_to_dec

```mach
pub fun u64_to_dec(n: u64, dst: *u8) usize;
```

writes n in decimal at dst, which holds at least 20 bytes, and returns the length

