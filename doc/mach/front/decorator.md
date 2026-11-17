[mach](../README.md) / [front](README.md) / **decorator**

# mach.front.decorator

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

Source: [`src/front/decorator.mach`](../../../src/front/decorator.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.base.textbuild`](../base/textbuild.md), [`mach.front.ast`](ast.md), [`mach.front.ast.decl`](ast/decl.md), [`mach.front.comptime.capability`](comptime/capability.md), [`mach.front.comptime.scope`](comptime/scope.md).

| Type | | |
| :--- | :--- | :--- |
| [`Id`](#id) | `def` |  |
| [`Places`](#places) | `def` | where a decorator may stand: one bit per declaration kind, one for an `ext` import, one for a tag case, and one for every declaration no decorator names |
| [`Flags`](#flags) | `def` | what a row asks of its decorator beyond its arguments and places |
| [`Row`](#row-1) | `rec` | one decorator |
| [`Rewrite`](#rewrite) | `rec` | what `mach migrate` writes for the previous spelling at `name` in `source` |

| On `Id` | |
| :--- | :--- |
| [`NONE`](#id), [`DEPRECATED`](#id), [`SYMBOL`](#id), [`SECTION`](#id), [`INLINE`](#id), [`EXPORT`](#id), [`ALIGN`](#id), [`PACKED`](#id), [`VOLATILE`](#id), [`LIBRARY`](#id), [`CONSTANT_TIME`](#id), [`VECTORIZE`](#id), [`NAKED`](#id), [`EXTENSION`](#id), [`EMBED`](#id), [`GPU_STAGE`](#id), [`GPU_WORKGROUP`](#id), [`GPU_INPUT`](#id), [`GPU_OUTPUT`](#id), [`GPU_BUILTIN`](#id), [`GPU_UNIFORM`](#id), [`GPU_STORAGE`](#id), [`GPU_SAMPLER`](#id), [`GPU_PUSH`](#id), [`GPU_SPEC`](#id), [`GPU_SHARED`](#id), [`INTRINSIC`](#id), [`HANDLE`](#id), [`ABI_TYPE`](#id), [`TESTING`](#id), [`EXPECT`](#id), [`THREAD`](#id), [`AT`](#id), [`WEAK`](#id), [`GPU_ATTACHMENT`](#id) | |
| [`row`](#row) | the row of `id`; nil for NONE and any id no row declares |
| [`id_of`](#id_of) | the decorator the name at `name` in `source` spells, NONE when it spells none |
| [`name_of`](#name_of) | the name of `id`; nil for NONE |
| [`admits`](#admits) | whether `id` may stand on the declaration `d` |
| [`flagged`](#flagged) | whether `id`'s row carries every one of `flags` |
| [`list_get`](#list_get) | the first decorator `id` among the `len` decorators from `start` |
| [`list_has`](#list_has) | whether the decorator `id` is among the `len` decorators from `start` |
| [`get`](#get) | the declaration's decorator `id`, the first when it repeats |
| [`has`](#has) | whether the declaration carries the decorator `id` |

| On `Places` | |
| :--- | :--- |
| [`PLACE_CASE`](#places), [`PLACES_ANY`](#places) | |
| [`places_of`](#places_of) | where the declaration `d` stands, for a row's `places` |

| On `Flags` | |
| :--- | :--- |
| [`FLAG_ROLE`](#flags), [`FLAG_TESTING_EXCLUSIVE`](#flags), [`FLAG_STRINGS_CHECKED`](#flags), [`FLAG_PLACE_CHECKED`](#flags), [`FLAG_DEFINES`](#flags) | |
| [`count_flagged`](#count_flagged) | how many distinct decorators the declaration carries that carry every one of `flags`; a repeat is refused as a duplicate, so it is not counted again |
| [`names_append`](#names_append) | appends the names of the rows that carry every one of `flags` and stand somewhere in `places`, `, `-separated in row order |

| On `Rewrite` | |
| :--- | :--- |
| [`rewrite_of`](#rewrite_of) | the rewrite of the decorator named at `name` in `source` with `args` arguments, nil name when it is written as this major spells it |

| Functions | |
| :--- | :--- |
| [`refusal_of`](#refusal_of) | the refusal of the previous spelling at `name` in `source` with `args` arguments, nil when it is none |
| [`constant_time_mode`](#constant_time_mode) | the constant-time mode `d` declares, "soft" or "hard", none when it declares none |
| [`mode_admits`](#mode_admits) | whether `value` is one of the `\|`-separated `modes` |
| [`takes_string`](#takes_string) | whether argument `ord` of `dec` is a string the load records |
| [`string_id`](#string_id) | the string argument `ord` of `dec` evaluated to in the scope `c`; STR_NIL when it is not a constant string |
| [`string_of`](#string_of) | the text of `string_id` |
| [`strings_record`](#strings_record) | records into `into` the string every string argument of a module's decorators evaluates to, with the phase's own evaluator over the scope `c` |

---

## Id

```mach
pub def Id: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` |  |
| `DEPRECATED` | `1` |  |
| `SYMBOL` | `2` |  |
| `SECTION` | `3` |  |
| `INLINE` | `4` |  |
| `EXPORT` | `5` |  |
| `ALIGN` | `6` |  |
| `PACKED` | `7` |  |
| `VOLATILE` | `8` |  |
| `LIBRARY` | `9` |  |
| `CONSTANT_TIME` | `10` |  |
| `VECTORIZE` | `11` |  |
| `NAKED` | `12` |  |
| `EXTENSION` | `13` |  |
| `EMBED` | `14` |  |
| `GPU_STAGE` | `15` |  |
| `GPU_WORKGROUP` | `16` |  |
| `GPU_INPUT` | `17` |  |
| `GPU_OUTPUT` | `18` |  |
| `GPU_BUILTIN` | `19` |  |
| `GPU_UNIFORM` | `20` |  |
| `GPU_STORAGE` | `21` |  |
| `GPU_SAMPLER` | `22` |  |
| `GPU_PUSH` | `23` |  |
| `GPU_SPEC` | `24` |  |
| `GPU_SHARED` | `25` |  |
| `INTRINSIC` | `26` |  |
| `HANDLE` | `27` |  |
| `ABI_TYPE` | `28` |  |
| `TESTING` | `29` |  |
| `EXPECT` | `30` |  |
| `THREAD` | `31` |  |
| `AT` | `32` |  |
| `WEAK` | `33` |  |
| `GPU_ATTACHMENT` | `34` |  |

### row

```mach
pub fun row(id: Id) *Row
```

the row of `id`; nil for NONE and any id no row declares

### id_of

```mach
pub fun id_of(source: str, name: mach.base.source.Span) Id
```

the decorator the name at `name` in `source` spells, NONE when it spells none

### name_of

```mach
pub fun name_of(id: Id) str
```

the name of `id`; nil for NONE

### admits

```mach
pub fun admits(id: Id, d: *mach.front.ast.decl.Decl) bool
```

whether `id` may stand on the declaration `d`

### flagged

```mach
pub fun flagged(id: Id, flags: Flags) bool
```

whether `id`'s row carries every one of `flags`

### list_get

```mach
pub fun list_get(a: *mach.front.ast.Ast, start: u32, len: u32, id: Id) opt[*mach.front.ast.decl.Decorator]
```

the first decorator `id` among the `len` decorators from `start`

### list_has

```mach
pub fun list_has(a: *mach.front.ast.Ast, start: u32, len: u32, id: Id) bool
```

whether the decorator `id` is among the `len` decorators from `start`

### get

```mach
pub fun get(a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl, id: Id) opt[*mach.front.ast.decl.Decorator]
```

the declaration's decorator `id`, the first when it repeats

### has

```mach
pub fun has(a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl, id: Id) bool
```

whether the declaration carries the decorator `id`

## Places

```mach
pub def Places: u32
```

where a decorator may stand: one bit per declaration kind, one for an `ext`
import, one for a tag case, and one for every declaration no decorator names.
PLACES_ANY is every declaration; a tag case admits only a row that names it

| Name | Value | |
| :--- | :--- | :--- |
| `PLACE_CASE` | `0x800` |  |
| `PLACES_ANY` | `0x17FF` |  |

### places_of

```mach
pub fun places_of(d: *mach.front.ast.decl.Decl) Places
```

where the declaration `d` stands, for a row's `places`

## Flags

```mach
pub def Flags: u32
```

what a row asks of its decorator beyond its arguments and places

| Field | Type | |
| :--- | :--- | :--- |
| `FLAG_ROLE` |  | it gives a module binding a shader interface role, and a binding has one |
| `FLAG_TESTING_EXCLUSIVE` |  | it names a consumer outside Mach source, which `#[testing]` cannot confine |
| `FLAG_STRINGS_CHECKED` |  | its string arguments are checked by the row, not by the decorator's own rule |
| `FLAG_PLACE_CHECKED` |  | the decorator refuses a misplacement itself, in a diagnostic of its own kind |
| `FLAG_DEFINES` |  | it supplies the type of a `def` that states none |

| Name | Value | |
| :--- | :--- | :--- |
| `FLAG_ROLE` | `0x2` |  |
| `FLAG_TESTING_EXCLUSIVE` | `0x4` |  |
| `FLAG_STRINGS_CHECKED` | `0x10` |  |
| `FLAG_PLACE_CHECKED` | `0x20` |  |
| `FLAG_DEFINES` | `0x40` |  |

### count_flagged

```mach
pub fun count_flagged(a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl, flags: Flags) u32
```

how many distinct decorators the declaration carries that carry every one of `flags`;
a repeat is refused as a duplicate, so it is not counted again

### names_append

```mach
pub fun names_append(b: *mach.base.textbuild.TextBuilder, flags: Flags, places: Places, conjunction: str) err[mach.base.textbuild.Error]
```

appends the names of the rows that carry every one of `flags` and stand somewhere
in `places`, `, `-separated in row order. with a `conjunction` each name is quoted
in backticks and the conjunction precedes the last, as in "`a`, `b`, and `c`"

## Row

```mach
pub rec Row {
    id:            Id;
    name:          str;
    places:        Places;
    args_min:      u32;
    args_max:      u32;
    strings_first: u32;
    strings_count: u32;
    integers:      u32;
    words:         u32;
    modes:         str;
    flags:         Flags;
    arity:         str;
    argument:      str;
    misplaced:     str;
    duplicate:     str;
}
```

one decorator

| Field | Type | |
| :--- | :--- | :--- |
| `places` | `Places` | where it stands, `misplaced` the refusal of anywhere else args_min, args_max:           the argument count it takes, `arity` the refusal of any other strings_first, strings_count: the arguments that are strings the load records, none when the count is 0 |
| `integers` | `u32` | one bit per argument the row checks as an integer constant, `argument` the refusal of a string or integer argument of the wrong kind |
| `words` | `u32` | one bit per integer argument the row holds to an unsigned 32-bit value, the one literal word its target states it in |
| `modes` | `str` | the values its first argument may take, `\|`-separated, nil when any string is its own; `argument` is the refusal of any other value too |
| `duplicate` | `str` | the refusal of a second one, since every decorator stands at most once |

## Rewrite

```mach
pub rec Rewrite {
    name:  str;
    mode:  str;
    words: bool;
}
```

what `mach migrate` writes for the previous spelling at `name` in `source`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the name this major gives it |
| `mode` | `str` | the argument a previous flag stood for, nil for none |
| `words` | `bool` | its arguments were bare names, which this major writes as strings |

### rewrite_of

```mach
pub fun rewrite_of(source: str, name: mach.base.source.Span, args: u32) Rewrite
```

the rewrite of the decorator named at `name` in `source` with `args` arguments, nil name
when it is written as this major spells it

## Functions

### refusal_of

```mach
pub fun refusal_of(source: str, name: mach.base.source.Span, args: u32) str
```

the refusal of the previous spelling at `name` in `source` with `args` arguments, nil when
it is none

### constant_time_mode

```mach
pub fun constant_time_mode(itn: *mach.base.intern.Interner, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl) opt[str]
```

the constant-time mode `d` declares, "soft" or "hard", none when it declares none. every
reader of the mode asks here, so none disagree

### mode_admits

```mach
pub fun mode_admits(modes: str, value: str) bool
```

whether `value` is one of the `|`-separated `modes`

### takes_string

```mach
pub fun takes_string(dec: *mach.front.ast.decl.Decorator, ord: u32) bool
```

whether argument `ord` of `dec` is a string the load records

### string_id

```mach
pub fun string_id(c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, dec: *mach.front.ast.decl.Decorator, ord: u32) mach.base.intern.StrId
```

the string argument `ord` of `dec` evaluated to in the scope `c`; STR_NIL when it is not a constant string

### string_of

```mach
pub fun string_of(itn: *mach.base.intern.Interner, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, dec: *mach.front.ast.decl.Decorator, ord: u32) opt[str]
```

the text of `string_id`

### strings_record

```mach
pub fun strings_record[T](itn: *mach.base.intern.Interner, into: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, source: str,
    c: *mach.front.comptime.scope.ComptimeCtx, cap_ctx: *T, caps: mach.front.comptime.capability.PhaseCapabilities[T], reached: *bool) err[mach.base.fail.Fail]
```

records into `into` the string every string argument of a module's decorators evaluates to, with
the phase's own evaluator over the scope `c`. an argument that is not a constant string is left
out, and so is a declaration `reached` does not mark, where it marks any

