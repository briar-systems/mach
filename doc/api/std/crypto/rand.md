# std.crypto.rand

## fun fill

```mach
pub fun fill(buf: *u8, len: usize) err[io_error.Error];
```

fill a buffer with cryptographic random bytes

the entropy source is read until len bytes are written. on failure the
bytes written before the refusal stay in buf (the completed prefix is
observable) and the error carries the native code under OP_READ.

buf: destination buffer
len: number of bytes to fill
ret: ok, or the entropy source's refusal

## fun u64

```mach
pub fun u64() u64;
```

return a random 64-bit unsigned integer

aborts if the OS entropy source fails.

ret: random value

## fun u32

```mach
pub fun u32() u32;
```

return a random 32-bit unsigned integer

aborts if the OS entropy source fails.

ret: random value

