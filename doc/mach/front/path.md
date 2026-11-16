# mach.front.path

## fun next

```mach
pub fun next(source: str, path: base_source.Span, cursor: *usize) base_source.Span;
```

paths are parser accepted identifier and dot sequences with original trivia intact

## fun leaf

```mach
pub fun leaf(source: str, path: base_source.Span) base_source.Span;
```

## fun prefix

```mach
pub fun prefix(source: str, path: base_source.Span) base_source.Span;
```

## fun intern_path

```mach
pub fun intern_path(itn: *intern.Interner, source: str, path: base_source.Span) res[intern.StrId, A.Error];
```

