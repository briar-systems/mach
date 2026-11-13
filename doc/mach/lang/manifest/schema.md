# mach.lang.manifest.schema

## def Shape

```mach
pub def Shape: u8
```

the TOML shape a manifest key's value must have; a value of any other shape is
refused where it is written

## def Rule

```mach
pub def Rule: u8
```

whether a manifest must write a key. a key may be left out only when leaving it
out safely means that it does not apply, and one rule holds in every manifest,
whoever reads it

## rec Rewrite

```mach
pub rec Rewrite;
```

the mechanical rewrite of a key as a previous major wrote it

label: what the rewrite does; nil when the value as written has none
whole: the rewrite replaces the key and its value, not the key alone
replacement: the text written in their place
why: for a key this major still reads, why the key as written is
             refused; nil for a removed key, whose row's note says it

## fun row_of

```mach
pub fun row_of(rows: Rows, key: str) *KeyRow;
```

the row of `key` among `rows`, nil when no row names it

## fun check

```mach
pub fun check(alloc: *A.Allocator, doc: *toml_value.Table) err[fail.Fail];
```

check a parsed manifest document against the schema: every key it writes is a
row of its table, every value has its row's shape, every required key is
written and no removed one is. one rule holds for every key whoever reads the
manifest. the first violation is refused, pointing at the key or value it
concerns, or at the table a missing key belongs to

alloc: owns the refusal's text
doc: the document root
ret: ok when the document has the schema's shape

## rec Setting

```mach
pub rec Setting;
```

a `--set` key and value, read as the manifest reads the key

section: the root key of the tables the key is written over, such as "target"
doc: the document `<key> = <value>`, released with toml_tree.dnit
text: the document's text, which a refusal of the value points into

## val SET_PATH

```mach
pub val SET_PATH: str = "--set"
```

the path a refusal of a `--set` value is placed in

## fun setting

```mach
pub fun setting(alloc: *A.Allocator, sources: *lang_source.SourceMap, key: str, value: str) res[Setting, fail.Fail];
```

a `--set` key and value checked against the row of the key: the key is one a row marks `set`,
and the value has the row's shape. a string-shaped key takes the value as written, any other
reads it as a TOML value

alloc: owns the setting and a refusal's text
sources: receives the setting's text, a file of its own, which a refusal points into
key: the key, as a manifest writes it
value: the value, as written
ret: the setting; a user failure naming the key when no row names it, when its row is not
         one `--set` writes, or when the value does not have its row's shape

## fun toml_string

```mach
pub fun toml_string(b: *textbuild.TextBuilder, text: str);
```

`text` as a TOML basic string: a quote and a backslash escaped by a backslash, and a control
byte, which a basic string cannot hold, as its `\u00XX` escape

## fun section_check

```mach
pub fun section_check(alloc: *A.Allocator, doc: *toml_value.Table, key: str) err[fail.Fail];
```

check one section of the document root, as `check` holds it, for a reader
that needs only that section

alloc: owns the refusal's text
doc: the document root
key: a key of the document root, such as "project"
ret: ok when the section has the schema's shape

## rec Fix

```mach
pub rec Fix;
```

one replacement of the bytes `at` to `at + len` of a manifest's text by
`text` and `pad` spaces after it

## fun rewrites

```mach
pub fun rewrites(alloc: *A.Allocator, doc: *toml_value.Table, out: *Vector[Fix]) err[A.Error];
```

every rewrite the schema's rows call for in a parsed manifest document, as
fixes of its text: each removed key's rewrite and each retired value's, in
every table the schema holds. a key no row names or of the wrong shape adds
none, since the check refuses it as it stands

alloc: owns the fixes' storage
doc: the document root
out: receives the fixes, in no order
ret: ok, or the allocator's refusal

## rec Leftover

```mach
pub rec Leftover;
```

a removed key a manifest writes that no rewrite migrates, which the check refuses until its
author decides what replaces it

section: the root key of its table, "" for the document root
name: its table's name under a section of named tables, "" for a section itself
key: the key
note: what replaces it, as its refusal says
at: the key token as written

