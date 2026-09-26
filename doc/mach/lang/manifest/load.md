# mach.lang.manifest.load

## fun parse

```mach
pub fun parse(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, as_root: bool) res[Manifest, outcome.Fail];
```

build a `Manifest` from a parsed TOML document. accepted root tables are
`[project]`, `[target.*]`, `[artifact.*]`, `[profile.*]`, `[dep.*]`,
`[link.*]` and `[step.*]`; any other root key, and any key a table does not
define, is an error naming it. on any error every array allocated so far is
freed before returning

alloc: owns the manifest's arrays
itn: receives every string of the manifest
t: the TOML document
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
         an error at the root and synthesizes `debug` and `release` in a dependency

