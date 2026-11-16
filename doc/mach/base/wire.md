[mach](../README.md) / [base](README.md) / **wire**

# mach.base.wire

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

<sub>source: [`src/base/wire.mach`](../../../src/base/wire.mach) · imported by 20 modules</sub>

<sub>`binary` is `std.encoding.binary` · `sha256` is `crypto.hash.sha256` · `fail` is [`mach.base.fail`](fail.md) · `A` is `std.allocator`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Destination`](#destination) | `def` | where a sink's bytes go |
| [`Sink`](#sink) | `rec` |  |
| [`Source`](#source) | `rec` |  |

| On `Destination` | |
| :--- | :--- |
| [`FIXED`](#destination) | |

| On `Sink` | |
| :--- | :--- |
| [`measure`](#measure) | a sink that counts the bytes a sequence takes and writes none |
| [`fixed`](#fixed) | a sink that writes into `cap` bytes at `storage`; a sequence longer than that fails the sink |
| [`grown`](#grown) | a sink that writes into storage it grows in `a`, released by dnit or handed off by release |
| [`digest`](#digest) | a sink that feeds a sha-256 digest, read by digest_final |
| [`dnit`](#dnit) | |
| [`refuse`](#refuse) | fail the sink with `f` unless it already failed |
| [`failure`](#failure) | |
| [`bytes`](#bytes) | the bytes a fixed or grown sink holds |
| [`put_bytes`](#put_bytes) · [`put_u8`](#put_u8) · [`put_u16`](#put_u16) · [`put_u32`](#put_u32) · [`put_u64`](#put_u64) · [`put_unsigned`](#put_unsigned) · [`put_signed`](#put_signed) | |
| [`patch_u32`](#patch_u32) | overwrite the u32 at `at`, already put, as a format fills a length it only knows once the rest is put; a measuring sink has nothing to overwrite |
| [`put_sized`](#put_sized) | a byte run's u32 length, then its bytes |
| [`take`](#take) | hand off a grown sink's bytes, exactly its length, owned by the caller in the sink's allocator; nil when it holds none |
| [`digest_final`](#digest_final) | the digest of everything a digest sink was fed |

| On `Source` | |
| :--- | :--- |
| [`source`](#source-1) | |
| [`reject`](#reject) | fail the source: what it read does not form a record |
| [`remaining`](#remaining) | |
| [`done`](#done) | whether every read succeeded and consumed the whole span |
| [`take_bytes`](#take_bytes) | the next `n` bytes in place, nil when fewer remain |
| [`take_into`](#take_into) | copy the next `n` bytes into dst |
| [`take_u8`](#take_u8) · [`take_u32`](#take_u32) · [`take_u64`](#take_u64) | |
| [`take_count`](#take_count) | a u32 count whose entries, at least `min` bytes each, fit in what remains |
| [`take_sized`](#take_sized) | a byte run's u32 length then its bytes in place; the length in `out_len` |

---

## Destination

```mach
pub def Destination: u8
```

where a sink's bytes go

| Name | Value | |
| :--- | :--- | :--- |
| `FIXED` | `1` |  |

## Sink

```mach
pub rec Sink {
    to:      Destination;
    storage: *u8;
    cap:     usize;
    grown:   binary.Encoder;
    state:   sha256.State;
    # the bytes put so far
    len:     usize;
    failed:  bool;
    failure: fail.Fail;
}
```

### measure

```mach
pub fun measure() Sink
```

a sink that counts the bytes a sequence takes and writes none

### fixed

```mach
pub fun fixed(storage: *u8, cap: usize) Sink
```

a sink that writes into `cap` bytes at `storage`; a sequence longer than
that fails the sink

### grown

```mach
pub fun grown(a: *A.Allocator) Sink
```

a sink that writes into storage it grows in `a`, released by dnit or
handed off by release

### digest

```mach
pub fun digest() Sink
```

a sink that feeds a sha-256 digest, read by digest_final

### dnit

```mach
pub fun dnit(s: *Sink)
```

### refuse

```mach
pub fun refuse(s: *Sink, f: fail.Fail)
```

fail the sink with `f` unless it already failed

### failure

```mach
pub fun failure(s: *Sink) opt[fail.Fail]
```

### bytes

```mach
pub fun bytes(s: *Sink) *u8
```

the bytes a fixed or grown sink holds

### put_bytes

```mach
pub fun put_bytes(s: *Sink, data: *u8, len: usize)
```

### put_u8

```mach
pub fun put_u8(s: *Sink, v: u8)
```

### put_u16

```mach
pub fun put_u16(s: *Sink, v: u16)
```

### put_u32

```mach
pub fun put_u32(s: *Sink, v: u32)
```

### put_u64

```mach
pub fun put_u64(s: *Sink, v: u64)
```

### put_unsigned

```mach
pub fun put_unsigned(s: *Sink, v: u64)
```

### put_signed

```mach
pub fun put_signed(s: *Sink, v: i64)
```

### patch_u32

```mach
pub fun patch_u32(s: *Sink, at: usize, v: u32)
```

overwrite the u32 at `at`, already put, as a format fills a length it only
knows once the rest is put; a measuring sink has nothing to overwrite

### put_sized

```mach
pub fun put_sized(s: *Sink, data: *u8, len: usize)
```

a byte run's u32 length, then its bytes

### take

```mach
pub fun take(s: *Sink) res[*u8, fail.Fail]
```

hand off a grown sink's bytes, exactly its length, owned by the caller in
the sink's allocator; nil when it holds none. the sink is left empty

### digest_final

```mach
pub fun digest_final(s: *Sink, out: *[32]u8)
```

the digest of everything a digest sink was fed

## Source

```mach
pub rec Source {
    data: *u8;
    len:  usize;
    pos:  usize;
    bad:  bool;
}
```

### source

```mach
pub fun source(data: *u8, len: usize) Source
```

### reject

```mach
pub fun reject(r: *Source)
```

fail the source: what it read does not form a record

### remaining

```mach
pub fun remaining(r: *Source) usize
```

### done

```mach
pub fun done(r: *Source) bool
```

whether every read succeeded and consumed the whole span

### take_bytes

```mach
pub fun take_bytes(r: *Source, n: usize) *u8
```

the next `n` bytes in place, nil when fewer remain

### take_into

```mach
pub fun take_into(r: *Source, dst: *u8, n: usize)
```

copy the next `n` bytes into dst

### take_u8

```mach
pub fun take_u8(r: *Source) u8
```

### take_u32

```mach
pub fun take_u32(r: *Source) u32
```

### take_u64

```mach
pub fun take_u64(r: *Source) u64
```

### take_count

```mach
pub fun take_count(r: *Source, min: usize) u32
```

a u32 count whose entries, at least `min` bytes each, fit in what remains

### take_sized

```mach
pub fun take_sized(r: *Source, out_len: *usize) *u8
```

a byte run's u32 length then its bytes in place; the length in `out_len`

