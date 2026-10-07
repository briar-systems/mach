# mach.lang.fe.decorator

the decorator registry: one row per decorator the language knows, which every
phase reads by id

the parser decodes each `#[name(...)]` once into the id its name spells, and no
later phase reads the name again. type checking holds each decorator to its row:
the count and kind of its arguments, the declarations it stands on, whether it
may repeat, and whether it is a shader interface role; a rule no row expresses
is the decorator's own check, keyed by its id. a new decorator is a row here and
its consumer

a string argument is a constant expression evaluated in its module's comptime
context: a literal, a `val`, a gated `val`, or an imported constant. the load
evaluates each once, after its walk has bound every constant, and records the
string in its own bindings; every later stage reads it there, so resolution,
type checking, lowering and the link all read one value

## def Id

```mach
pub def Id: u8
```

## val NONE

```mach
pub val NONE:       Id = 0
```

## val DEPRECATED

```mach
pub val DEPRECATED: Id = 1
```

## val SYMBOL

```mach
pub val SYMBOL:     Id = 2
```

## val SECTION

```mach
pub val SECTION:    Id = 3
```

## val INLINE

```mach
pub val INLINE:     Id = 4
```

## val NOINLINE

```mach
pub val NOINLINE:   Id = 5
```

## val ALIGN

```mach
pub val ALIGN:      Id = 6
```

## val PACKED

```mach
pub val PACKED:     Id = 7
```

## val VOLATILE

```mach
pub val VOLATILE:   Id = 8
```

## val LIBRARY

```mach
pub val LIBRARY:    Id = 9
```

## val OBLIVIOUS

```mach
pub val OBLIVIOUS:  Id = 10
```

## val SCALAR

```mach
pub val SCALAR:     Id = 11
```

## val NAKED

```mach
pub val NAKED:      Id = 12
```

## val EXTENSIONS

```mach
pub val EXTENSIONS: Id = 13
```

## val EMBED

```mach
pub val EMBED:      Id = 14
```

## val STAGE

```mach
pub val STAGE:      Id = 15
```

## val WORKGROUP

```mach
pub val WORKGROUP:  Id = 16
```

## val INPUT

```mach
pub val INPUT:      Id = 17
```

## val OUTPUT

```mach
pub val OUTPUT:     Id = 18
```

## val BUILTIN

```mach
pub val BUILTIN:    Id = 19
```

## val UNIFORM

```mach
pub val UNIFORM:    Id = 20
```

## val STORAGE

```mach
pub val STORAGE:    Id = 21
```

## val SAMPLER

```mach
pub val SAMPLER:    Id = 22
```

## val PUSH

```mach
pub val PUSH:       Id = 23
```

## val SPEC

```mach
pub val SPEC:       Id = 24
```

## val SHARED

```mach
pub val SHARED:     Id = 25
```

## val OP

```mach
pub val OP:         Id = 26
```

## val HANDLE

```mach
pub val HANDLE:     Id = 27
```

## val ABI_TYPE

```mach
pub val ABI_TYPE:   Id = 28
```

## val TESTING

```mach
pub val TESTING:    Id = 29
```

## val EXPECT

```mach
pub val EXPECT:     Id = 30
```

## def Places

```mach
pub def Places: u32
```

where a decorator may stand: one bit per declaration kind, one for an `ext`
import, one for a tag case, and one for every declaration no decorator names.
PLACES_ANY is every declaration; a tag case admits only a row that names it

## val PLACE_USE

```mach
pub val PLACE_USE:   Places = 0x1
```

## val PLACE_FWD

```mach
pub val PLACE_FWD:   Places = 0x2
```

## val PLACE_FUN

```mach
pub val PLACE_FUN:   Places = 0x4
```

## val PLACE_REC

```mach
pub val PLACE_REC:   Places = 0x8
```

## val PLACE_UNI

```mach
pub val PLACE_UNI:   Places = 0x10
```

## val PLACE_TAG

```mach
pub val PLACE_TAG:   Places = 0x20
```

## val PLACE_DEF

```mach
pub val PLACE_DEF:   Places = 0x40
```

## val PLACE_VAL

```mach
pub val PLACE_VAL:   Places = 0x80
```

## val PLACE_VAR

```mach
pub val PLACE_VAR:   Places = 0x100
```

## val PLACE_TEST

```mach
pub val PLACE_TEST:  Places = 0x200
```

## val PLACE_EXT

```mach
pub val PLACE_EXT:   Places = 0x400
```

## val PLACE_CASE

```mach
pub val PLACE_CASE:  Places = 0x800
```

## val PLACE_OTHER

```mach
pub val PLACE_OTHER: Places = 0x1000
```

## val PLACES_ANY

```mach
pub val PLACES_ANY:  Places = 0x17FF
```

## def Flags

```mach
pub def Flags: u32
```

what a row asks of its decorator beyond its arguments and places

