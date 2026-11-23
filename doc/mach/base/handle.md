[mach](../README.md) / [base](README.md) / **handle**

# mach.base.handle

stores whose elements never move

a `StableChunks` grows by fixed chunks, so a pointer to an element stays valid until the store
is released, and one exclusive editor appends at a time.

Source: [`src/base/handle.mach`](../../../src/base/handle.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `tag` | a store's refusal: the allocator's, a store or editor that is not live, or a second exclusive editor |
| [`StableChunks`](#stablechunks) | `rec` | elements kept in fixed chunks, so an element never moves until the store is released |
| [`ChunkEditor`](#chunkeditor) | `rec` | the exclusive right to append to one store, from chunk_editor until chunk_editor_finish |

| On `Error` | |
| :--- | :--- |
| [`text`](#text) | the text of a store's refusal |

| On `StableChunks` | |
| :--- | :--- |
| [`chunk_init`](#chunk_init) | an empty store, released with chunk_dnit |
| [`chunk_borrow`](#chunk_borrow) | a store that reads every element of `from` as its own and adds its own past them: it borrows `from`'s chunks, which take nothing new while it lives, and at its first edit copies the list of them and the last one when `from` left it part full |
| [`chunk_dnit`](#chunk_dnit) | release a store and its chunks; a nil store holds nothing |
| [`chunk_count`](#chunk_count) | how many elements the store holds, none for a nil store |
| [`chunk_at`](#chunk_at) | the element at `index`, absent past the end or on a nil store |
| [`chunk_get`](#chunk_get) | the element at `index`, which the store handed out and so lies before its end; any other index is a broken precondition of the caller, and it stops the compiler |
| [`chunk_editor`](#chunk_editor) | open the store's one exclusive editor |

| On `ChunkEditor` | |
| :--- | :--- |
| [`chunk_editor_finish`](#chunk_editor_finish) | end an edit of a store; an editor that opened no store ends nothing |
| [`chunk_grow`](#chunk_grow) | make room for `additional` more elements, so the appends that fill it cannot allocate |
| [`chunk_append`](#chunk_append) | append `value`, growing by one chunk when the store is full |

---

## Error

```mach
pub tag Error: u8 {
    alloc: std.allocator.Error;
    absent;
    busy;
}
```

a store's refusal: the allocator's, a store or editor that is not live, or a
second exclusive editor

### text

```mach
pub fun text(e: Error) str
```

the text of a store's refusal

## StableChunks

```mach
pub rec StableChunks[T] {
    a:        *std.allocator.Allocator;
    chunks:   Vector[*T];
    len:      usize;
    editing:  bool;
    borrowed: usize;
    lent:     bool;
}
```

elements kept in fixed chunks, so an element never moves until the store is released. a store may
borrow another's chunks (chunk_borrow): its first `borrowed` chunks are the lender's, and until its
first edit so is the list of them

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | owns the chunks past the borrowed ones |
| `chunks` | `Vector[*T]` | the chunks, each CHUNK_CAPACITY elements |
| `len` | `usize` | how many elements the store holds, the lender's included |
| `editing` | `bool` | an editor holds the store |
| `borrowed` | `usize` | how many leading chunks are the lender's, never released by this store |
| `lent` | `bool` | the list of chunks is still the lender's, copied at the first edit |

### chunk_init

```mach
pub fun chunk_init[T](a: *std.allocator.Allocator) StableChunks[T]
```

an empty store, released with chunk_dnit

### chunk_borrow

```mach
pub fun chunk_borrow[T](a: *std.allocator.Allocator, from: *StableChunks[T]) StableChunks[T]
```

a store that reads every element of `from` as its own and adds its own past them: it borrows
`from`'s chunks, which take nothing new while it lives, and at its first edit copies the list of
them and the last one when `from` left it part full

### chunk_dnit

```mach
pub fun chunk_dnit[T](s: *StableChunks[T])
```

release a store and its chunks; a nil store holds nothing

### chunk_count

```mach
pub fun chunk_count[T](s: *StableChunks[T]) usize
```

how many elements the store holds, none for a nil store

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

open the store's one exclusive editor

| Parameter | |
| :--- | :--- |
| `s` | the store |
| **returns** | the editor, or absent for a nil store and busy when an editor is already open |

## ChunkEditor

```mach
pub rec ChunkEditor[T] {
    store: *StableChunks[T];
}
```

the exclusive right to append to one store, from chunk_editor until chunk_editor_finish

| Field | Type | |
| :--- | :--- | :--- |
| `store` | `*StableChunks[T]` | the store it edits, nil once finished |

### chunk_editor_finish

```mach
pub fun chunk_editor_finish[T](e: *ChunkEditor[T])
```

end an edit of a store; an editor that opened no store ends nothing

### chunk_grow

```mach
pub fun chunk_grow[T](e: *ChunkEditor[T], additional: usize) err[Error]
```

make room for `additional` more elements, so the appends that fill it cannot allocate

| Parameter | |
| :--- | :--- |
| `e` | an open editor |
| `additional` | how many elements to make room for |
| **returns** | absent for an editor that opened no store, or the allocator's refusal, which releases the chunks this call added |

### chunk_append

```mach
pub fun chunk_append[T](e: *ChunkEditor[T], value: T) res[*T, Error]
```

append `value`, growing by one chunk when the store is full

| Parameter | |
| :--- | :--- |
| `e` | an open editor |
| `value` | the element |
| **returns** | where the element now lives, stable until the store is released, or absent for an editor that opened no store, or the allocator's refusal |

