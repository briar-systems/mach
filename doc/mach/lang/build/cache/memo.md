# mach.lang.build.cache.memo

the per-file digest memo: what a source file's bytes digest to, remembered
under the path, size, modification time and identity the file had when it
was hashed, so an unchanged file costs a stat and not a hash. the memo lives
at `<out>/.cache/digests` and belongs to one compiler identity, whose
parser decides the surface digests; a memo written by another compiler, or
one that is missing, truncated or damaged, reads as empty. the layout is
`"MDM2"`, the 32-byte compiler digest, the entry count, the entries, then
the sha-256 of everything before it. an entry is its path (length then
bytes), its size, modification seconds and nanoseconds, its 41-byte
identity, then its five digests, every number little endian

## rec Digests

```mach
pub rec Digests;
```

what the memo remembers of one file: the digest of its bytes, of its surface
without positions, of its surface with the positions of what it omits, and of
its code without its test declarations, without and with their positions

## rec Stamp

```mach
pub rec Stamp;
```

what a file looked like when it was observed

## rec Memo

```mach
pub rec Memo;
```

## fun init

```mach
pub fun init(a: *std_allocator.Allocator, compiler: *[32]u8) Memo;
```

## fun dnit

```mach
pub fun dnit(m: *Memo);
```

## fun stamp

```mach
pub fun stamp(p: str) opt[Stamp];
```

how the file at `p` looks now, none when it cannot be opened or observed

## fun lookup

```mach
pub fun lookup(m: *Memo, p: str, s: *Stamp) opt[Digests];
```

the digests remembered for `p` when the file still looks as it did

## fun record

```mach
pub fun record(m: *Memo, p: str, s: *Stamp, d: *Digests, now: time.Time) err[std_allocator.Error];
```

remember the digests of `p` as stamped, unless it was modified so recently
(relative to `now`) that a second edit could share its timestamp

## fun decode

```mach
pub fun decode(a: *std_allocator.Allocator, compiler: *[32]u8, bytes: *u8, len: usize) res[Memo, std_allocator.Error];
```

the memo `bytes` hold for `compiler`: every entry, or none at all when the
bytes are not a complete memo of that compiler. err only when allocation fails

## fun encode

```mach
pub fun encode(m: *Memo, a: *std_allocator.Allocator, out_len: *usize) res[*u8, std_allocator.Error];
```

the memo as bytes, allocated in a and owned by the caller

## fun load

```mach
pub fun load(a: *std_allocator.Allocator, compiler: *[32]u8, file: str) res[Memo, std_allocator.Error];
```

the memo stored at `file`, empty when there is none or it cannot be read

## fun store

```mach
pub fun store(m: *Memo, out_dir: str, file: str) err[outcome.Fail];
```

write the memo to `file` through a sibling temporary when it changed

