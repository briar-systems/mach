[mach](../README.md) / [base](README.md) / **handle**

# mach.base.handle

stores whose elements never move

a `StableChunks` grows by fixed chunks, so a pointer to an element stays valid until the store
is released, and one exclusive editor appends at a time.

Source: [`src/base/handle.mach`](../../../src/base/handle.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `tag` | a store's refusal: the allocator's, a store or editor that is not live, or a second exclusive editor |
| [`StableChunks`](#stablechunks) | `rec` |  |
| [`ChunkEditor`](#chunkeditor) | `rec` |  |

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
| [`chunk_grow`](#chunk_grow), [`chunk_append`](#chunk_append) | |

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

## StableChunks

```mach
pub rec StableChunks[T] {
    a:       *std.allocator.Allocator;
    chunks:  Vector[*T];
    len:     usize;
    editing: bool;
}
```

### chunk_init

```mach
pub fun chunk_init[T](a: *std.allocator.Allocator) StableChunks[T]
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

