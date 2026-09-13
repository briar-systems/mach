# std.system.file_identity

## def Access

```mach
pub def Access: u8
```

maximum access authorized while retaining identity, never a required upgrade

## val METADATA_ONLY

```mach
pub val METADATA_ONLY:   Access = 0
```

## val READ_AUTHORIZED

```mach
pub val READ_AUTHORIZED: Access = 1
```

## val LINUX_LOCAL

```mach
pub val LINUX_LOCAL:   u8    = 1
```

## val DARWIN_LOCAL

```mach
pub val DARWIN_LOCAL:  u8    = 2
```

## val WINDOWS_LOCAL

```mach
pub val WINDOWS_LOCAL: u8    = 3
```

## val TEXT_CAPACITY

```mach
pub val TEXT_CAPACITY: usize = 83
```

## rec Identity

```mach
pub rec Identity;
```

opaque to filesystem callers, native producers initialize every byte

## fun equal

```mach
pub fun equal(a: Identity, b: Identity) bool;
```

## fun encode

```mach
pub fun encode(value: Identity, out: *u8, capacity: usize) bool;
```

failure leaves the output untouched

## fun decode

```mach
pub fun decode(text: str, out: *Identity) bool;
```

only the complete current representation is accepted

## fun native

```mach
pub fun native(backend: u8, volume: u64, file: u64) Identity;
```

