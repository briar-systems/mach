# mach.lang.intern

## def StrId

```mach
pub def StrId: u32
```

## val STR_NIL

```mach
pub val STR_NIL: StrId = 0xFFFFFFFF
```

## rec Interner

```mach
pub rec Interner;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Interner;
```

## fun make_child

```mach
pub fun make_child(base: *Interner, a: *A.Allocator) Interner;
```

## fun dnit

```mach
pub fun dnit(itn: *Interner);
```

## fun lookup

```mach
pub fun lookup(itn: *Interner, id: StrId) opt[str];
```

## fun intern

```mach
pub fun intern(itn: *Interner, text: str) res[StrId, A.Error];
```

## fun intern_span

```mach
pub fun intern_span(itn: *Interner, source: str, span: token.Span) res[StrId, A.Error];
```

## fun intern_bytes

```mach
pub fun intern_bytes(itn: *Interner, data: str, len: usize) res[StrId, A.Error];
```

## fun child_base_len

```mach
pub fun child_base_len(itn: *Interner) usize;
```

## fun count

```mach
pub fun count(itn: *Interner) usize;
```

## rec ReinternMap

```mach
pub rec ReinternMap;
```

## fun reintern_map_dnit

```mach
pub fun reintern_map_dnit(alloc: *A.Allocator, remap: *ReinternMap);
```

## fun reintern_child

```mach
pub fun reintern_child(child: *Interner, alloc_: *A.Allocator) res[ReinternMap, A.Error];
```

a non-child interner is a contract violation of the receiver, reported as
the allocator layer reports its own (`invalid`)

