[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **declared**

# mach.front.sema.declared

the types declarations and type nodes declare: resolving type nodes, typing nominals and
laying them out, and the occurs and secrecy checks over them

Source: [`src/front/sema/declared.mach`](../../../../src/front/sema/declared.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.failure`](../comptime/failure.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.context`](context.md), [`mach.session`](../../session.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`seed_builtins`](#seed_builtins), [`infer_fun_sig`](#infer_fun_sig), [`infer_nominal`](#infer_nominal), [`infer_nominal_uni`](#infer_nominal_uni), [`elaborate_nominal`](#elaborate_nominal) | |
| [`ensure_layout_ready`](#ensure_layout_ready) | elaborates every nominal `tid` holds by value, each once; the name of a nominal whose layout is still being decided when it is reached again, nil otherwise |
| [`prepare_nominal_recipe`](#prepare_nominal_recipe), [`resolve_type_query`](#resolve_type_query), [`answer_type_query`](#answer_type_query) | |
| [`offset_of_member_index`](#offset_of_member_index) | the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload |
| [`field_offset`](#field_offset) | a field descriptor's offset, from the checked type layout |
| [`resolve_layout_intrinsic`](#resolve_layout_intrinsic), [`check_occurs_all`](#check_occurs_all), [`check_uni_secrecy_all`](#check_uni_secrecy_all) | |
| [`declared_align`](#declared_align) | the alignment `d` declares, zero when it declares none or a refused one, which is reported here for a type, a function and a global alike |
| [`intrinsic_operand_type`](#intrinsic_operand_type), [`resolve_type_ref`](#resolve_type_ref), [`type_for_symbol`](#type_for_symbol), [`def_underlying`](#def_underlying), [`resolve_type_anon`](#resolve_type_anon), [`ptr_to`](#ptr_to), [`prim_or_error`](#prim_or_error), [`intern_span_or_nil`](#intern_span_or_nil), [`allocate_params`](#allocate_params), [`params_free`](#params_free) | |

---

## Functions

### seed_builtins

```mach
pub fun seed_builtins(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

### infer_fun_sig

```mach
pub fun infer_fun_sig(sc: *mach.front.sema.context.SemaContext, f: *mach.front.ast.decl.DeclFun) mach.type.kind.TypeId
```

### infer_nominal

```mach
pub fun infer_nominal(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, name_span: mach.base.source.Span, r: *mach.front.ast.decl.DeclRec, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

### infer_nominal_uni

```mach
pub fun infer_nominal_uni(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, name_span: mach.base.source.Span, u: *mach.front.ast.decl.DeclUni, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

### elaborate_nominal

```mach
pub fun elaborate_nominal(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, ty: mach.type.kind.TypeId,
    fields_start: u32, fields_len: u32) mach.base.intern.StrId
```

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

### resolve_type_query

```mach
pub fun resolve_type_query(sc: *mach.front.sema.context.SemaContext, tid: u32, which: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

### answer_type_query

```mach
pub fun answer_type_query(s: *mach.session.Session, tid: u32, which: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

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

### check_occurs_all

```mach
pub fun check_occurs_all(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

### check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *mach.front.sema.context.SemaContext) err[mach.base.fail.Fail]
```

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

### resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *mach.front.sema.context.SemaContext, tid: mach.front.ast.id.TypeId) mach.type.kind.TypeId
```

### type_for_symbol

```mach
pub fun type_for_symbol(sc: *mach.front.sema.context.SemaContext, sym: *mach.front.resolve.Symbol, span: mach.base.source.Span) mach.type.kind.TypeId
```

### def_underlying

```mach
pub fun def_underlying(sc: *mach.front.sema.context.SemaContext, sym: *mach.front.resolve.Symbol, span: mach.base.source.Span) mach.type.kind.TypeId
```

### resolve_type_anon

```mach
pub fun resolve_type_anon(sc: *mach.front.sema.context.SemaContext, tid: mach.front.ast.id.TypeId, fields_start: u32, fields_len: u32, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

### ptr_to

```mach
pub fun ptr_to(sc: *mach.front.sema.context.SemaContext, base: mach.type.kind.TypeId) mach.type.kind.TypeId
```

### prim_or_error

```mach
pub fun prim_or_error(sc: *mach.front.sema.context.SemaContext, kind: mach.type.kind.TypeKind) mach.type.kind.TypeId
```

### intern_span_or_nil

```mach
pub fun intern_span_or_nil(sc: *mach.front.sema.context.SemaContext, span: mach.base.source.Span) mach.base.intern.StrId
```

### allocate_params

```mach
pub fun allocate_params(sc: *mach.front.sema.context.SemaContext, count: u32) res[*mach.type.kind.TypeId, mach.base.fail.Fail]
```

### params_free

```mach
pub fun params_free(sc: *mach.front.sema.context.SemaContext, params: *mach.type.kind.TypeId, count: u32)
```

