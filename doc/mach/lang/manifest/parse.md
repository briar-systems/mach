# mach.lang.manifest.parse

## fun validate_source_bytes

```mach
pub fun validate_source_bytes(src: *u8, n: usize) err[outcome.Fail];
```

reject manifest text that would carry a NUL byte into the TOML parser

src: manifest bytes, not required to be NUL-terminated
n: number of bytes to scan
ret: ok when clean; err "mach.toml: source cannot contain NUL bytes" for a raw
     0 byte anywhere, or "mach.toml: strings cannot contain NUL bytes" for a
     `\u0000` or `\U00000000` escape inside a basic (double-quoted) string,
     single-line or multi-line. comments and literal (single-quoted) strings are
     skipped without escape checks

## fun validate_source_text

```mach
pub fun validate_source_text(src: str) err[outcome.Fail];
```

`validate_source_bytes` over a NUL-terminated string

src: manifest text
ret: as `validate_source_bytes`

## rec StrArr

```mach
pub rec StrArr;
```

## fun sfmt

```mach
pub fun sfmt(alloc: *A.Allocator, fmt: str, va: ...) str;
```

an sfmt result is owned and freed by its receiver; a formatting oom has nothing owned to hand back

## fun is_valid_id

```mach
pub fun is_valid_id(s: str) bool;
```

whether `s` is a portable identifier: non-empty, only ASCII letters, digits,
'_' and '-'. the rule for project, target, artifact, profile, dependency,
link and step names

s: the candidate
ret: true when every byte is allowed

## fun key_text

```mach
pub fun key_text(t: *toml.Table, key: str) str;
```

a string key as the manifest reads it: absent and empty are the same err
(`is missing required key`), so both read as the empty string

## fun intern_unwrap

```mach
pub fun intern_unwrap(itn: *intern.Interner, text: str) intern.StrId;
```

## fun intern_opt_unwrap

```mach
pub fun intern_opt_unwrap(itn: *intern.Interner, text: str) intern.StrId;
```

## fun parse_str_array

```mach
pub fun parse_str_array(alloc: *A.Allocator, itn: *intern.Interner, arr: *toml.Array,
label: str, key: str, allow_scalar: bool) res[StrArr, outcome.Fail];
```

## fun opt_value_text

```mach
pub fun opt_value_text(alloc: *A.Allocator, v: *toml.Value) str;
```

## fun parse_default_flag

```mach
pub fun parse_default_flag(alloc: *A.Allocator, table: str, name: str, sub: *toml.Table) res[bool, outcome.Fail];
```

## rec DefaultTables

```mach
pub rec DefaultTables;
```

## fun default_tables_init

```mach
pub fun default_tables_init() DefaultTables;
```

## fun default_tables_note

```mach
pub fun default_tables_note(alloc: *A.Allocator, d: *DefaultTables, label: str);
```

## fun default_tables_dnit

```mach
pub fun default_tables_dnit(alloc: *A.Allocator, d: *DefaultTables);
```

## fun duplicate_default_err

```mach
pub fun duplicate_default_err(alloc: *A.Allocator, kind: str, d: *DefaultTables) str;
```

## fun check_keys

```mach
pub fun check_keys(alloc: *A.Allocator, tab: *toml.Table, label: str, known: fun(str) bool) err[outcome.Fail];
```

refuse the first key of a table that its section's `known` predicate does not admit

## fun is_project_path

```mach
pub fun is_project_path(value: str) bool;
```

whether `value` is a canonical strict descendant of the project root: non-empty,
'/'-separated, no '\', not absolute, no drive letter, no empty component, no
'.' or '..' component

value: the candidate path
ret: true when every rule holds

## fun check_path

```mach
pub fun check_path(alloc: *A.Allocator, value: str, field: str) err[outcome.Fail];
```

## fun free_strarr

```mach
pub fun free_strarr(alloc: *A.Allocator, items: *intern.StrId, count: u32);
```

## fun must_lookup

```mach
pub fun must_lookup(itn: *intern.Interner, id: intern.StrId) res[str, outcome.Fail];
```

## fun is_glob

```mach
pub fun is_glob(s: str) bool;
```

## fun glob_matches

```mach
pub fun glob_matches(pat: str, name: str) bool;
```

## fun need_pattern

```mach
pub fun need_pattern(entry: str, category: str) str;
```

## fun need_matches

```mach
pub fun need_matches(entry: str, category: str, name: str) bool;
```

## fun idstr

```mach
pub fun idstr(itn: *intern.Interner, id: intern.StrId) str;
```

