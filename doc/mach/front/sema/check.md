[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **check**

# mach.front.sema.check

the type rules sema holds each site to: assignments, calls, indexes, members and casts

each check takes the types the walk found at a site, reports a refusal there with the types
spelled and, where one can be offered, the cast that would fix it, and answers whether the site
holds or the type it gives. returns, conditions, shift counts and constant ranges are checked
here too, beside the size of a type as sema measures it and the reports the other checks share.

<sub>source: [`src/front/sema/check.mach`](../../../../src/front/sema/check.mach) · imported by 5 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md) · `secrecy` is [`mach.type.query.secrecy`](../../type/query/secrecy.md) · `transition` is [`mach.front.transition`](../transition.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `coerce` is [`mach.front.sema.coerce`](coerce.md)</sub>

| Functions | |
| :--- | :--- |
| [`check_coercible`](#check_coercible) · [`check_call`](#check_call) · [`check_comptime_call`](#check_comptime_call) · [`imported_module_const_sym`](#imported_module_const_sym) · [`check_index`](#check_index) · [`check_const_index`](#check_const_index) | |
| [`check_range_count`](#check_range_count) | the count of a range `x[i, n]`: a comptime constant, at least one |
| [`check_const_range`](#check_const_range) | a constant start must keep the whole range inside an array or a vector: `start + count` may reach the length and not pass it |
| [`const_index_value`](#const_index_value) | |
| [`check_shift_count`](#check_shift_count) | a shift whose count is a comptime constant must keep the count below the left operand's width: at or above it the runtime answer is the saturated value (0, or the sign fill), which a constant program never means |
| [`check_secret_address`](#check_secret_address) · [`check_member`](#check_member) · [`check_cast`](#check_cast) | |
| [`report_secrecy`](#report_secrecy) | a secrecy refusal whose message names the field its answer turned on: where the classes differ, or the instance that grows and what kept it from lining up with the other side |
| [`check_reinterpret`](#check_reinterpret) · [`check_return`](#check_return) · [`check_condition`](#check_condition) | |
| [`refuse_form`](#refuse_form) | form `form` of a previous major written at `span`, refused naming what this major spells |
| [`byte_size`](#byte_size) | the size of a value of `t`, 0 when it has none or does not fit 32 bits; an array is its element's size times every count above it, read in a loop, so an array nested through flat defs deeper than the stack still has its size |
| [`type_to_str`](#type_to_str) · [`type_str_free`](#type_str_free) · [`report_no_field`](#report_no_field) · [`report_named`](#report_named) · [`report_missing_type_args`](#report_missing_type_args) · [`report_typed`](#report_typed) · [`report_typed2`](#report_typed2) · [`expr_span_of`](#expr_span_of) · [`report_out_of_range`](#report_out_of_range) | |
| [`report_default_out_of_range`](#report_default_out_of_range) | the range refusal of a literal no context typed, which took the i64 default |

| Constants | |
| :--- | :--- |
| [`GROWTH_NOTE`](#constants) | |

---

## Functions

### check_coercible

```mach
pub fun check_coercible(sc: *sema_context.SemaContext, expected: type_kind.TypeId, eid: ast_id.ExprId, actual: type_kind.TypeId, span: base_source.Span) bool
```

### check_call

```mach
pub fun check_call(sc: *sema_context.SemaContext, callee_sig: type_kind.TypeId, args_start: u32, args_len: u32, span: base_source.Span, pack_to_c_tail: bool) bool
```

### check_comptime_call

```mach
pub fun check_comptime_call(sc: *sema_context.SemaContext, eid: ast_id.ExprId, callee_sig: type_kind.TypeId, callee: ast_id.ExprId, f: *ast_decl.DeclFun, args_start: u32, args_len: u32, span: base_source.Span) bool
```

### imported_module_const_sym

```mach
pub fun imported_module_const_sym(sc: *sema_context.SemaContext, sym: *resolve.Symbol) bool
```

### check_index

```mach
pub fun check_index(sc: *sema_context.SemaContext, object: type_kind.TypeId, index: type_kind.TypeId, span: base_source.Span) opt[type_kind.TypeId]
```

### check_const_index

```mach
pub fun check_const_index(sc: *sema_context.SemaContext, object: type_kind.TypeId, idx: ast_id.ExprId, span: base_source.Span) bool
```

### check_range_count

```mach
pub fun check_range_count(sc: *sema_context.SemaContext, count: ast_id.ExprId) opt[u32]
```

the count of a range `x[i, n]`: a comptime constant, at least one. absent,
reported at the count, when it is not

### check_const_range

```mach
pub fun check_const_range(sc: *sema_context.SemaContext, object: type_kind.TypeId, start: ast_id.ExprId, count: u32, span: base_source.Span) bool
```

a constant start must keep the whole range inside an array or a vector:
`start + count` may reach the length and not pass it. a pointer has no length

### const_index_value

```mach
pub fun const_index_value(sc: *sema_context.SemaContext, eid: ast_id.ExprId) opt[comptime_value.CTValue]
```

### check_shift_count

```mach
pub fun check_shift_count(sc: *sema_context.SemaContext, span: base_source.Span, lt: type_kind.TypeId, count: ast_id.ExprId) bool
```

a shift whose count is a comptime constant must keep the count below the
left operand's width: at or above it the runtime answer is the saturated
value (0, or the sign fill), which a constant program never means

### check_secret_address

```mach
pub fun check_secret_address(sc: *sema_context.SemaContext, ty: type_kind.TypeId, span: base_source.Span) bool
```

### check_member

```mach
pub fun check_member(sc: *sema_context.SemaContext, object: type_kind.TypeId, name: intern.StrId, guarded: bool, span: base_source.Span) opt[type_kind.TypeId]
```

### check_cast

```mach
pub fun check_cast(sc: *sema_context.SemaContext, from: type_kind.TypeId, to: type_kind.TypeId, span: base_source.Span) opt[type_kind.TypeId]
```

### report_secrecy

```mach
pub fun report_secrecy(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, head: str, v: secrecy.Verdict, note: str)
```

a secrecy refusal whose message names the field its answer turned on: where the classes
differ, or the instance that grows and what kept it from lining up with the other side

### check_reinterpret

```mach
pub fun check_reinterpret(sc: *sema_context.SemaContext, from: type_kind.TypeId, to: type_kind.TypeId, span: base_source.Span) opt[type_kind.TypeId]
```

### check_return

```mach
pub fun check_return(sc: *sema_context.SemaContext, fn_ret: type_kind.TypeId, value: type_kind.TypeId, span: base_source.Span) bool
```

### check_condition

```mach
pub fun check_condition(sc: *sema_context.SemaContext, cond: type_kind.TypeId, span: base_source.Span) bool
```

### refuse_form

```mach
pub fun refuse_form(sc: *sema_context.SemaContext, form: transition.Form, span: base_source.Span)
```

form `form` of a previous major written at `span`, refused naming what this major spells

### byte_size

```mach
pub fun byte_size(sc: *sema_context.SemaContext, t: type_kind.TypeId, span: base_source.Span) u32
```

the size of a value of `t`, 0 when it has none or does not fit 32 bits; an array is its
element's size times every count above it, read in a loop, so an array nested through flat
defs deeper than the stack still has its size. a measurement asked at `span` that refuses
the input is reported there

### type_to_str

```mach
pub fun type_to_str(sc: *sema_context.SemaContext, t: type_kind.TypeId) res[str, fail.Fail]
```

### type_str_free

```mach
pub fun type_str_free(sc: *sema_context.SemaContext, owned: str)
```

### report_no_field

```mach
pub fun report_no_field(sc: *sema_context.SemaContext, span: base_source.Span, name: intern.StrId, record: type_kind.TypeId)
```

### report_named

```mach
pub fun report_named(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, prefix: str, name: intern.StrId, suffix: str)
```

### report_missing_type_args

```mach
pub fun report_missing_type_args(sc: *sema_context.SemaContext, span: base_source.Span, name: intern.StrId, expected: u32)
```

### report_typed

```mach
pub fun report_typed(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, prefix: str, t: type_kind.TypeId, suffix: str)
```

### report_typed2

```mach
pub fun report_typed2(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, prefix: str, a: type_kind.TypeId, mid: str, b: type_kind.TypeId, suffix: str)
```

### expr_span_of

```mach
pub fun expr_span_of(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[base_source.Span, fail.Fail]
```

### report_out_of_range

```mach
pub fun report_out_of_range(sc: *sema_context.SemaContext, span: base_source.Span, cr: coerce.CoerceResult)
```

### report_default_out_of_range

```mach
pub fun report_default_out_of_range(sc: *sema_context.SemaContext, span: base_source.Span, cr: coerce.CoerceResult)
```

the range refusal of a literal no context typed, which took the i64 default

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `GROWTH_NOTE` | `str` | `"a generic whose argument grows through a pointer is compared by the arguments its fields lay out, never walked. where two laid arguments differ in size or alignment, padding can line the layouts up at one depth and not the next, so the comparison is refused rather than decided"` |  |

