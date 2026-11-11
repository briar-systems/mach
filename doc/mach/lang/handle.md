# mach.lang.handle

## tag Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    absent;
    foreign;
    stale;
    busy;
}
```

a store's refusal: the allocator's, a store or editor that is not live, an
identifier that belongs to another store, one whose slot was reused since
it was issued, or a second exclusive editor

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

release a store and its chunks; a nil store holds nothing

## fun chunk_count

```mach
pub fun chunk_count[T](s: *StableChunks[T]) usize;
```

## fun chunk_at

```mach
pub fun chunk_at[T](s: *StableChunks[T], index: usize) opt[*T];
```

the element at `index`, absent past the end or on a nil store

## fun chunk_get

```mach
pub fun chunk_get[T](s: *StableChunks[T], index: usize) *T;
```

the element at `index`, which the store handed out and so lies before its end; any
other index is a broken precondition of the caller, and it stops the compiler

## fun chunk_editor

```mach
pub fun chunk_editor[T](s: *StableChunks[T]) res[ChunkEditor[T], Error];
```

## fun chunk_editor_finish

```mach
pub fun chunk_editor_finish[T](e: *ChunkEditor[T]);
```

end an edit of a store; an editor that opened no store ends nothing

## fun chunk_grow

```mach
pub fun chunk_grow[T](e: *ChunkEditor[T], additional: usize) err[Error];
```

## fun chunk_append

```mach
pub fun chunk_append[T](e: *ChunkEditor[T], value: T) res[*T, Error];
```

## rec HandleId

```mach
pub rec HandleId[Domain];
```

## rec HandleTable

```mach
pub rec HandleTable[T, Domain];
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

release a table and its slots; a nil table holds nothing

## fun get

```mach
pub fun get[T, Domain](t: *HandleTable[T, Domain], id: HandleId[Domain]) res[*T, Error];
```

## fun edit

```mach
pub fun edit[T, Domain](t: *HandleTable[T, Domain]) res[HandleEditor[T, Domain], Error];
```

## fun editor_finish

```mach
pub fun editor_finish[T, Domain](e: *HandleEditor[T, Domain]);
```

end an edit of a table; an editor that opened no table ends nothing

## fun editor_insert

```mach
pub fun editor_insert[T, Domain](e: *HandleEditor[T, Domain], value: T) res[HandleId[Domain], Error];
```

## fun editor_remove

```mach
pub fun editor_remove[T, Domain](e: *HandleEditor[T, Domain], id: HandleId[Domain]) res[T, Error];
```

