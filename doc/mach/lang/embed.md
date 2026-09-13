# mach.lang.embed

## def EmbedStatus

```mach
pub def EmbedStatus: u8
```

## val EMBED_OK

```mach
pub val EMBED_OK:         EmbedStatus = 0
```

## val EMBED_MISSING

```mach
pub val EMBED_MISSING:    EmbedStatus = 1
```

## val EMBED_IS_DIR

```mach
pub val EMBED_IS_DIR:     EmbedStatus = 2
```

## val EMBED_UNREADABLE

```mach
pub val EMBED_UNREADABLE: EmbedStatus = 3
```

## val EMBED_TOO_LARGE

```mach
pub val EMBED_TOO_LARGE:  EmbedStatus = 4
```

## rec EmbedFile

```mach
pub rec EmbedFile;
```

## rec EmbedCache

```mach
pub rec EmbedCache;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator) EmbedCache;
```

## fun set_path_env

```mach
pub fun set_path_env(c: *EmbedCache, root: str, reqs: *manifest.ArtifactReq, req_count: u32);
```

## fun dnit

```mach
pub fun dnit(c: *EmbedCache);
```

## fun get

```mach
pub fun get(c: *EmbedCache, path_id: intern.StrId) opt[*EmbedFile];
```

## fun resolve_arg

```mach
pub fun resolve_arg(alloc: *A.Allocator, c: *EmbedCache, decl_file: str, arg: str) res[str, manifest.TemplateError];
```

## fun escapes_root

```mach
pub fun escapes_root(alloc: *A.Allocator, c: *EmbedCache, resolved: str) res[bool, fail.Fail];
```

## fun resolve_path

```mach
pub fun resolve_path(alloc: *A.Allocator, decl_file: str, arg: str) res[str, fail.Fail];
```

## fun refresh

```mach
pub fun refresh(c: *EmbedCache, db: *query.QueryDb, itn: *intern.Interner, p: str) res[intern.StrId, fail.Fail];
```

## fun path_arg_span

```mach
pub fun path_arg_span(a: *ast.Ast, dec: *decl.Decorator) opt[token.Span];
```

