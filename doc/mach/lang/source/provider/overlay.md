# mach.lang.source.provider.overlay

## rec Overlay

```mach
pub rec Overlay;
```

the provider an editor reads through: the session's open buffers stand in front of the
provider under them, so a buffer is read for its file and a buffer whose file is not on
disk yet still names a module

base: the provider the buffers stand in front of
s: the buffers, keyed by the path the base locates

## fun init

```mach
pub fun init(base: provider.Provider, s: *Store) Overlay;
```

## fun provider_of

```mach
pub fun provider_of(o: *Overlay) provider.Provider;
```

## rec Store

```mach
pub rec Store;
```

the open buffers, one per file however its path is spelled; scanned by path, as an
editor holds a handful

a: owns every entry's path and text
items: the buffers

## fun store_init

```mach
pub fun store_init(a: *A.Allocator) Store;
```

## fun store_dnit

```mach
pub fun store_dnit(s: *Store);
```

## rec Prepared

```mach
pub rec Prepared;
```

## fun prepare

```mach
pub fun prepare(s: *Store, path: str, text: str, remove: bool) res[Prepared, fail.Fail];
```

the caller exclusively borrows overlay state through commit or discard

## fun discard

```mach
pub fun discard(prepared: *Prepared);
```

release an overlay that was prepared and not committed; one already committed or
discarded owns nothing

## fun commit

```mach
pub fun commit(prepared: *Prepared) bool;
```

the prepared change made, and whether it changed the texts; a change expires the views borrowed
from the session holding the store

## fun set

```mach
pub fun set(s: *Store, path: str, text: str) res[bool, fail.Fail];
```

set the text standing in for `path`; true when it changed the texts

## fun get

```mach
pub fun get(s: *Store, path: str) opt[str];
```

the text standing in for `path`, none when no buffer does

