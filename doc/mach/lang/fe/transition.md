# mach.lang.fe.transition

the forms of the previous major this one still reads, until the family's sources are
migrated

std and the libraries on it are spelled for 6.x and the seed compiles them, so this
compiler reads both spellings: each form below is a warning, and the form this major
spells works beside it. a row's `refused` is the whole of the flip once the seed rolls:
set it and the form is refused at the same places, naming the same replacement

## def Form

```mach
pub def Form: u8
```

## val CONDITION_U8

```mach
pub val CONDITION_U8: Form = 0
```

a condition, or an operand of `!`, `&&` or `||`, of type u8

## val U1_AS_U8

```mach
pub val U1_AS_U8: Form = 1
```

a u1 value read as a u8: stored in one, mixed with one, or under arithmetic

## val U8_CONSTANT

```mach
pub val U8_CONSTANT: Form = 2
```

a u8 constant holding 0 or 1 where a u1 is expected, as std's `true` and `false`

## val ASM_HEAD

```mach
pub val ASM_HEAD: Form = 3
```

an `asm` block headed by a previous major's identifier

## rec Row

```mach
pub rec Row;
```

one form

form: the form
warning: the kind it is warned with while it is read
refusal: the kind it is refused with once the row is `refused`
refused: the form is refused, the flip
warned: it warns at all. a form every std `bool` reaches states nothing a project can
         act on until std is migrated
every: it warns at each place, not once per module. a form as common as `bool` is
         warned once in each module of the root project
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

