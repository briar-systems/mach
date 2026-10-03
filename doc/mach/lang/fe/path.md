# mach.lang.fe.path

## fun next

```mach
pub fun next(source: str, path: lang_source.Span, cursor: *usize) lang_source.Span;
```

paths are parser accepted identifier and dot sequences with original trivia intact

## fun leaf

```mach
pub fun leaf(source: str, path: lang_source.Span) lang_source.Span;
```

## fun prefix

```mach
pub fun prefix(source: str, path: lang_source.Span) lang_source.Span;
```

## fun intern_path

```mach
pub fun intern_path(itn: *intern.Interner, source: str, path: lang_source.Span) res[intern.StrId, A.Error];
```

