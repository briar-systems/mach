# mach.lang.wire

the compiler's one binary codec. a Sink puts little-endian numbers, LEB128
numbers and byte runs, and a Source takes them back. the formats the
compiler writes and reads (the link carrier, object snapshots, cache records,
the digest memo, recorded warnings, query fingerprints and cache keys) are
each a sequence of these. both fail sticky: once an operation fails every
later one does nothing, so a format writes or reads a whole record and
checks once at the end.

a sink measures, writing nothing, fills storage its caller sized (usually
by measuring first), grows storage it owns, or feeds a sha-256 digest, and
a format writes the same sequence to each. a source reads a bounded span and
never reads past it.

## def Destination

```mach
pub def Destination: u8
```

where a sink's bytes go

## val MEASURE

```mach
pub val MEASURE: Destination = 0
```

## val FIXED

```mach
pub val FIXED:   Destination = 1
```

## val GROWN

```mach
pub val GROWN:   Destination = 2
```

## val DIGEST

```mach
pub val DIGEST:  Destination = 3
```

## rec Sink

```mach
pub rec Sink;
```

## fun measure

```mach
pub fun measure() Sink;
```

a sink that counts the bytes a sequence takes and writes none

## fun fixed

```mach
pub fun fixed(storage: *u8, cap: usize) Sink;
```

a sink that writes into `cap` bytes at `storage`; a sequence longer than
that fails the sink

## fun grown

```mach
pub fun grown(a: *A.Allocator) Sink;
```

a sink that writes into storage it grows in `a`, released by dnit or
handed off by release

## fun digest

```mach
pub fun digest() Sink;
```

a sink that feeds a sha-256 digest, read by digest_final

## fun dnit

```mach
pub fun dnit(s: *Sink);
```

## fun refuse

```mach
pub fun refuse(s: *Sink, f: fail.Fail);
```

fail the sink with `f` unless it already failed

## fun failure

```mach
pub fun failure(s: *Sink) opt[fail.Fail];
```

## fun bytes

```mach
pub fun bytes(s: *Sink) *u8;
```

the bytes a fixed or grown sink holds

## fun put_bytes

```mach
pub fun put_bytes(s: *Sink, data: *u8, len: usize);
```

## fun put_u8

```mach
pub fun put_u8(s: *Sink, v: u8);
```

## fun put_u16

```mach
pub fun put_u16(s: *Sink, v: u16);
```

## fun put_u32

```mach
pub fun put_u32(s: *Sink, v: u32);
```

## fun put_u64

```mach
pub fun put_u64(s: *Sink, v: u64);
```

## fun put_unsigned

```mach
pub fun put_unsigned(s: *Sink, v: u64);
```

## fun put_signed

```mach
pub fun put_signed(s: *Sink, v: i64);
```

## fun patch_u32

```mach
pub fun patch_u32(s: *Sink, at: usize, v: u32);
```

overwrite the u32 at `at`, already put, as a format fills a length it only
knows once the rest is put; a measuring sink has nothing to overwrite

## fun put_sized

```mach
pub fun put_sized(s: *Sink, data: *u8, len: usize);
```

a byte run's u32 length, then its bytes

## fun release

```mach
pub fun release(s: *Sink) res[*u8, fail.Fail];
```

hand off a grown sink's bytes, exactly its length, owned by the caller in
the sink's allocator; nil when it holds none. the sink is left empty

## fun digest_final

```mach
pub fun digest_final(s: *Sink, out: *[32]u8);
```

the digest of everything a digest sink was fed

## rec Source

```mach
pub rec Source;
```

## fun source

```mach
pub fun source(data: *u8, len: usize) Source;
```

## fun reject

```mach
pub fun reject(r: *Source);
```

fail the source: what it read does not form a record

## fun remaining

```mach
pub fun remaining(r: *Source) usize;
```

## fun done

```mach
pub fun done(r: *Source) bool;
```

whether every read succeeded and consumed the whole span

## fun take_bytes

```mach
pub fun take_bytes(r: *Source, n: usize) *u8;
```

the next `n` bytes in place, nil when fewer remain

## fun take_into

```mach
pub fun take_into(r: *Source, dst: *u8, n: usize);
```

copy the next `n` bytes into dst

## fun take_u8

```mach
pub fun take_u8(r: *Source) u8;
```

## fun take_u16

```mach
pub fun take_u16(r: *Source) u16;
```

## fun take_u32

```mach
pub fun take_u32(r: *Source) u32;
```

## fun take_u64

```mach
pub fun take_u64(r: *Source) u64;
```

## fun take_unsigned

```mach
pub fun take_unsigned(r: *Source) u64;
```

## fun take_signed

```mach
pub fun take_signed(r: *Source) i64;
```

## fun take_count

```mach
pub fun take_count(r: *Source, min: usize) u32;
```

a u32 count whose entries, at least `min` bytes each, fit in what remains

## fun take_sized

```mach
pub fun take_sized(r: *Source, out_len: *usize) *u8;
```

a byte run's u32 length then its bytes in place; the length in `out_len`

