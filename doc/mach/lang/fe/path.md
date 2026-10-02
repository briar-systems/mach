# mach.lang.fe.path

## fun next

```mach
pub fun next(source: str, path: token.Span, cursor: *usize) token.Span;
```

paths are parser accepted identifier and dot sequences with original trivia intact

## fun leaf

```mach
pub fun leaf(source: str, path: token.Span) token.Span;
```

## fun prefix

```mach
pub fun prefix(source: str, path: token.Span) token.Span;
```

## fun intern_path

```mach
pub fun intern_path(itn: *intern.Interner, source: str, path: token.Span) res[intern.StrId, A.Error];
```

