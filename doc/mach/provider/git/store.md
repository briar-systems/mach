# mach.provider.git.store

Git-compatible loose object storage, refs and committed tree inspection

## rec Repository

```mach
pub rec Repository;
```

## rec TreeEntry

```mach
pub rec TreeEntry;
```

## fun open

```mach
pub fun open(a: *A.Allocator, worktree: str) res[Repository, str];
```

open an ordinary checkout or submodule; linked-worktree common dirs are refused

## fun read_ref

```mach
pub fun read_ref(a: *A.Allocator, repo: *Repository, name: str) res[object.Id, str];
```

## fun join

```mach
pub fun join(a: *A.Allocator, root: str, relative: str) res[str, str];
```

## fun read_bounded

```mach
pub fun read_bounded(a: *A.Allocator, file: str, limit: usize) res[Vector[u8], str];
```

## fun has_loose

```mach
pub fun has_loose(a: *A.Allocator, repo: *Repository, id: *object.Id) res[bool, str];
```

## fun write_object

```mach
pub fun write_object(a: *A.Allocator, repo: *Repository, item: *pack.Object) err[str];
```

store a canonical object as an RFC 1950 stream with uncompressed deflate blocks

## fun write_pack

```mach
pub fun write_pack(a: *A.Allocator, repo: *Repository, p: *pack.Pack) err[str];
```

## fun read_object

```mach
pub fun read_object(a: *A.Allocator, repo: *Repository, id: *object.Id) res[pack.Object, str];
```

## fun tree_next

```mach
pub fun tree_next(r: *protocol.Reader, hash_len: usize) res[TreeEntry, str];
```

parse one borrowed tree entry; refuse checkout-control names before path creation

