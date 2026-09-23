# mach.lang.target.extension

## rec Extension

```mach
pub rec Extension;
```

## val MAX_SELECTED

```mach
pub val MAX_SELECTED: u32 = 64
```

the most names a selection can hold: one per bit

## rec Level

```mach
pub rec Level;
```

a level is a published bundle of an isa's extensions under one manifest
spelling (`x86-64-v3`). it is not a row: it has no bit, so it is never a
comptime member, a decorator argument or a feature-record field, and only
the manifest and a target request spell it. its members are closed over
`implies` with the rest of the selection, so a level is exactly the union
of its rows and whatever they bring

## fun bit_of

```mach
pub fun bit_of(table: *Extension, count: u32, name: str, len: usize) u64;
```

the bit `table` gives the name at `name[0..len]`, 0 when the table does not name it

## fun level_of

```mach
pub fun level_of(table: *Level, count: u32, name: str, len: usize) u64;
```

the members of the level `table` names at `name[0..len]`, 0 when it names none

## fun close

```mach
pub fun close(table: *Extension, count: u32, bits: u64) u64;
```

`bits` closed over `implies`: every extension a selected one brings, to a fixpoint

## fun admitted

```mach
pub fun admitted(selected: u64, function: u64) u64;
```

the extensions a body may emit: the target's selection and whatever its
`#[extensions(...)]` admits beyond it. one predicate for the inliner and the
encoder: an instruction needing `required` is emitted only into a body whose
admitted set holds it

## fun admits

```mach
pub fun admits(admitted_bits: u64, required: u64) bool;
```

## fun domain

```mach
pub fun domain(table: *Extension, count: u32) u64;
```

every bit the table names

## fun first_name

```mach
pub fun first_name(table: *Extension, count: u32, bits: u64) str;
```

the first name the table gives any bit of `bits`, nil when it names none

## fun is_name

```mach
pub fun is_name(name: str, len: usize) bool;
```

an extension name is spelled by the lexer's own rules: one identifier token,
never a keyword, so every surface that names it parses it the same way

## fun is_level_spelling

```mach
pub fun is_level_spelling(name: str, len: usize) bool;
```

a level spelling is an identifier with `-` admitted inside it (`x86-64-v2`),
so a manifest can carry a level where it carries a name and the resolver
tells them apart by table, never by shape

## fun is_spelling

```mach
pub fun is_spelling(name: str, len: usize) bool;
```

what a manifest's `extensions` entry may spell: a name or a level

## fun levels_valid

```mach
pub fun levels_valid(levels: *Level, count: u32, table: *Extension, ext_count: u32) bool;
```

a level table is well formed against its extension table: every spelling a
level spelling that no row also answers, no spelling repeated, every member
a row of the table, nonempty, and each level a strict superset of the one
before it, which is the order the published levels have

## fun table_valid

```mach
pub fun table_valid(table: *Extension, count: u32) bool;
```

a table is well formed: every name an identifier, every bit a single nonzero
bit, no name or bit repeated, and every implied bit another row of the table

## fun spell_levels

```mach
pub fun spell_levels(levels: *Level, count: u32, buf: *u8, cap: usize, off: usize) usize;
```

the level table's spellings, `, `-separated, appended at `off`; returns the new end

## fun spell_bits

```mach
pub fun spell_bits(table: *Extension, count: u32, bits: u64, buf: *u8, cap: usize, off: usize) usize;
```

the names of every row whose bit is in `bits`, `, `-separated, appended at `off`

## fun spell_names

```mach
pub fun spell_names(table: *Extension, count: u32, buf: *u8, cap: usize, off: usize) usize;
```

the table's names, `, `-separated, appended at `off`; returns the new end

