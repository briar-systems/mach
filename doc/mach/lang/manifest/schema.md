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

## val SHAPE_FILTER

```mach
pub val SHAPE_FILTER: Shape = 4
```

a string, or an array whose every element is a string

## val SHAPE_TABLE

```mach
pub val SHAPE_TABLE: Shape = 5
```

a table whose keys are the author's own, such as a step's `env`

## val SHAPE_SECTION

```mach
pub val SHAPE_SECTION: Shape = 6
```

a table checked against the rows of its own key, such as `[project]`

## val SHAPE_ENTRIES

```mach
pub val SHAPE_ENTRIES: Shape = 7
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