## fun leftovers

```mach
pub fun leftovers(doc: *toml_value.Table, out: *Vector[Leftover]) err[A.Error];
```

every removed key a parsed manifest document writes that no rewrite migrates, in every table
the schema holds

doc: the document root
out: receives the keys, in document order; its strings are borrowed from `doc` and the rows
ret: ok, or the allocator's refusal

## fun key_text

```mach
pub fun key_text(t: *toml_value.Table, key: str) str;
```

a string key as the manifest reads it: absent and empty are the same err
(`is missing required key`), so both read as the empty string

## fun flag

```mach
pub fun flag(t: *toml_value.Table, key: str) bool;
```

a boolean key's value, false when it is not written; the schema check has
refused any other shape

## fun written_value

```mach
pub fun written_value(t: *toml_value.Table, key: str) *toml_value.Value;
```

the value of a key the schema check has seen written with its row's shape, nil
when it is not written

## fun key_span

```mach
pub fun key_span(tab: *toml_value.Table, i: usize) toml_value.Span;
```

the key token of a table's entry `i` as written; the zero span when absent

## fun element_span

```mach
pub fun element_span(arr: *toml_value.Array, i: usize) toml_value.Span;
```

the literal of an array's element `i`; the zero span when absent

## fun origin

```mach
pub fun origin(alloc: *A.Allocator, itn: *intern.Interner, sub: *toml_value.Table, lead: str, key: str) res[Origin, fail.Fail];
```

the literal of the value `key` names in a table; the zero span when absent
where the table `sub` writes `key`, named for a refusal under `lead`, the table's label in
mach.toml such as `[target.x]`, or under `--set` for a nil lead. the place stays unresolved
until the manifest or the `--set` text resolves it

alloc: owns the name's text until it is interned
itn: holds the name for the session
sub: the table, which writes `key`
lead: the table's label, nil for a `--set` value
key: the key
ret: the origin, or a refused allocation

## fun value_span

```mach
pub fun value_span(tab: *toml_value.Table, key: str) toml_value.Span;
```

## fun value_text

```mach
pub fun value_text(alloc: *A.Allocator, v: *toml_value.Value) res[str, fail.Fail];
```

a scalar value as a refusal quotes it; a refused allocation is the failure

## fun is_valid_id

```mach
pub fun is_valid_id(s: str) bool;
```

whether `s` is a portable identifier: non-empty, only ASCII letters, digits,
'_' and '-'. the rule for project, target, artifact, profile, dependency,
link and step names

s: the candidate
ret: true when every byte is allowed

## fun is_project_path

```mach
pub fun is_project_path(value: str) bool;
```

whether `value` is a canonical strict descendant of the project root: non-empty,
'/'-separated, no '\', not absolute, no drive letter, no empty component, no
'.' or '..' component

value: the candidate path
ret: true when every rule holds

## val OUTPUT_CONSTRAINT

```mach
pub val OUTPUT_CONSTRAINT: str = "-o must name a canonical path inside the project root: relative, with no . or .. component"
```

the rule `-o` keeps wherever it names an output, which the cli prints and build refuses by

## fun check_path

```mach
pub fun check_path(alloc: *A.Allocator, value: str, field: str, sp: toml_value.Span) err[fail.Fail];
```

refuse a path that `is_project_path` does not admit, naming it as `field`
and pointing at `sp`, the value as written

## rec StrArr

```mach
pub rec StrArr;
```

a decoded array of strings, each interned

items: the ids, nil when the array is absent or empty
count: how many
present: the key is written

## fun ids_of

```mach
pub fun ids_of(alloc: *A.Allocator, itn: *intern.Interner, arr: *toml_value.Array, label: str, key: str) res[StrArr, fail.Fail];
```

intern every element of an array of strings the schema check has admitted

alloc: owns the id array, freed with `model.free_strarr`
itn: receives the strings
arr: the array, nil when the key is absent
label: the table, as a refusal names it
key: the key the array is written under
ret: the ids; err for an array too long to count, or a refused allocation

