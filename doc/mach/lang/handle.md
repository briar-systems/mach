# mach.lang.handle

## tag Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    absent;
    foreign;
    stale;
    outdated;
    busy;
}
```

a store's refusal: the allocator's, a store or editor that is not live, an
identifier that belongs to another store, one whose slot was reused since
it was issued, a view outdated by a mutation, or a second exclusive editor

## fun text

```mach
pub fun text(e: Error) str;
```

## rec StableChunks

```mach
pub rec StableChunks[T];
```

## rec ChunkEditor

```mach
pub rec ChunkEditor[T];
```

## fun chunk_init

```mach
pub fun chunk_init[T](a: *A.Allocator) StableChunks[T];
```

## fun chunk_dnit

```mach
pub fun chunk_dnit[T](s: *StableChunks[T]);
```

## fun chunk_count

```mach
pub fun chunk_count[T](s: *StableChunks[T]) usize;
```

## fun chunk_at

```mach
pub fun chunk_at[T](s: *StableChunks[T], index: usize) opt[*T];
```

the element at `index`, absent past the end or on a nil store

## fun chunk_editor

```mach
pub fun chunk_editor[T](s: *StableChunks[T]) res[ChunkEditor[T], Error];
```

## fun chunk_editor_finish

```mach
pub fun chunk_editor_finish[T](e: *ChunkEditor[T]);
```

## fun chunk_grow

```mach
pub fun chunk_grow[T](e: *ChunkEditor[T], additional: usize) err[Error];
```

## fun chunk_append

```mach
pub fun chunk_append[T](e: *ChunkEditor[T], value: T) res[*T, Error];
```

## rec HandleSlot

```mach
pub rec HandleSlot[T];
```

## rec HandleId

```mach
pub rec HandleId[Domain];
```

## rec HandleTable

```mach
pub rec HandleTable[T, Domain];
```

## rec HandleView

```mach
pub rec HandleView[T, Domain];
```

## rec HandleEditor

```mach
pub rec HandleEditor[T, Domain];
```

## fun table_init

```mach
pub fun table_init[T, Domain](a: *A.Allocator) res[HandleTable[T, Domain], Error];
```

## fun table_dnit

```mach
pub fun table_dnit[T, Domain](t: *HandleTable[T, Domain]);
```

## fun table_count

```mach
pub fun table_count[T, Domain](t: *HandleTable[T, Domain]) usize;
```

## fun get

```mach
pub fun get[T, Domain](t: *HandleTable[T, Domain], id: HandleId[Domain]) res[*T, Error];
```

## fun contains

```mach
pub fun contains[T, Domain](t: *HandleTable[T, Domain], id: HandleId[Domain]) bool;
```

## fun view

```mach
pub fun view[T, Domain](t: *HandleTable[T, Domain]) HandleView[T, Domain];
```

## fun view_valid

```mach
pub fun view_valid[T, Domain](v: *HandleView[T, Domain]) bool;
```

## fun view_get

```mach
pub fun view_get[T, Domain](v: *HandleView[T, Domain], id: HandleId[Domain]) res[*T, Error];
```

## fun edit

```mach
pub fun edit[T, Domain](t: *HandleTable[T, Domain]) res[HandleEditor[T, Domain], Error];
```

## fun editor_finish

```mach
pub fun editor_finish[T, Domain](e: *HandleEditor[T, Domain]);
```

## fun editor_insert

```mach
pub fun editor_insert[T, Domain](e: *HandleEditor[T, Domain], value: T) res[HandleId[Domain], Error];
```

## fun editor_remove

```mach
pub fun editor_remove[T, Domain](e: *HandleEditor[T, Domain], id: HandleId[Domain]) res[T, Error];
```

