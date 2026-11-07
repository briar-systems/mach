# mach.lang.fe.transition

the forms of the previous major and the tables that carry a project across them

each form is refused, its diagnostic naming what this major spells. the head, option
and alias tables are what `mach migrate` rewrites with, and the next major deletes them

## def Form

```mach
pub def Form: u8
```

## val CONDITION_U8

```mach
pub val CONDITION_U8: Form = 0
```

a condition of type u8

## val U1_AS_U8

```mach
pub val U1_AS_U8: Form = 1
```

a u1 under arithmetic, a shift, or a bitwise operator beside a wider operand

## val ASM_HEAD

```mach
pub val ASM_HEAD: Form = 2
```

an `asm` block headed by a previous major's identifier

## rec Row

```mach
pub rec Row;
```

one form

refusal: the kind it is refused with
message: what the diagnostic says
note: the replacement it names

## fun row

```mach
pub fun row(form: Form) *Row;
```

the row of `form`

## rec Head

```mach
pub rec Head;
```

a head a previous major wrote and the selection of masc's that stands for it: the
architecture's first level, which adds nothing to a target of it

## fun head_of

```mach
pub fun head_of(source: str, span: lang_source.Span) *Head;
```

the row of the head `span` of `source` writes, nil when no previous major wrote it

## rec Flag

```mach
pub rec Flag;
```

an option of mach's a previous major spelled otherwise

## val FLAG_COUNT

```mach
pub val FLAG_COUNT: usize            = 1
```

## fun flag_at

```mach
pub fun flag_at(i: usize) *Flag;
```

the row of renamed option `i`

## rec Alias

```mach
pub rec Alias;
```

a `def` a previous major spelled over a width this major has a truth value for

## fun alias_of

```mach
pub fun alias_of(source: str, name: lang_source.Span, over: lang_source.Span) *Alias;
```

the row of a `def` of the name at `name` over the type at `over`, nil for any other

