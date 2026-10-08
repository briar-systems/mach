# mach.lang.manifest.schema

## def Shape

```mach
pub def Shape: u8
```

the TOML shape a manifest key's value must have; a value of any other shape is
refused where it is written

## val SHAPE_STRING

```mach
pub val SHAPE_STRING: Shape = 0
```

a string, never empty

## val SHAPE_INTEGER

```mach
pub val SHAPE_INTEGER: Shape = 1
```

an integer

## val SHAPE_BOOL

```mach
pub val SHAPE_BOOL: Shape = 2
```

a boolean

## val SHAPE_STRINGS

```mach
pub val SHAPE_STRINGS: Shape = 3
```

an array whose every element is a string

## val SHAPE_TABLE

```mach
pub val SHAPE_TABLE: Shape = 4
```

a table whose keys are the author's own, such as a step's `env`

## val SHAPE_SECTION

```mach
pub val SHAPE_SECTION: Shape = 5
```

a table checked against the rows of its own key, such as `[project]`

## val SHAPE_ENTRIES

```mach
pub val SHAPE_ENTRIES: Shape = 6
```

a table of named tables, each checked against the rows of the key, such as `[target.*]`

## def Rule

```mach
pub def Rule: u8
```

whether a manifest must write a key. a key may be left out only when leaving it
out safely means that it does not apply, and one rule holds in every manifest,
whoever reads it

## val RULE_REQUIRED

```mach
pub val RULE_REQUIRED: Rule = 0
```

every manifest writes it

## val RULE_OPTIONAL

```mach
pub val RULE_OPTIONAL: Rule = 1
```

a manifest may leave it out; the row's note says what its absence means

## val RULE_DERIVED

```mach
pub val RULE_DERIVED: Rule = 2
```

another value of the same table decides whether it is written; the row's note
names that value

## val RULE_REMOVED

```mach
pub val RULE_REMOVED: Rule = 3
```

no longer read and refused by name; the row's note says what replaces it

## rec KeyRow

```mach
pub rec KeyRow;
```

one key of a manifest table

key: the key as written
shape: the shape its value must have
rule: whether a manifest must write it
note: for a required key, a hint added to the refusal of its absence, "" for
         none; for an optional key, what its absence means; for a derived key, the
         value that decides it; for a removed key, what replaces it
migrate: for a removed key, the rewrite its refusal carries as a fix; nil for
         a key whose replacement is no mechanical rewrite

## rec Rewrite

```mach
pub rec Rewrite;
```

the mechanical rewrite of a removed key as written

label: what the rewrite does; nil when the value as written has none
whole: the rewrite replaces the key and its value, not the key alone
replacement: the text written in their place

## def Migrate

```mach
pub def Migrate: fun(*toml.Table, *toml.Value) Rewrite
```

the rewrite of the removed key `v` of `tab`

## rec Rows

```mach
pub rec Rows;
```

the rows of one table

rows: the first row
count: how many rows there are
noun: what one entry of an `SHAPE_ENTRIES` table is called in a refusal

## fun root_rows

```mach
pub fun root_rows() Rows;
```

the rows of the document root

## fun rows_of

```mach
pub fun rows_of(key: str) Rows;
```

the rows of the table a root key names, the empty set for any other key

key: a key of the document root

## fun row_of

```mach
pub fun row_of(rows: Rows, key: str) *KeyRow;
```

the row of `key` among `rows`, nil when no row names it

## fun check

```mach
pub fun check(alloc: *A.Allocator, doc: *toml.Table) err[fail.Fail];
```

check a parsed manifest document against the schema: every key it writes is a
row of its table, every value has its row's shape, every required key is
written and no removed one is. one rule holds for every key whoever reads the
manifest. the first violation is refused, pointing at the key or value it
concerns, or at the table a missing key belongs to

alloc: owns the refusal's text
doc: the document root
ret: ok when the document has the schema's shape

## fun section_check

```mach
pub fun section_check(alloc: *A.Allocator, doc: *toml.Table, key: str) err[fail.Fail];
```

check one section of the document root, as `check` holds it, for a reader
that needs only that section

alloc: owns the refusal's text
doc: the document root
key: a key of the document root, such as "project"
ret: ok when the section has the schema's shape

## fun shape_text

```mach
pub fun shape_text(shape: Shape) str;
```

what a value of `shape` is, as a refusal names it

## fun key_text

```mach
pub fun key_text(t: *toml.Table, key: str) str;
```

a string key as the manifest reads it: absent and empty are the same err
(`is missing required key`), so both read as the empty string

## fun flag

```mach
pub fun flag(t: *toml.Table, key: str) bool;
```

a boolean key's value, false when it is not written; the schema check has
refused any other shape

## fun written_value

```mach
pub fun written_value(t: *toml.Table, key: str) *toml.Value;
```

the value of a key the schema check has seen written with its row's shape, nil
when it is not written

## fun key_span

```mach
pub fun key_span(tab: *toml.Table, i: usize) toml.Span;
```

the key token of a table's entry `i` as written; the zero span when absent

## fun element_span

```mach
pub fun element_span(arr: *toml.Array, i: usize) toml.Span;
```

the literal of an array's element `i`; the zero span when absent

## fun value_span

```mach
pub fun value_span(tab: *toml.Table, key: str) toml.Span;
```

the literal of the value `key` names in a table; the zero span when absent

## fun value_text

```mach
pub fun value_text(alloc: *A.Allocator, v: *toml.Value) res[str, fail.Fail];
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
pub fun check_path(alloc: *A.Allocator, value: str, field: str, sp: toml.Span) err[fail.Fail];
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
pub fun ids_of(alloc: *A.Allocator, itn: *intern.Interner, arr: *toml.Array, label: str, key: str) res[StrArr, fail.Fail];
```

intern every element of an array of strings the schema check has admitted

alloc: owns the id array, freed with `model.free_strarr`
itn: receives the strings
arr: the array, nil when the key is absent
label: the table, as a refusal names it
key: the key the array is written under
ret: the ids; err for an array too long to count, or a refused allocation

