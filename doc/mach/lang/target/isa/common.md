# mach.lang.target.isa.common

the target-independent helpers every isa backend reads: predicates over the
selected MIR, bit arithmetic, and the relocation type a backend
may leave unset

## fun is_mir_compare

```mach
pub fun is_mir_compare(op: lang_mir.MirOpcode) bool;
```

## fun is_mir_divide

```mach
pub fun is_mir_divide(op: lang_mir.MirOpcode) bool;
```

## fun is_mir_float_conv

```mach
pub fun is_mir_float_conv(op: lang_mir.MirOpcode) bool;
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

## fun frame_align_up

```mach
pub fun frame_align_up(n: u64, align: u32) res[u32, fail.Fail];
```

a frame extent rounded up to `align`, a power of two or 0 for none. the
extent is summed in 64 bits by the caller, so no part of it wraps, and an
extent past 4 GiB is refused

