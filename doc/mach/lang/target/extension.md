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

## fun bit_of

```mach
pub fun bit_of(table: *Extension, count: u32, name: str, len: usize) u64;
```

the bit `table` gives the name at `name[0..len]`, 0 when the table does not name it

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

## fun table_valid

```mach
pub fun table_valid(table: *Extension, count: u32) bool;
```

a table is well formed: every name an identifier, every bit a single nonzero
bit, no name or bit repeated, and every implied bit another row of the table

## fun spell_names

```mach
pub fun spell_names(table: *Extension, count: u32, buf: *u8, cap: usize, off: usize) usize;
```

the table's names, `, `-separated, appended at `off`; returns the new end

