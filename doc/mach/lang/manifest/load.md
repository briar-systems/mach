# mach.lang.manifest.load

## fun parse

```mach
pub fun parse(alloc: *A.Allocator, itn: *intern.Interner, doc: *Doc, as_root: bool) res[Manifest, outcome.Fail];
```

build a `Manifest` from a parsed TOML document. accepted root tables are
`[project]`, `[target.*]`, `[artifact.*]`, `[profile.*]`, `[dep.*]`,
`[link.*]` and `[step.*]`; any other root key, and any key a table does not
define, is an error naming it. on any error every array allocated so far is
freed before returning

alloc: owns the manifest's arrays
itn: receives every string of the manifest
doc: the manifest's text and its TOML document
as_root: true for the project being built, false for a dependency's manifest.
         the root form requires at least one `[profile.*]` table, `[artifact].link`
         and `need`, `[link].os`, `isa`, `abi` and `export`, `[step].need`, requires
         link filter values to be canonical or "*", and rejects more than one
         `default = true` profile. the dependency form treats each of those keys as
         optional. a declared `[profile.*]` table requires `opt`, `debug`, `simd`,
         `vectorize` and `float_reassoc` in both forms. the key set is closed for
         both forms: a key is read or refused as unknown
ret: the manifest, or the first error as a "mach.toml: ..." message. the
         `[project]` table and its `id`, `version`, `src` and `out` are always
         required; `src`, `out`, artifact `entry` and `out`, local link `path` and
         step `in` and `out` entries must satisfy `is_project_path`; every table name
         must satisfy `is_valid_id`; `[target.native]` and `[dep.<x>].version` are
         reserved; a `[step]` with `cmd` or `shell` is rejected by name; `need`
         entries are checked by `validate_needs`. `[profile.*]` absent or empty is
         an error at the root and synthesizes `debug` and `release` in a dependency.
         a failure that names a key points at it in `doc`, and the manifest records
         where `[project]` and its `mach` value are written

## rec Doc

```mach
pub rec Doc;
```

a manifest's text, the file it was read from, and its TOML tree, all owned
through the allocator that made it and released with `doc_dnit`. a failure
that names a key points into `text` at `path`

## fun doc_parse

```mach
pub fun doc_parse(alloc: *A.Allocator, path: str, text: str) res[Doc, outcome.Fail];
```

`text`, the manifest at `path`, as a doc: bytes a manifest refuses and a
document that is not TOML are failures pointing at the byte refused. the doc
keeps its own copies of both

## fun doc_read

```mach
pub fun doc_read(alloc: *A.Allocator, path: str) res[Doc, outcome.Fail];
```

the manifest file at `path` read and parsed as `doc_parse` parses it; a file
that cannot be read is the environment's

## fun doc_dnit

```mach
pub fun doc_dnit(alloc: *A.Allocator, d: *Doc);
```

release a doc and everything it owns

