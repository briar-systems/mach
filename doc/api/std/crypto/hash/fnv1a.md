# std.crypto.hash.fnv1a

## val FNV_PRIME

```mach
pub val FNV_PRIME: u64 = 1099511628211
```

FNV-1a 64-bit prime

## val FNV_OFFSET

```mach
pub val FNV_OFFSET: u64 = 14695981039346656037
```

FNV-1a 64-bit offset basis

## val FNV_INIT

```mach
pub val FNV_INIT:   u64 = FNV_OFFSET
```

## fun step_u8

```mach
pub fun step_u8(h: u64, v: u8) u64;
```

fold one byte into the running hash

h: running hash
v: byte to fold in
ret: updated hash

## fun step_u32

```mach
pub fun step_u32(h: u64, v: u32) u64;
```

fold a 32-bit value into the running hash, byte by byte (little-endian)

h: running hash
v: value to fold in
ret: updated hash

