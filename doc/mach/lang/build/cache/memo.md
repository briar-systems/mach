# mach.lang.build.cache.memo

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
pub fun init(a: *A.Allocator, compiler: *[32]u8) Memo;
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
pub fun record(m: *Memo, p: str, s: *Stamp, d: *Digests, now: time.Time) err[A.Error];
```

remember the digests of `p` as stamped, unless it was modified so recently
(relative to `now`) that a second edit could share its timestamp

## fun decode

```mach
pub fun decode(a: *A.Allocator, compiler: *[32]u8, bytes: *u8, len: usize) res[Memo, A.Error];
```

the memo `bytes` hold for `compiler`: every entry, or none at all when the
bytes are not a complete memo of that compiler. err only when allocation fails

## fun encode

```mach
pub fun encode(m: *Memo, a: *A.Allocator, out_len: *usize) res[*u8, A.Error];
```

the memo as bytes, allocated in a and owned by the caller

## fun load

```mach
pub fun load(a: *A.Allocator, compiler: *[32]u8, file: str) res[Memo, A.Error];
```

the memo stored at `file`, empty when there is none or it cannot be read

## fun store

```mach
pub fun store(m: *Memo, file: str) err[outcome.Fail];
```

write the memo to `file` through a sibling temporary when it changed

