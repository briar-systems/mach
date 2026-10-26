# mach.provider.git.worktree

repository operations shared by native dependency acquisition and verification

## rec Context

```mach
pub rec Context;
```

## fun dnit

```mach
pub fun dnit(c: *Context);
```

## fun read_config

```mach
pub fun read_config(a: *A.Allocator, file: str) res[cfg.Config, str];
```

## fun absolute

```mach
pub fun absolute(a: *A.Allocator, directory: str) res[str, str];
```

## fun open

```mach
pub fun open(a: *A.Allocator, directory: str) res[Context, str];
```

## fun entry

```mach
pub fun entry(c: *Context, relative: str) *index.Entry;
```

## fun load

```mach
pub fun load(c: *Context, id: *object.Id) res[*pack.Object, str];
```

## fun peel

```mach
pub fun peel(c: *Context, id: object.Id) res[object.Id, str];
```

## fun resolve

```mach
pub fun resolve(c: *Context, spec: str) res[object.Id, str];
```

## fun tree

```mach
pub fun tree(c: *Context, revision: str) res[object.Id, str];
```

## fun lookup

```mach
pub fun lookup(c: *Context, root: object.Id, relative: str) res[store.TreeEntry, str];
```

## fun mode_text

```mach
pub fun mode_text(mode: u32) str;
```

## fun append

```mach
pub fun append(out: *Vector[u8], text: str) err[str];
```

## fun id_text

```mach
pub fun id_text(a: *A.Allocator, id: *object.Id) res[str, str];
```

## fun save_index

```mach
pub fun save_index(c: *Context) err[str];
```

## fun config_set

```mach
pub fun config_set(a: *A.Allocator, file: str, key: str, value: str, remove_section: bool) err[str];
```

