# mach.lang.fe.comptime.intrinsic

the comptime intrinsic registry: one row per `$name(...)` the compiler answers,
which every phase reads by id

each phase decodes a comptime call's `$name` to the id it spells and decides by
the id and its row, never by the name. a new intrinsic is a row here and the
consumers that answer it

## def Id

```mach
pub def Id: u8
```

## val NONE

```mach
pub val NONE:            Id = 0
```

## val SIZE_OF

```mach
pub val SIZE_OF:         Id = 1
```

## val ALIGN_OF

```mach
pub val ALIGN_OF:        Id = 2
```

## val OFFSET_OF

```mach
pub val OFFSET_OF:       Id = 3
```

## val LENGTH_OF

```mach
pub val LENGTH_OF:       Id = 4
```

## val TYPE_ID

```mach
pub val TYPE_ID:         Id = 5
```

## val TYPE_NAME

```mach
pub val TYPE_NAME:       Id = 6
```

## val IS_RECORD

```mach
pub val IS_RECORD:       Id = 7
```

## val IS_UNION

```mach
pub val IS_UNION:        Id = 8
```

## val IS_TAG

```mach
pub val IS_TAG:          Id = 9
```

## val IS_POINTER

```mach
pub val IS_POINTER:      Id = 10
```

## val IS_SECRET

```mach
pub val IS_SECRET:       Id = 11
```

## val HOLDS_SECRET

```mach
pub val HOLDS_SECRET:    Id = 12
```

## val IS_INTEGER

```mach
pub val IS_INTEGER:      Id = 13
```

## val IS_FLOAT

```mach
pub val IS_FLOAT:        Id = 14
```

## val TYPE_OF

```mach
pub val TYPE_OF:         Id = 15
```

## val POINTEE_OF

```mach
pub val POINTEE_OF:      Id = 16
```

## val DISCRIMINANT_OF

```mach
pub val DISCRIMINANT_OF: Id = 17
```

## val FIELDS

```mach
pub val FIELDS:          Id = 18
```

## val CASES

```mach
pub val CASES:           Id = 19
```

## val ERROR

```mach
pub val ERROR:           Id = 20
```

## def Class

```mach
pub def Class: u8
```

what an intrinsic answers

CLASS_LAYOUT: a constant measured from a type's layout or identity
CLASS_QUERY: a predicate over a type, or its spelling
CLASS_GATE: a type compared inside a `$if` gate, never a value
CLASS_CONSTRUCTOR: a type built from a type, written where a type is expected
CLASS_ITERATION: the members a `$each` walks
CLASS_DIRECTIVE: a compile-time refusal

## val CLASS_LAYOUT

```mach
pub val CLASS_LAYOUT:      Class = 0
```

## val CLASS_QUERY

```mach
pub val CLASS_QUERY:       Class = 1
```

## val CLASS_GATE

```mach
pub val CLASS_GATE:        Class = 2
```

## val CLASS_CONSTRUCTOR

```mach
pub val CLASS_CONSTRUCTOR: Class = 3
```

## val CLASS_ITERATION

```mach
pub val CLASS_ITERATION:   Class = 4
```

## val CLASS_DIRECTIVE

```mach
pub val CLASS_DIRECTIVE:   Class = 5
```

## rec Row

```mach
pub rec Row;
```

one intrinsic

args: the argument count it takes
type_operand: its first argument is a type, written in the full type grammar

## fun row

```mach
pub fun row(id: Id) *Row;
```

the row of `id`; nil for NONE and any id no row declares

## fun id_of

```mach
pub fun id_of(source: str, name: lang_source.Span) Id;
```

the intrinsic the name at `name` in `source` spells, without its `$`; NONE when it spells none

## fun in_class

```mach
pub fun in_class(id: Id, class: Class) bool;
```

whether `id` answers within `class`

## fun takes_type_operand

```mach
pub fun takes_type_operand(id: Id) bool;
```

whether `id` takes a type as its first argument

