# mach.lang.manifest.dep

## fun parse_deps

```mach
pub fun parse_deps(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest, as_root: bool) err[outcome.Fail];
```

## def DepSource

```mach
pub def DepSource: u8
```

the source key a `mach dep add` writes

## val DEP_SOURCE_GIT

```mach
pub val DEP_SOURCE_GIT: DepSource = 1
```

a `git = "<url>"` dependency, with an optional `ref`

## val DEP_SOURCE_PATH

```mach
pub val DEP_SOURCE_PATH: DepSource = 2
```

a `path = "<dir>"` dependency

## rec DepTableSpec

```mach
pub rec DepTableSpec;
```

a `[dep.<name>]` table to append to a manifest's text

name: the dependency name; must satisfy `is_valid_id`
source: which key `value` is written under
value: the git URL or the path; escaped as a TOML basic string
ref: the `ref` value; written only for a git source and only when non-empty
version: the `version` range; written only for a git source, only when set, and never with a ref

## fun toml_escape_value

```mach
pub fun toml_escape_value(alloc: *A.Allocator, s: str) res[str, outcome.Fail];
```

the body of a TOML basic string for a value: backslash, quote and control
bytes escaped so the text reparses to the same value

alloc: owns the returned text
s: the value
ret: the escaped body, without the surrounding quotes

## fun manifest_add_dep_table

```mach
pub fun manifest_add_dep_table(alloc: *A.Allocator, source_text: str, spec: *DepTableSpec) res[str, outcome.Fail];
```

append a `[dep.<name>]` table to manifest text, leaving every existing byte in
place. the block uses the file's line ending and is separated by one blank
line (two when the text lacks a trailing newline)

alloc: owns the returned text
source_text: the current `mach.toml`
spec: the table to add
ret: the new text; err when the name is not an identifier, the source is not
             git or path, the text is not valid TOML, the dependency is already declared
             in any form, or the result does not reparse with the table present

## fun manifest_remove_dep_table

```mach
pub fun manifest_remove_dep_table(alloc: *A.Allocator, source_text: str, name: str) res[str, outcome.Fail];
```

cut a `[dep.<name>]` table out of manifest text, from its header line to the
next header, leaving everything else byte for byte

alloc: owns the returned text
source_text: the current `mach.toml`
name: the dependency name
ret: the new text; err when the name is not an identifier, the text is not
             valid TOML, the dependency is absent, is declared but not as a table, is
             declared more than once, or its header is not spelled exactly `[dep.<name>]`
             at the start of a line

