# mach.lang.build.cache.record

## rec TestFact

```mach
pub rec TestFact;
```

## rec Facts

```mach
pub rec Facts;
```

## rec Record

```mach
pub rec Record;
```

## fun facts_dnit

```mach
pub fun facts_dnit(alloc: *A.Allocator, f: *Facts);
```

## fun record_dnit

```mach
pub fun record_dnit(alloc: *A.Allocator, r: *Record);
```

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, itn: *intern.Interner, key: *[32]u8, facts: *Facts,
image: *of.ObjectImage, out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

the record for an object built under `key` whose module lowered to `facts`
and generated `image`; the caller owns the bytes, allocated in alloc

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[opt[Record], fail.Fail];
```

the record in `bytes`, or none when it is missing, truncated or malformed:
an object whose record cannot be read is not reused. err only when
allocation fails