FLAG_ONCE: it stands at most once on a declaration
FLAG_ROLE: it gives a module binding a shader interface role, and a binding has one
FLAG_TESTING_EXCLUSIVE: it names a consumer outside Mach source, which `#[testing]` cannot confine
FLAG_WORDS: its arguments are bare names that bind to nothing, never expressions
FLAG_STRINGS_CHECKED: its string arguments are checked by the row, not by the decorator's own rule
FLAG_PLACE_CHECKED: the decorator refuses a misplacement itself, in a diagnostic of its own kind

## val FLAG_ONCE

```mach
pub val FLAG_ONCE:              Flags = 0x1
```

## val FLAG_ROLE

```mach
pub val FLAG_ROLE:              Flags = 0x2
```

## val FLAG_TESTING_EXCLUSIVE

```mach
pub val FLAG_TESTING_EXCLUSIVE: Flags = 0x4
```

## val FLAG_WORDS

```mach
pub val FLAG_WORDS:             Flags = 0x8
```

## val FLAG_STRINGS_CHECKED

```mach
pub val FLAG_STRINGS_CHECKED:   Flags = 0x10
```

## val FLAG_PLACE_CHECKED

```mach
pub val FLAG_PLACE_CHECKED:     Flags = 0x20
```

## val ARGS_ANY

```mach
pub val ARGS_ANY:     u32 = 0xFFFFFFFF
```

an argument count with no upper bound, and string arguments that run to the last

## val STRINGS_REST

```mach
pub val STRINGS_REST: u32 = 0xFFFFFFFF
```

## rec Row

```mach
pub rec Row;
```

one decorator

places: where it stands, `misplaced` the refusal of anywhere else
args_min, args_max:           the argument count it takes, `arity` the refusal of any other
strings_first, strings_count: the arguments that are strings the load records, none when the count is 0
integers: one bit per argument the row checks as an integer constant, `argument`
           the refusal of a string or integer argument of the wrong kind
duplicate: the refusal of a second one where FLAG_ONCE holds

## fun row

```mach
pub fun row(id: Id) *Row;
```

the row of `id`; nil for NONE and any id no row declares

## fun id_of

```mach
pub fun id_of(source: str, name: lang_source.Span) Id;
```

the decorator the name at `name` in `source` spells; NONE when it spells none

## fun name_of

```mach
pub fun name_of(id: Id) str;
```

the name of `id`; nil for NONE

## fun places_of

```mach
pub fun places_of(d: *ast_decl.Decl) Places;
```

where the declaration `d` stands, for a row's `places`

## fun admits

```mach
pub fun admits(id: Id, d: *ast_decl.Decl) bool;
```

whether `id` may stand on the declaration `d`

## fun flagged

```mach
pub fun flagged(id: Id, flags: Flags) bool;
```

whether `id`'s row carries every one of `flags`

## fun list_get

```mach
pub fun list_get(a: *ast.Ast, start: u32, len: u32, id: Id) opt[*ast_decl.Decorator];
```

the first decorator `id` among the `len` decorators from `start`

## fun list_has

```mach
pub fun list_has(a: *ast.Ast, start: u32, len: u32, id: Id) bool;
```

whether the decorator `id` is among the `len` decorators from `start`

## fun get

```mach
pub fun get(a: *ast.Ast, d: *ast_decl.Decl, id: Id) opt[*ast_decl.Decorator];
```

the declaration's decorator `id`, the first when it repeats

## fun has

```mach
pub fun has(a: *ast.Ast, d: *ast_decl.Decl, id: Id) bool;
```

whether the declaration carries the decorator `id`

## fun count_flagged

```mach
pub fun count_flagged(a: *ast.Ast, d: *ast_decl.Decl, flags: Flags) u32;
```

how many of the declaration's decorators carry every one of `flags`

## fun names_append

```mach
pub fun names_append(b: *textbuild.TextBuilder, flags: Flags, places: Places, conjunction: str) err[textbuild.Error];
```

appends the names of the rows that carry every one of `flags` and stand somewhere
in `places`, `, `-separated in row order. with a `conjunction` each name is quoted
in backticks and the conjunction precedes the last, as in "`a`, `b`, and `c`"

## fun takes_string

```mach
pub fun takes_string(dec: *ast_decl.Decorator, ord: u32) bool;
```

whether argument `ord` of `dec` is a string the load records

## fun string_id

```mach
pub fun string_id(c: *comptime.ComptimeCtx, a: *ast.Ast, dec: *ast_decl.Decorator, ord: u32) intern.StrId;
```

the string argument `ord` of `dec` evaluated to in the scope `c`; STR_NIL when it is not a constant string

## fun string_of

```mach
pub fun string_of(itn: *intern.Interner, c: *comptime.ComptimeCtx, a: *ast.Ast, dec: *ast_decl.Decorator, ord: u32) opt[str];
```

the text of `string_id`

## fun strings_record

```mach
pub fun strings_record[T](itn: *intern.Interner, into: *comptime.ComptimeCtx, a: *ast.Ast, source: str,
c: *comptime.ComptimeCtx, cap_ctx: *T, caps: comptime.PhaseCapabilities[T], reached: *bool) err[fail.Fail];
```

records into `into` the string every string argument of a module's decorators evaluates to, with
the phase's own evaluator over the scope `c`. an argument that is not a constant string is left
out, and so is a declaration `reached` does not mark, where it marks any

