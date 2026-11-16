[mach](../README.md) / [base](README.md) / **handle**

# mach.base.handle

stores whose elements never move, and handle tables with generation-checked ids

a `StableChunks` grows by fixed chunks, so a pointer to an element stays valid until the store
is released, and one exclusive editor appends at a time. a `HandleTable` issues ids that carry
their slot's generation and their table's identity, so a stale or foreign id is refused.

<sub>source: [`src/base/handle.mach`](../../../src/base/handle.mach) · imported by 5 modules</sub>

<sub>`A` is `std.allocator` · `HandleSlot` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `tag` | a store's refusal: the allocator's, a store or editor that is not live, an identifier that belongs to another store, one whose slot was reused since it was issued, or a second exclusive editor |
| [`StableChunks`](#stablechunks) | `rec` |  |
| [`ChunkEditor`](#chunkeditor) | `rec` |  |
| [`HandleId`](#handleid) | `rec` |  |
| [`HandleTable`](#handletable) | `rec` |  |
| [`HandleEditor`](#handleeditor) | `rec` |  |

| On `Error` | |
| :--- | :--- |
| [`text`](#text) | |

| On `StableChunks` | |
| :--- | :--- |
| [`chunk_init`](#chunk_init) | |
| [`chunk_dnit`](#chunk_dnit) | release a store and its chunks; a nil store holds nothing |
| [`chunk_count`](#chunk_count) | |
| [`chunk_at`](#chunk_at) | the element at `index`, absent past the end or on a nil store |
| [`chunk_get`](#chunk_get) | the element at `index`, which the store handed out and so lies before its end; any other index is a broken precondition of the caller, and it stops the compiler |
| [`chunk_editor`](#chunk_editor) | |

| On `ChunkEditor` | |
| :--- | :--- |
| [`chunk_editor_finish`](#chunk_editor_finish) | end an edit of a store; an editor that opened no store ends nothing |
| [`chunk_grow`](#chunk_grow) · [`chunk_append`](#chunk_append) | |

| On `HandleTable` | |
| :--- | :--- |
| [`table_init`](#table_init) | |
| [`table_dnit`](#table_dnit) | release a table and its slots; a nil table holds nothing |
| [`get`](#get) · [`edit`](#edit) | |

| On `HandleEditor` | |
| :--- | :--- |
| [`editor_finish`](#editor_finish) | end an edit of a table; an editor that opened no table ends nothing |
| [`editor_insert`](#editor_insert) · [`editor_remove`](#editor_remove) | |

---

## Error

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

### text

```mach
pub fun text(e: Error) str
```

## StableChunks

```mach
pub rec StableChunks[T] {
    a:       *A.Allocator;
    chunks:  Vector[*T];
    len:     usize;
    editing: bool;
}
```

### chunk_init

```mach
pub fun chunk_init[T](a: *A.Allocator) StableChunks[T]
```

### chunk_dnit

```mach
pub fun chunk_dnit[T](s: *StableChunks[T])
```

release a store and its chunks; a nil store holds nothing

### chunk_count

```mach
pub fun chunk_count[T](s: *StableChunks[T]) usize
```

### chunk_at

```mach
pub fun chunk_at[T](s: *StableChunks[T], index: usize) opt[*T]
```

the element at `index`, absent past the end or on a nil store

### chunk_get

```mach
pub fun chunk_get[T](s: *StableChunks[T], index: usize) *T
```

the element at `index`, which the store handed out and so lies before its end; any
other index is a broken precondition of the caller, and it stops the compiler

### chunk_editor

```mach
pub fun chunk_editor[T](s: *StableChunks[T]) res[ChunkEditor[T], Error]
```

## ChunkEditor

```mach
pub rec ChunkEditor[T] {
    store: *StableChunks[T];
}
```

### chunk_editor_finish

```mach
pub fun chunk_editor_finish[T](e: *ChunkEditor[T])
```

end an edit of a store; an editor that opened no store ends nothing

### chunk_grow

```mach
pub fun chunk_grow[T](e: *ChunkEditor[T], additional: usize) err[Error]
```

### chunk_append

```mach
pub fun chunk_append[T](e: *ChunkEditor[T], value: T) res[*T, Error]
```

## HandleId

```mach
pub rec HandleId[Domain] {
    index:        usize;
    generation:   u32;
    instance_tag: usize;
}
```

## HandleTable

```mach
pub rec HandleTable[T, Domain] {
    a:         *A.Allocator;
    slots:     StableChunks[HandleSlot[T]];
    free_list: Vector[usize];
    identity:  *u8;
    editing:   bool;
}
```

### table_init

```mach
pub fun table_init[T, Domain](a: *A.Allocator) res[HandleTable[T, Domain], Error]
```

### table_dnit

```mach
pub fun table_dnit[T, Domain](t: *HandleTable[T, Domain])
```

release a table and its slots; a nil table holds nothing

### get

```mach
pub fun get[T, Domain](t: *HandleTable[T, Domain], id: HandleId[Domain]) res[*T, Error]
```

### edit

```mach
pub fun edit[T, Domain](t: *HandleTable[T, Domain]) res[HandleEditor[T, Domain], Error]
```

## HandleEditor

```mach
pub rec HandleEditor[T, Domain] {
    table: *HandleTable[T, Domain];
}
```

### editor_finish

```mach
pub fun editor_finish[T, Domain](e: *HandleEditor[T, Domain])
```

end an edit of a table; an editor that opened no table ends nothing

### editor_insert

```mach
pub fun editor_insert[T, Domain](e: *HandleEditor[T, Domain], value: T) res[HandleId[Domain], Error]
```

### editor_remove

```mach
pub fun editor_remove[T, Domain](e: *HandleEditor[T, Domain], id: HandleId[Domain]) res[T, Error]
```

