# mach.lang.build.cache.record

the record an `obj/` object carries for the object cache (see
target/of/record for the section it rides in): the key the object was built
under, what the engine reads from a module's lowered ir besides its object,
so a module whose object is reused never has to lower, and the image codegen
produced, which is what a reused module links: an object format need not
spell everything a link reads. the layout is `"MCR6"`, the 32-byte key, the
32-byte digest of the module source it was built from, the scalarization
count, the test count, then per test its qualified name and its line, then
the length and bytes of the module's warnings (driver/cache encodes them),
then the length and bytes of the image's snapshot (see target/of/snapshot),
every number a little-endian u32 and every name its length then its bytes

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
pub fun facts_dnit(alloc: *std_allocator.Allocator, f: *Facts);
```

## fun record_dnit

```mach
pub fun record_dnit(alloc: *std_allocator.Allocator, r: *Record);
```

## fun encode

```mach
pub fun encode(alloc: *std_allocator.Allocator, itn: *intern.Interner, key: *[32]u8, facts: *Facts,
image: *target_of.ObjectImage, out_bytes: **u8, out_len: *u32) err[fail.Fail];
```

the record for an object built under `key` whose module lowered to `facts`
and generated `image`; the caller owns the bytes, allocated in alloc

## fun decode

```mach
pub fun decode(alloc: *std_allocator.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[opt[Record], fail.Fail];
```

the record in `bytes`, or none when it is missing, truncated or malformed:
an object whose record cannot be read is not reused. err only when
allocation fails

