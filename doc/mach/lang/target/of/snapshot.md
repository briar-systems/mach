# mach.lang.target.of.snapshot

a lossless encoding of an object image: every section with its bytes, every
symbol, relocation, frame, import, indirect symbol and export request, the
machine flags, the attributes, whether codegen made it and whether its
symbols bound atoms, each with its native metadata. decoding
gives back the image that was encoded, whatever an object format can spell,
so a module read back from it links exactly as the image did. the image's
cache record is not part of it. names are their length then their bytes, a
nil name the length 0xFFFFFFFF, and every number is little-endian

## fun put_frame

```mach
pub fun put_frame(w: *wire.Sink, fr: *target_of.FrameUnwind);
```

a frame: its section, offset and step count, then per step its kind,
register, end offset and value. the link carrier writes frames this way too

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, img: *target_of.ObjectImage, out_bytes: **u8, out_len: *usize) err[fail.Fail];
```

the snapshot of img; the caller owns the bytes, allocated in alloc

## fun take_frame

```mach
pub fun take_frame(r: *wire.Source, fr: *target_of.FrameUnwind);
```

a frame put_frame wrote

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize,
out: *target_of.ObjectImage) res[bool, fail.Fail];
```

the image a snapshot holds, owned in alloc with its names in itn: false when
the bytes are truncated, trail, or do not describe a valid image. err only
when allocation fails

