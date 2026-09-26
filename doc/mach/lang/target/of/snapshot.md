# mach.lang.target.of.snapshot

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, img: *of.ObjectImage, out_bytes: **u8, out_len: *usize) err[fail.Fail];
```

the snapshot of img; the caller owns the bytes, allocated in alloc

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize,
out: *of.ObjectImage) res[bool, fail.Fail];
```

the image a snapshot holds, owned in alloc with its names in itn: false when
the bytes are truncated, trail, or do not describe a valid image. err only
when allocation fails

