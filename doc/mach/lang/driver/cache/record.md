# mach.lang.driver.cache.record

the record an `obj/` object carries for the object cache in its `.mach.cache`
section: the key the object was built under and what the engine reads from a
module's lowered unit besides its object, so a module whose object is reused
never has to lower. the object itself is what mink reads back from the file.
the layout is `"MCR7"`, the 32-byte key, the 32-byte digest of the module
source it was built from, the test count, then per test its qualified name and
its line, then the length and bytes of the module's warnings (driver/cache
encodes them), every number a little-endian u32 and every name its length then
its bytes

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

release recorded facts; nil facts hold nothing

## fun record_dnit

```mach
pub fun record_dnit(alloc: *A.Allocator, r: *Record);
```

release a record; a nil record holds nothing

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, itn: *intern.Interner, key: *[32]u8, facts: *Facts,
out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

the record for an object built under `key` whose module lowered to `facts`;
the caller owns the bytes, allocated in alloc

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[opt[Record], fail.Fail];
```

the record in `bytes`, or none when it is missing, truncated or malformed:
an object whose record cannot be read is not reused. err only when
allocation fails

