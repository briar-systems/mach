# mach.lang.fe.decorator

the decorator registry: one row per decorator the language knows, which every
phase reads by id

the parser decodes each `#[name(...)]` once into the id its name spells, and no
later phase reads the name again. type checking holds each decorator to its row:
the count and kind of its arguments, the declarations it stands on, whether it
may repeat, and whether it is a shader interface role; a rule no row expresses
is the decorator's own check, keyed by its id. a new decorator is a row here and
its consumer

a name is lowercase, `snake_case` where it is several words, and dotted only in
the `gpu.` set, whose generic words apply only to shader stages. every argument
is a constant expression: a number is an integer, and every name, mode or path
is a string, so no decorator takes a bare word. a mode is a string argument of
one decorator, never a sibling decorator, and its row lists the modes it takes

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
pub val NONE:          Id = 0
```

## val DEPRECATED

```mach
pub val DEPRECATED:    Id = 1
```

## val SYMBOL

```mach
pub val SYMBOL:        Id = 2
```

## val SECTION

```mach
pub val SECTION:       Id = 3
```

## val INLINE

```mach
pub val INLINE:        Id = 4
```

## val EXPORT

```mach
pub val EXPORT:        Id = 5
```

## val ALIGN

```mach
pub val ALIGN:         Id = 6
```

## val PACKED

```mach
pub val PACKED:        Id = 7
```

## val VOLATILE

```mach
pub val VOLATILE:      Id = 8
```

## val LIBRARY

```mach
pub val LIBRARY:       Id = 9
```

## val CONSTANT_TIME

```mach
pub val CONSTANT_TIME: Id = 10
```

## val VECTORIZE

```mach
pub val VECTORIZE:     Id = 11
```

## val NAKED

```mach
pub val NAKED:         Id = 12
```

## val EXTENSION

```mach
pub val EXTENSION:     Id = 13
```

## val EMBED

```mach
pub val EMBED:         Id = 14
```

## val GPU_STAGE

```mach
pub val GPU_STAGE:     Id = 15
```

## val GPU_WORKGROUP

```mach
pub val GPU_WORKGROUP: Id = 16
```

## val GPU_INPUT

```mach
pub val GPU_INPUT:     Id = 17
```

## val GPU_OUTPUT

```mach
pub val GPU_OUTPUT:    Id = 18
```

## val GPU_BUILTIN

```mach
pub val GPU_BUILTIN:   Id = 19
```

## val GPU_UNIFORM

```mach
pub val GPU_UNIFORM:   Id = 20
```

## val GPU_STORAGE

```mach
pub val GPU_STORAGE:   Id = 21
```

## val GPU_SAMPLER

```mach
pub val GPU_SAMPLER:   Id = 22
```

## val GPU_PUSH

```mach
pub val GPU_PUSH:      Id = 23
```

## val GPU_SPEC

```mach
pub val GPU_SPEC:      Id = 24
```

## val GPU_SHARED

```mach
pub val GPU_SHARED:    Id = 25
```

## val INTRINSIC

```mach
pub val INTRINSIC:     Id = 26
```

## val HANDLE

```mach
pub val HANDLE:        Id = 27
```

## val ABI_TYPE

```mach
pub val ABI_TYPE:      Id = 28
```

## val TESTING

```mach
pub val TESTING:       Id = 29
```

## val EXPECT

```mach
pub val EXPECT:        Id = 30
```

## val THREAD

```mach
pub val THREAD:        Id = 31
```

## val AT

```mach
pub val AT:            Id = 32
```

## val WEAK

```mach
pub val WEAK:          Id = 33
```

## def Places

```mach
pub def Places: u32
```

where a decorator may stand: one bit per declaration kind, one for an `ext`
import, one for a tag case, and one for every declaration no decorator names.
PLACES_ANY is every declaration; a tag case admits only a row that names it

## val PLACE_CASE

```mach
pub val PLACE_CASE:  Places = 0x800
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
FLAG_STRINGS_CHECKED: its string arguments are checked by the row, not by the decorator's own rule
FLAG_PLACE_CHECKED: the decorator refuses a misplacement itself, in a diagnostic of its own kind
FLAG_DEFINES: it supplies the type of a `def` that states none

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

## val FLAG_STRINGS_CHECKED

```mach
pub val FLAG_STRINGS_CHECKED:   Flags = 0x10
```

## val FLAG_PLACE_CHECKED

```mach
pub val FLAG_PLACE_CHECKED:     Flags = 0x20
```

## val FLAG_DEFINES

```mach
pub val FLAG_DEFINES:           Flags = 0x40
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
modes: the values its first argument may take, `|`-separated, nil when any string is
           its own; `argument` is the refusal of any other value too
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

the decorator the name at `name` in `source` spells, this major's or a previous one's;
NONE when it spells none

## def Form

```mach
pub def Form: u8
```

how a decorator is written: as this major spells it, or in a previous major's spelling,
which is read with a `decorator.renamed` warning naming its replacement until the
family's sources use this major's (#4894 makes it a refusal)

FORM_V7: this major's name and arguments
FORM_SAME: a previous name with this major's arguments
FORM_BARE: a previous flag with no arguments, standing for its row's mode
FORM_WORDS: a previous name whose arguments are bare names, which bind to nothing

## val FORM_V7

```mach
pub val FORM_V7:    Form = 0
```

## val FORM_SAME

```mach
pub val FORM_SAME:  Form = 1
```

## val FORM_BARE

```mach
pub val FORM_BARE:  Form = 2
```

## val FORM_WORDS

```mach
pub val FORM_WORDS: Form = 3
```

## fun renamed

```mach
pub fun renamed(source: str, name: lang_source.Span) str;
```

the name this major gives the decorator the name at `name` in `source`
spells, nil when no previous major named one so or the rename changes its arguments too

## fun form_of

```mach
pub fun form_of(source: str, name: lang_source.Span, args: u32) Form;
```

how the decorator named at `name` in `source` with `args` arguments is written

## fun renamed_note

```mach
pub fun renamed_note(source: str, dec: *ast_decl.Decorator) str;
```

the warning on the previous spelling `dec` is written in, nil when it is written as
this major spells it

## fun mode_of

```mach
pub fun mode_of(itn: *intern.Interner, c: *comptime.ComptimeCtx, a: *ast.Ast, source: str, dec: *ast_decl.Decorator) opt[str];
```

the mode `dec` states: its first argument, or the mode a previous flag stands for

## fun constant_time_mode

```mach
pub fun constant_time_mode(itn: *intern.Interner, c: *comptime.ComptimeCtx, a: *ast.Ast, source: str, d: *ast_decl.Decl) opt[str];
```

the constant-time mode `d` declares, "soft" or "hard", none when it declares none. a previous
major's bare `#[oblivious]` is "soft". every reader of the mode asks here, so none disagree

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

## fun mode_admits

```mach
pub fun mode_admits(modes: str, value: str) bool;
```

whether `value` is one of the `|`-separated `modes`

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

