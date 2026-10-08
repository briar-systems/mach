# mach.lang.intern

## rec StrId

```mach
pub rec StrId;
```

the identity of one interned string

a record, not a `def` alias, so a string id cannot be handed where another
id or a bare integer is meant. absence is `STR_NIL`, never an `opt[StrId]`.

## val STR_NIL

```mach
pub val STR_NIL: StrId = StrId;
```

## fun str_id

```mach
pub fun str_id(index: u32) StrId;
```

## fun str_index

```mach
pub fun str_index(id: StrId) u32;
```

## fun str_same

```mach
pub fun str_same(left: StrId, right: StrId) bool;
```

## fun str_is_nil

```mach
pub fun str_is_nil(id: StrId) bool;
```

## rec Interner

```mach
pub rec Interner;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Interner;
```

## fun child_init

```mach
pub fun child_init(base: *Interner, a: *A.Allocator) Interner;
```

## fun dnit

```mach
pub fun dnit(itn: *Interner);
```

## fun text

```mach
pub fun text(itn: *Interner, id: StrId) str;
```

the text of `id`. an interner answers every id it or its base issued for as
long as it lives, so the read is total: an id it never issued, STR_NIL among
them, is a broken precondition of the caller, and it stops the compiler

## fun get

```mach
pub fun get(itn: *Interner, id: StrId) opt[str];
```

the text of `id`, or none for an id the interner never issued. it ends at
the first NUL, so a string that may hold one is read through `bytes`

## fun bytes

```mach
pub fun bytes(itn: *Interner, id: StrId) opt[View];
```

every byte of `id`, NULs included, or none for an id the interner never issued

## fun add

```mach
pub fun add(itn: *Interner, text: str) res[StrId, A.Error];
```

## fun bytes_add

```mach
pub fun bytes_add(itn: *Interner, data: str, len: usize) res[StrId, A.Error];
```

## fun range_add

```mach
pub fun range_add(itn: *Interner, source: str, offset: usize, len: usize) res[StrId, A.Error];
```

the `len` bytes of `source` from `offset`, interned. the bytes are taken as
they are, so a NUL among them is part of the string

## rec Remap

```mach
pub rec Remap;
```

## fun remap_dnit

```mach
pub fun remap_dnit(alloc: *A.Allocator, remap: *Remap);
```

## fun child_reintern

```mach
pub fun child_reintern(child: *Interner, alloc_: *A.Allocator) res[Remap, A.Error];
```

a non-child interner is a contract violation of the receiver, reported as
the allocator layer reports its own (`invalid`)

