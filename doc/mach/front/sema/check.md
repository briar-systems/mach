[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **check**

# mach.front.sema.check

the type rules sema holds each site to: assignments, calls, indexes, members and casts

each check takes the types the walk found at a site, reports a refusal there with the types
spelled and, where one can be offered, the cast that would fix it, and answers whether the site
holds or the type it gives. returns, conditions, shift counts and constant ranges are checked
here too, beside the size of a type as sema measures it and the reports the other checks share.

Source: [`src/front/sema/check.mach`](../../../../src/front/sema/check.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.coerce`](coerce.md), [`mach.front.sema.context`](context.md), [`mach.front.transition`](../transition.md), [`mach.type.kind`](../../type/kind.md), [`mach.type.query.secrecy`](../../type/query/secrecy.md).

| Type | | |
| :--- | :--- | :--- |
| [`Written`](#written) | `tag` | where an expected type was written, found only once a mismatch is reported: an annotation the syntax `a` holds, a parameter of the function a callee names, or a field of the record or union a literal head names, by its position |
| [`Annotation`](#annotation-1) | `rec` | a type annotation and the syntax that holds it |
| [`Member`](#member) | `rec` | the function a callee expression names, or the record or union a literal's head names, and the position of a parameter or field in it |

| On `Written` | |
| :--- | :--- |
| [`unwritten`](#unwritten) | an expected type written nowhere a mismatch can name |
| [`annotation`](#annotation) | an expected type written as the annotation `ty` of the syntax `a` |
| [`parameter`](#parameter) | an expected type written as a parameter of the function `callee` names |
| [`field`](#field) | an expected type written as a field of the record or union a literal's `head` names |
| [`check_coercible`](#check_coercible) | whether expression `eid` may be assigned where `expected` is, after a literal in it takes that type |

| Functions | |
| :--- | :--- |
| [`check_call`](#check_call) | check a call's arguments against the callee's signature: the count, each fixed argument's type, pack spreads, and what a C-variadic or pack argument may be |
| [`check_comptime_call`](#check_comptime_call) | check a call to a function with comptime parameters: each comptime argument is evaluated and held to its parameter's type, each runtime one checked as an assignment, and a call whose comptime values are all decided asks for the value instance they make |
| [`imported_module_const_sym`](#imported_module_const_sym) | whether `sym` is an import of another module's `val` |
| [`check_index`](#check_index) | the type of an index into a value of type `object`, reporting a secret index, a secret address, an object that is not a pointer or array, and an index that is not an integer |
| [`check_const_index`](#check_const_index) | whether a constant index `idx` lies inside the array or vector type `object`, reporting it at `span` when not |
| [`check_range_count`](#check_range_count) | the count of a range `x[i, n]`: an integer comptime constant, at least one |
| [`check_const_range`](#check_const_range) | a constant start must keep the whole range inside an array or a vector: `start + count` may reach the length and not pass it |
| [`const_index_value`](#const_index_value) | the integer constant expression `eid` evaluates to, none when it is nil, no constant or no integer |
| [`check_shift_count`](#check_shift_count) | a shift whose count is a comptime constant must keep the count below the left operand's width: at or above it the runtime answer is the saturated value (0, or the sign fill), which a constant program never means |
| [`check_secret_address`](#check_secret_address) | whether a value of type `ty` may be used as an address: a secret pointer is reported at `span` |
| [`report_untyped_access`](#report_untyped_access) | the refusal of `access` read through the untyped `ptr`, which names no type to read |
| [`check_member`](#check_member) | the type of member `name` of a value of type `object`, through one pointer: a record or union field, or a tag payload, which needs an open guard for its case |
| [`check_cast`](#check_cast) | check a `::` cast from `from` to `to`: numeric conversion, vector lanes, arrays and vectors of the same element, equal-size values, and on a logical-addressing target a pointer to or from a 64-bit integer |
| [`report_secrecy`](#report_secrecy) | a secrecy refusal whose message names the field its answer turned on: where the classes differ, or the instance that grows and what kept it from lining up with the other side |
| [`check_reinterpret`](#check_reinterpret) | check a `:~` reinterpret from `from` to `to`, which needs two types of equal size and refuses what a `::` cast refuses of secrecy, tags and ABI types |
| [`check_return`](#check_return) | check a `ret` against the function's result type |
| [`check_condition`](#check_condition) | check a branch or loop condition, which must be a public `u1` |
| [`refuse_form`](#refuse_form) | form `form` of a previous major written at `span`, refused naming what this major spells |
| [`byte_size`](#byte_size) | the size of a value of `t`, 0 when it has none or does not fit 32 bits; an array is its element's size times every count above it, read in a loop, so an array nested through flat defs deeper than the stack still has its size |
| [`type_to_str`](#type_to_str) | the spelling of type `t`, owned by the caller and released with type_str_free, or the failure |
| [`type_str_free`](#type_str_free) | release a spelling type_to_str returned |
| [`report_no_field`](#report_no_field) | report that type `record` has no field `name` |
| [`report_named`](#report_named) | report a diagnostic of kind `k` whose message is `prefix`, the text of `name`, then `suffix` |
| [`report_missing_type_args`](#report_missing_type_args) | report a generic type named without its `expected` type arguments |
| [`report_missing_call_type_args`](#report_missing_call_type_args) | report a generic function called without its `expected` type arguments |
| [`report_typed`](#report_typed) | report a diagnostic of kind `k` whose message is `prefix`, the spelling of `t`, then `suffix` |
| [`report_typed2`](#report_typed2) | report a diagnostic of kind `k` spelling two types between `prefix`, `mid` and `suffix`, with a note naming their modules when two distinct types spell the same |
| [`expr_span_of`](#expr_span_of) | the span of expression `eid`, or the failure reading it, which also fails the phase as internal |
| [`report_out_of_range`](#report_out_of_range) | |
| [`report_default_out_of_range`](#report_default_out_of_range) | the range refusal of a literal no context typed, which took the i64 default |
| [`report_cast_out_of_range`](#report_cast_out_of_range) | the range refusal of a literal a cast to `to` reinterprets, which took an integer of `to`'s size |
| [`report_literal_out_of_range`](#report_literal_out_of_range) | the range refusal of a suffixed literal its own suffix's type does not hold |

| Constants | |
| :--- | :--- |
| [`GROWTH_NOTE`](#constants) | |

---

## Written

```mach
pub tag Written: u8 {
    none;
    annotation: Annotation;
    parameter:  Member;
    field:      Member;
}
```

where an expected type was written, found only once a mismatch is reported: an annotation
the syntax `a` holds, a parameter of the function a callee names, or a field of the record
or union a literal head names, by its position

| Case | Payload | |
| :--- | :--- | :--- |
| `none` |  | written nowhere a mismatch can name |
| `annotation` | `Annotation` | a type annotation |
| `parameter` | `Member` | a parameter of the function a callee names |
| `field` | `Member` | a field of the record or union a literal head names |

### unwritten

```mach
pub fun unwritten() Written
```

an expected type written nowhere a mismatch can name

| Parameter | |
| :--- | :--- |
| **returns** | the `none` case |

### annotation

```mach
pub fun annotation(a: *mach.front.ast.Ast, ty: mach.front.ast.id.TypeId) Written
```

an expected type written as the annotation `ty` of the syntax `a`

| Parameter | |
| :--- | :--- |
| `a` | the syntax holding the annotation |
| `ty` | the annotation's type node |
| **returns** | the `annotation` case |

### parameter

```mach
pub fun parameter(callee: mach.front.ast.id.ExprId, index: u32) Written
```

an expected type written as a parameter of the function `callee` names

| Parameter | |
| :--- | :--- |
| `callee` | the callee expression |
| `index` | the parameter's position |
| **returns** | the `parameter` case |

### field

```mach
pub fun field(head: mach.front.ast.id.TypeId, index: u32) Written
```

an expected type written as a field of the record or union a literal's `head` names

| Parameter | |
| :--- | :--- |
| `head` | the literal's head |
| `index` | the field's position |
| **returns** | the `field` case |

### check_coercible

```mach
pub fun check_coercible(sc: *mach.front.sema.context.SemaContext, expected: mach.type.kind.TypeId, eid: mach.front.ast.id.ExprId, actual: mach.type.kind.TypeId, span: mach.base.source.Span, written: Written) bool
```

whether expression `eid` may be assigned where `expected` is, after a literal in it takes
that type. a range miss or a mismatch is reported at `span`, a mismatch a numeric cast would
fix with that cast offered

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `expected` | the type expected at the site |
| `eid` | the expression |
| `actual` | the type the walk gave it |
| `span` | where a refusal is reported |
| `written` | where the expected type was written, whose spelling a mismatch names, or `unwritten()` |
| **returns** | true when the site holds, false when refused or the expression cannot be read |

## Annotation

```mach
pub rec Annotation {
    a:  *mach.front.ast.Ast;
    ty: mach.front.ast.id.TypeId;
}
```

a type annotation and the syntax that holds it

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*mach.front.ast.Ast` | the syntax holding it, nil for none |
| `ty` | `mach.front.ast.id.TypeId` | the annotation's type node |

## Member

```mach
pub rec Member {
    callee: mach.front.ast.id.ExprId;
    head:   mach.front.ast.id.TypeId;
    index:  u32;
}
```

the function a callee expression names, or the record or union a literal's head names, and
the position of a parameter or field in it

| Field | Type | |
| :--- | :--- | :--- |
| `callee` | `mach.front.ast.id.ExprId` | the callee expression, nil for a field |
| `head` | `mach.front.ast.id.TypeId` | the literal's head, nil for a parameter |
| `index` | `u32` | the parameter's or field's position |

## Functions

### check_call

```mach
pub fun check_call(sc: *mach.front.sema.context.SemaContext, callee: mach.front.ast.id.ExprId, callee_sig: mach.type.kind.TypeId, args_start: u32, args_len: u32, span: mach.base.source.Span, pack_to_c_tail: bool) bool
```

check a call's arguments against the callee's signature: the count, each fixed argument's
type, pack spreads, and what a C-variadic or pack argument may be

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `callee` | the callee expression, whose declaration a mismatched argument's parameter annotation is read from |
| `callee_sig` | the callee's type |
| `args_start` | the first argument in the module's expression id list |
| `args_len` | how many arguments |
| `span` | where a refusal of the callee or the count is reported |
| `pack_to_c_tail` | whether the pack's elements reach a C-variadic tail, so each is held to its rules |
| **returns** | true when the call holds or the callee's type is an error |

### check_comptime_call

```mach
pub fun check_comptime_call(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId, callee_sig: mach.type.kind.TypeId, callee: mach.front.ast.id.ExprId, f: *mach.front.ast.decl.DeclFun, args_start: u32, args_len: u32, span: mach.base.source.Span) bool
```

check a call to a function with comptime parameters: each comptime argument is evaluated and
held to its parameter's type, each runtime one checked as an assignment, and a call whose
comptime values are all decided asks for the value instance they make

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `eid` | the call, which the instance is recorded at |
| `callee_sig` | the callee's runtime signature |
| `callee` | the callee expression |
| `f` | the callee's declaration |
| `args_start` | the first argument in the module's expression id list |
| `args_len` | how many arguments |
| `span` | where an arity refusal is reported |
| **returns** | true when the call holds, or the callee's declaring syntax cannot be had |

### imported_module_const_sym

```mach
pub fun imported_module_const_sym(sc: *mach.front.sema.context.SemaContext, sym: *mach.front.resolve.Symbol) bool
```

whether `sym` is an import of another module's `val`

### check_index

```mach
pub fun check_index(sc: *mach.front.sema.context.SemaContext, object: mach.type.kind.TypeId, index: mach.type.kind.TypeId, span: mach.base.source.Span) opt[mach.type.kind.TypeId]
```

the type of an index into a value of type `object`, reporting a secret index, a secret
address, an object that is not a pointer or array, and an index that is not an integer

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `object` | the indexed value's type |
| `index` | the index's type |
| `span` | where a refusal is reported |
| **returns** | the element type, secret when the object is, TYPE_ERROR for an error object, or none when the object cannot be indexed |

### check_const_index

```mach
pub fun check_const_index(sc: *mach.front.sema.context.SemaContext, object: mach.type.kind.TypeId, idx: mach.front.ast.id.ExprId, span: mach.base.source.Span) bool
```

whether a constant index `idx` lies inside the array or vector type `object`, reporting it at
`span` when not. a pointer, or an index that is no constant, holds

### check_range_count

```mach
pub fun check_range_count(sc: *mach.front.sema.context.SemaContext, count: mach.front.ast.id.ExprId, count_type: mach.type.kind.TypeId) opt[u32]
```

the count of a range `x[i, n]`: an integer comptime constant, at least one.
absent, reported at the count, when it is not

### check_const_range

```mach
pub fun check_const_range(sc: *mach.front.sema.context.SemaContext, object: mach.type.kind.TypeId, start: mach.front.ast.id.ExprId, count: u32, span: mach.base.source.Span) bool
```

a constant start must keep the whole range inside an array or a vector:
`start + count` may reach the length and not pass it. a pointer has no length

### const_index_value

```mach
pub fun const_index_value(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) opt[mach.front.comptime.value.CTValue]
```

the integer constant expression `eid` evaluates to, none when it is nil, no constant or no integer

### check_shift_count

```mach
pub fun check_shift_count(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, lt: mach.type.kind.TypeId, count: mach.front.ast.id.ExprId) bool
```

a shift whose count is a comptime constant must keep the count below the
left operand's width: at or above it the runtime answer is the saturated
value (0, or the sign fill), which a constant program never means

### check_secret_address

```mach
pub fun check_secret_address(sc: *mach.front.sema.context.SemaContext, ty: mach.type.kind.TypeId, span: mach.base.source.Span) bool
```

whether a value of type `ty` may be used as an address: a secret pointer is reported at `span`

### report_untyped_access

```mach
pub fun report_untyped_access(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, access: str)
```

the refusal of `access` read through the untyped `ptr`, which names no type to read

### check_member

```mach
pub fun check_member(sc: *mach.front.sema.context.SemaContext, object: mach.type.kind.TypeId, name: mach.base.intern.StrId, hold: mach.front.sema.context.Hold, span: mach.base.source.Span) opt[mach.type.kind.TypeId]
```

the type of member `name` of a value of type `object`, through one pointer: a record or
union field, or a tag payload, which needs an open guard for its case

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `object` | the value's type |
| `name` | the member's name |
| `hold` | what the open guards say of this place's case |
| `span` | where a refusal is reported |
| **returns** | the member's type, secret when its container is, TYPE_ERROR for an error object, or none when it is refused |

### check_cast

```mach
pub fun check_cast(sc: *mach.front.sema.context.SemaContext, from: mach.type.kind.TypeId, to: mach.type.kind.TypeId, span: mach.base.source.Span) opt[mach.type.kind.TypeId]
```

check a `::` cast from `from` to `to`: numeric conversion, vector lanes, arrays and vectors of
the same element, equal-size values, and on a logical-addressing target a pointer to or from a
64-bit integer. a cast that touches secrecy, a tag's representation or an ABI type is refused

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `from` | the operand's type |
| `to` | the type cast to |
| `span` | where a refusal is reported |
| **returns** | `to` when the cast holds, none when it is refused |

### report_secrecy

```mach
pub fun report_secrecy(sc: *mach.front.sema.context.SemaContext, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, head: str, v: mach.type.query.secrecy.Verdict, note: str)
```

a secrecy refusal whose message names the field its answer turned on: where the classes
differ, or the instance that grows and what kept it from lining up with the other side

### check_reinterpret

```mach
pub fun check_reinterpret(sc: *mach.front.sema.context.SemaContext, from: mach.type.kind.TypeId, to: mach.type.kind.TypeId, span: mach.base.source.Span) opt[mach.type.kind.TypeId]
```

check a `:~` reinterpret from `from` to `to`, which needs two types of equal size and refuses
what a `::` cast refuses of secrecy, tags and ABI types

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `from` | the operand's type |
| `to` | the type reinterpreted to |
| `span` | where a refusal is reported |
| **returns** | `to` when it holds, none when it is refused |

### check_return

```mach
pub fun check_return(sc: *mach.front.sema.context.SemaContext, fn_ret: mach.type.kind.TypeId, value: mach.type.kind.TypeId, span: mach.base.source.Span) bool
```

check a `ret` against the function's result type

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `fn_ret` | the function's result type, nil for none |
| `value` | the returned value's type, nil for a bare `ret` |
| `span` | where a refusal is reported |
| **returns** | true when the return holds |

### check_condition

```mach
pub fun check_condition(sc: *mach.front.sema.context.SemaContext, cond: mach.type.kind.TypeId, span: mach.base.source.Span) bool
```

check a branch or loop condition, which must be a public `u1`. a secret, a vector, a `u8`
and any other type are refused at `span`

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `cond` | the condition's type |
| `span` | where a refusal is reported |
| **returns** | true when the condition holds, or its type is an error or awaits an instance |

### refuse_form

```mach
pub fun refuse_form(sc: *mach.front.sema.context.SemaContext, form: mach.front.transition.Form, span: mach.base.source.Span)
```

form `form` of a previous major written at `span`, refused naming what this major spells

### byte_size

```mach
pub fun byte_size(sc: *mach.front.sema.context.SemaContext, t: mach.type.kind.TypeId, span: mach.base.source.Span) u32
```

the size of a value of `t`, 0 when it has none or does not fit 32 bits; an array is its
element's size times every count above it, read in a loop, so an array nested through flat
defs deeper than the stack still has its size. a measurement asked at `span` that refuses
the input is reported there

### type_to_str

```mach
pub fun type_to_str(sc: *mach.front.sema.context.SemaContext, t: mach.type.kind.TypeId) res[str, mach.base.fail.Fail]
```

the spelling of type `t`, owned by the caller and released with type_str_free, or the failure

### type_str_free

```mach
pub fun type_str_free(sc: *mach.front.sema.context.SemaContext, owned: str)
```

release a spelling type_to_str returned

### report_no_field

```mach
pub fun report_no_field(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, name: mach.base.intern.StrId, record: mach.type.kind.TypeId)
```

report that type `record` has no field `name`

### report_named

```mach
pub fun report_named(sc: *mach.front.sema.context.SemaContext, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, prefix: str, name: mach.base.intern.StrId, suffix: str)
```

report a diagnostic of kind `k` whose message is `prefix`, the text of `name`, then `suffix`

### report_missing_type_args

```mach
pub fun report_missing_type_args(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, name: mach.base.intern.StrId, expected: u32)
```

report a generic type named without its `expected` type arguments

### report_missing_call_type_args

```mach
pub fun report_missing_call_type_args(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, name: mach.base.intern.StrId, expected: u32)
```

report a generic function called without its `expected` type arguments

### report_typed

```mach
pub fun report_typed(sc: *mach.front.sema.context.SemaContext, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, prefix: str, t: mach.type.kind.TypeId, suffix: str)
```

report a diagnostic of kind `k` whose message is `prefix`, the spelling of `t`, then `suffix`

### report_typed2

```mach
pub fun report_typed2(sc: *mach.front.sema.context.SemaContext, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, prefix: str, a: mach.type.kind.TypeId, mid: str, b: mach.type.kind.TypeId, suffix: str)
```

report a diagnostic of kind `k` spelling two types between `prefix`, `mid` and `suffix`, with
a note naming their modules when two distinct types spell the same

### expr_span_of

```mach
pub fun expr_span_of(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) res[mach.base.source.Span, mach.base.fail.Fail]
```

the span of expression `eid`, or the failure reading it, which also fails the phase as internal

### report_out_of_range

```mach
pub fun report_out_of_range(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, cr: mach.front.sema.coerce.CoerceResult)
```

### report_default_out_of_range

```mach
pub fun report_default_out_of_range(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, cr: mach.front.sema.coerce.CoerceResult)
```

the range refusal of a literal no context typed, which took the i64 default

### report_cast_out_of_range

```mach
pub fun report_cast_out_of_range(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, cr: mach.front.sema.coerce.CoerceResult, to: mach.type.kind.TypeId)
```

the range refusal of a literal a cast to `to` reinterprets, which took an integer of `to`'s size

### report_literal_out_of_range

```mach
pub fun report_literal_out_of_range(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span, cr: mach.front.sema.coerce.CoerceResult)
```

the range refusal of a suffixed literal its own suffix's type does not hold

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `GROWTH_NOTE` | `str` | `"a generic whose argument grows through a pointer is compared by the layout of the arguments its fields hold, and arguments that differ in size or alignment cannot be compared. give both sides arguments of the same size and alignment"` |  |

