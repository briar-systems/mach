# mach.lang.session

## val FILE_TEXT

```mach
pub val FILE_TEXT: query.Kind = query.Kind;
```

a file's text as the session loaded it, keyed by the file's index. the source map and the
editor hold it across builds, so no released module takes it

## rec Session

```mach
pub rec Session;
```

the compiler's state across builds: the stores the phases read and write, each owned and
worked by its own module, and the generation of the views borrowed from them

## fun init

```mach
pub fun init(alloc: *A.Allocator) res[Session, fail.Fail];
```

## fun init_over

```mach
pub fun init_over(alloc: *A.Allocator, base: *intern.Interner) res[Session, fail.Fail];
```

a session whose names are layered over `base`: every id `base` has issued
names the same string here, so what was planned in another session, its
manifests and its cells, is read here as it was planned. `base` outlives the
session and takes no new names while it lives

## fun dnit

```mach
pub fun dnit(s: *Session);
```

release a session and everything it owns; a nil session holds nothing

## fun expire_views

```mach
pub fun expire_views(s: *Session);
```

a change to the session's inputs or module identities expires every view borrowed from it. exhaustion
permanently refuses new borrowed views while teardown remains infallible

## fun reset_type_projection

```mach
pub fun reset_type_projection(s: *Session) err[fail.Fail];
```

## fun next_parse_incarnation

```mach
pub fun next_parse_incarnation(s: *Session) res[u64, fail.Fail];
```

## fun load_source

```mach
pub fun load_source(s: *Session, path: str, text: str) res[lang_source.FileId, fail.Fail];
```

