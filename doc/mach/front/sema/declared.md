[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **declared**

# mach.front.sema.declared

the types declarations and type nodes declare: resolving type nodes, typing nominals and
laying them out, and the occurs and secrecy checks over them

Source: [`src/front/sema/declared.mach`](../../../../src/front/sema/declared.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.failure`](../comptime/failure.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.context`](context.md), [`mach.session`](../../session.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`seed_builtins`](#seed_builtins) | seed the walk's built-in bindings, of which there are none: it does nothing |
| [`timed_of`](#timed_of) | the constant-time mode `#[constant_time]` gives a declaration |
| [`fun_value_type`](#fun_value_type) | the type of a function's value, its signature marked with the mode the function is declared constant time in, so it converts to a function type marked no stronger |
| [`alias_type`](#alias_type) | the type an alias names for its written type: the function type marked with the mode its `#[constant_time]` states, the one alias that is not the type it is written as, else the written type itself |
| [`infer_fun_sig`](#infer_fun_sig) | the function type a declaration's signature declares, its comptime parameters left out and no constant-time mode stated, which `fun_value_type` adds for the function's value |
| [`infer_nominal`](#infer_nominal) | the nominal type a record declaration declares, interned under this module and elaborated |
| [`infer_nominal_uni`](#infer_nominal_uni) | the nominal type a union declaration declares, interned under this module and elaborated |
| [`elaborate_nominal`](#elaborate_nominal) | build and publish the field table and decorations of nominal `ty` from its declaration, once |
| [`ensure_layout_ready`](#ensure_layout_ready) | elaborates every nominal `tid` holds by value, each once; the name of a nominal whose layout is still being decided when it is reached again, nil otherwise |
| [`prepare_nominal_recipe`](#prepare_nominal_recipe) | make sure this module's nominal `ty` has a complete field table, elaborating it when it has none |
| [`resolve_type_query`](#resolve_type_query) | answer the type query `which` about type `tid` for a comptime evaluation in this walk |
| [`answer_type_query`](#answer_type_query) | answer the type query `which` about type `tid`: whether it is secret, holds a secret, is a record, union, tag, pointer, integer or float, or its spelling |
| [`offset_of_member_index`](#offset_of_member_index) | the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload |
| [`field_offset`](#field_offset) | a field descriptor's offset, from the checked type layout |
| [`resolve_layout_intrinsic`](#resolve_layout_intrinsic) | the value of layout intrinsic call `eid` for a comptime evaluation in this walk |
| [`check_occurs_all`](#check_occurs_all) | report every field or case of this module's records, unions and tags that holds its own type by value |
| [`check_uni_secrecy_all`](#check_uni_secrecy_all) | check the secrecy agreement of overlapping variants in every union this module declares, named or anonymous |
| [`declared_align`](#declared_align) | the alignment `d` declares, zero when it declares none or a refused one, which is reported here for a type, a function and a global alike |
| [`intrinsic_operand_type`](#intrinsic_operand_type) | the type a comptime intrinsic's type argument names: a `$fields` field's type or a type node |
| [`resolve_type_ref`](#resolve_type_ref) | the type type node `tid` names, memoized per node except where it names a `$fields` field type |
| [`type_for_symbol`](#type_for_symbol) | the type a symbol written bare in type position names: a primitive, a vector, a type parameter, a nominal, an alias, or an imported type |
| [`def_underlying`](#def_underlying) | the type the alias `sym` stands for: its definition resolved, the target's layout for a bodyless `#[abi_type]`, a handle for any other bodyless `def`, or the type another module published |
| [`resolve_type_anon`](#resolve_type_anon) | the anonymous record or union type a type node spells, keyed by its fields, with its field table |
| [`ptr_to`](#ptr_to) | the pointer type to `base` |
| [`prim_or_error`](#prim_or_error) | the primitive type of kind `kind`, TYPE_ERROR when there is none |
| [`intern_span_or_nil`](#intern_span_or_nil) | the interned text of `span` in this view's source |
| [`allocate_params`](#allocate_params) | an array of `count` type ids, owned by the caller and released with params_free |
| [`params_free`](#params_free) | release an array allocate_params returned, `count` long |

---

## Functions

### seed_builtins

```mach
pub fun seed_builtins(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

seed the walk's built-in bindings, of which there are none: it does nothing

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| **returns** | always ok |

### timed_of

```mach
pub fun timed_of(sc: *mach.front.sema.context.SemaContext, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl) u8
```

the constant-time mode `#[constant_time]` gives a declaration

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `c` | the decorator scope the mode's argument is read under |
| `a` | the declaration's syntax |
| `d` | the declaration |
| **returns** | TIMED_SOFT or TIMED_HARD as the decorator states, TIMED_NONE when it is absent |

### fun_value_type

```mach
pub fun fun_value_type(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, sig: mach.type.kind.TypeId) mach.type.kind.TypeId
```

the type of a function's value, its signature marked with the mode the function is declared
constant time in, so it converts to a function type marked no stronger. a `#[weak]` default,
which another definition may replace, and a declaration with no body of its own state no mode

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the function declaration |
| `sig` | the type its signature declares |
| **returns** | `sig` marked with the function's mode, `sig` itself when it states none, or TYPE_ERROR when interning fails, which fails the phase as internal |

### alias_type

```mach
pub fun alias_type(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, written: mach.type.kind.TypeId) mach.type.kind.TypeId
```

the type an alias names for its written type: the function type marked with the mode its
`#[constant_time]` states, the one alias that is not the type it is written as, else the
written type itself. a misplaced mark is refused by the decorator's validation and names
the written type

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the alias declaration |
| `written` | the type written after its colon |
| **returns** | the type the alias names, or TYPE_ERROR when interning fails, which fails the phase as internal |

### infer_fun_sig

```mach
pub fun infer_fun_sig(sc: *mach.front.sema.context.SemaContext, f: *mach.front.ast.decl.DeclFun) mach.type.kind.TypeId
```

the function type a declaration's signature declares, its comptime parameters left out and
no constant-time mode stated, which `fun_value_type` adds for the function's value. a comptime
parameter that holds a secret is reported

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `f` | the function declaration |
| **returns** | the interned function type, or TYPE_ERROR when interning or allocation fails, which fails the phase as internal |

### infer_nominal

```mach
pub fun infer_nominal(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, name_span: mach.base.source.Span, r: *mach.front.ast.decl.DeclRec, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

the nominal type a record declaration declares, interned under this module and elaborated

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `did` | the declaration |
| `name_span` | the declared name |
| `r` | the record's syntax |
| `kind` | the nominal's type kind |
| **returns** | the type, or TYPE_ERROR when interning fails, which fails the phase as internal |

### infer_nominal_uni

```mach
pub fun infer_nominal_uni(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, name_span: mach.base.source.Span, u: *mach.front.ast.decl.DeclUni, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

the nominal type a union declaration declares, interned under this module and elaborated

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `did` | the declaration |
| `name_span` | the declared name |
| `u` | the union's syntax |
| `kind` | the nominal's type kind |
| **returns** | the type, or TYPE_ERROR when interning fails, which fails the phase as internal |

### elaborate_nominal

```mach
pub fun elaborate_nominal(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, ty: mach.type.kind.TypeId,
    fields_start: u32, fields_len: u32) mach.base.intern.StrId
```

build and publish the field table and decorations of nominal `ty` from its declaration, once.
a tag's discriminator is recorded first, and what the declaration reports goes out whatever
walk reached it

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `did` | the declaration |
| `ty` | the nominal type |
| `fields_start` | its first field, or first case for a tag |
| `fields_len` | how many fields or cases |
| **returns** | the name of the nominal when it is reached again while its layout is being decided, nil otherwise |

### ensure_layout_ready

```mach
pub fun ensure_layout_ready(sc: *mach.front.sema.context.SemaContext, tid: mach.type.kind.TypeId) mach.base.intern.StrId
```

elaborates every nominal `tid` holds by value, each once; the name of a nominal whose
layout is still being decided when it is reached again, nil otherwise

### prepare_nominal_recipe

```mach
pub fun prepare_nominal_recipe(sc: *mach.front.sema.context.SemaContext, ty: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

make sure this module's nominal `ty` has a complete field table, elaborating it when it has none

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `ty` | the nominal type |
| **returns** | the failure when it has no declaration here, the walk was refused, or the table is still incomplete because of a layout cycle |

### resolve_type_query

```mach
pub fun resolve_type_query(sc: *mach.front.sema.context.SemaContext, tid: u32, which: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

answer the type query `which` about type `tid` for a comptime evaluation in this walk

| Parameter | |
| :--- | :--- |
| `sc` | the walk, nil for none |
| `tid` | the type asked about |
| `which` | the query, as `comptime_capability.TYPE_QUERY_*` |
| **returns** | as answer_type_query, and none with no walk |

### answer_type_query

```mach
pub fun answer_type_query(s: *mach.session.Session, tid: u32, which: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

answer the type query `which` about type `tid`: whether it is secret, holds a secret, is a
record, union, tag, pointer, integer or float, or its spelling. a secret type answers false to
every shape query

| Parameter | |
| :--- | :--- |
| `s` | the session holding the types |
| `tid` | the type asked about |
| `which` | the query, as `comptime_capability.TYPE_QUERY_*` |
| **returns** | the answer, none for a nil or error type or one that waits on a type parameter, or the failure: a refused spelling under its own kind, anything else internal |

### offset_of_member_index

```mach
pub fun offset_of_member_index(sc: *mach.front.sema.context.SemaContext, c: *mach.front.ast.expr.ExprCall, owner: mach.type.kind.TypeId, span: mach.base.source.Span, silent: bool) res[opt[u32], mach.base.fail.Fail]
```

the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload

### field_offset

```mach
pub fun field_offset(sc: *mach.front.sema.context.SemaContext, owner: u32, index: u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

a field descriptor's offset, from the checked type layout

### resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(sc: *mach.front.sema.context.SemaContext, eid: u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

the value of layout intrinsic call `eid` for a comptime evaluation in this walk

| Parameter | |
| :--- | :--- |
| `sc` | the walk, nil for none |
| `eid` | the expression |
| **returns** | the value, none when `eid` is no layout intrinsic call or there is no walk, or the failure: a refused measurement, already reported, or an internal one |

### check_occurs_all

```mach
pub fun check_occurs_all(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

report every field or case of this module's records, unions and tags that holds its own type
by value

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| **returns** | the failure when the scratch table cannot be allocated, which also fails the phase |

### check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

check the secrecy agreement of overlapping variants in every union this module declares, named
or anonymous

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| **returns** | the failure when the scratch table cannot be allocated, which also fails the phase |

### declared_align

```mach
pub fun declared_align(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl) u32
```

the alignment `d` declares, zero when it declares none or a refused one, which is
reported here for a type, a function and a global alike

### intrinsic_operand_type

```mach
pub fun intrinsic_operand_type(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId, span: mach.base.source.Span) mach.type.kind.TypeId
```

the type a comptime intrinsic's type argument names: a `$fields` field's type or a type node

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `eid` | the argument expression |
| `span` | where a refusal is reported |
| **returns** | the type, or TYPE_ERROR when it is refused or was not parsed as a type |

### resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *mach.front.sema.context.SemaContext, tid: mach.front.ast.id.TypeId) mach.type.kind.TypeId
```

the type type node `tid` names, memoized per node except where it names a `$fields` field type.
what a memoized node reports it reports the first time, whatever walk reached it

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `tid` | the type node |
| **returns** | the type, or TYPE_ERROR when it is refused |

### type_for_symbol

```mach
pub fun type_for_symbol(sc: *mach.front.sema.context.SemaContext, sym: *mach.front.resolve.Symbol, span: mach.base.source.Span) mach.type.kind.TypeId
```

the type a symbol written bare in type position names: a primitive, a vector, a type
parameter, a nominal, an alias, or an imported type. a generic named without its arguments,
and a symbol that is no type, are reported

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `sym` | the symbol |
| `span` | where a refusal is reported |
| **returns** | the type, or TYPE_ERROR when it is refused |

### def_underlying

```mach
pub fun def_underlying(sc: *mach.front.sema.context.SemaContext, sym: *mach.front.resolve.Symbol, span: mach.base.source.Span) mach.type.kind.TypeId
```

the type the alias `sym` stands for: its definition resolved, the target's layout for a bodyless
`#[abi_type]`, a handle for any other bodyless `def`, or the type another module published. an
alias that reaches itself is reported

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `sym` | the alias symbol |
| `span` | where a cycle is reported |
| **returns** | the type, or TYPE_ERROR when it is refused |

### resolve_type_anon

```mach
pub fun resolve_type_anon(sc: *mach.front.sema.context.SemaContext, tid: mach.front.ast.id.TypeId, fields_start: u32, fields_len: u32, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

the anonymous record or union type a type node spells, keyed by its fields, with its field table

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `tid` | the type node |
| `fields_start` | its first field |
| `fields_len` | how many fields |
| `kind` | TYPE_REC or TYPE_UNI |
| **returns** | the type, or TYPE_ERROR when a field fails to type, interning fails or its field table cannot be built |

### ptr_to

```mach
pub fun ptr_to(sc: *mach.front.sema.context.SemaContext, base: mach.type.kind.TypeId) mach.type.kind.TypeId
```

the pointer type to `base`

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `base` | the pointee |
| **returns** | the type, or TYPE_ERROR for an error pointee or when interning fails, which fails the phase as internal |

### prim_or_error

```mach
pub fun prim_or_error(sc: *mach.front.sema.context.SemaContext, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

the primitive type of kind `kind`, TYPE_ERROR when there is none

### intern_span_or_nil

```mach
pub fun intern_span_or_nil(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span) mach.base.intern.StrId
```

the interned text of `span` in this view's source

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `span` | the span |
| **returns** | the interned text, nil for an empty span or when interning fails, which fails the phase as internal |

### allocate_params

```mach
pub fun allocate_params(sc: *mach.front.sema.context.SemaContext, count: u32) res[*mach.type.kind.TypeId, mach.base.fail.Fail]
```

an array of `count` type ids, owned by the caller and released with params_free

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `count` | how many |
| **returns** | the array, or the out-of-memory failure |

### params_free

```mach
pub fun params_free(sc: *mach.front.sema.context.SemaContext, params: *mach.type.kind.TypeId, count: u32)
```

release an array allocate_params returned, `count` long

