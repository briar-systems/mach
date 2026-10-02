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
pub fun init(a: *std_allocator.Allocator) Interner;
```

## fun make_child

```mach
pub fun make_child(base: *Interner, a: *std_allocator.Allocator) Interner;
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
pub fun intern(itn: *Interner, text: str) res[StrId, std_allocator.Error];
```

## fun intern_span

```mach
pub fun intern_span(itn: *Interner, source: str, span: token.Span) res[StrId, std_allocator.Error];
```

## fun intern_bytes

```mach
pub fun intern_bytes(itn: *Interner, data: str, len: usize) res[StrId, std_allocator.Error];
```

## rec ReinternMap

```mach
pub rec ReinternMap;
```

## fun reintern_map_dnit

```mach
pub fun reintern_map_dnit(alloc: *std_allocator.Allocator, remap: *ReinternMap);
```

## fun reintern_child

```mach
pub fun reintern_child(child: *Interner, alloc_: *std_allocator.Allocator) res[ReinternMap, std_allocator.Error];
```

a non-child interner is a contract violation of the receiver, reported as
the allocator layer reports its own (`invalid`)

