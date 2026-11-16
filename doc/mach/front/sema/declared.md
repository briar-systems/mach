[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **declared**

# mach.front.sema.declared

the types declarations and type nodes declare: resolving type nodes, typing nominals and
laying them out, and the occurs and secrecy checks over them

<sub>source: [`src/front/sema/declared.mach`](../../../../src/front/sema/declared.mach) · imported by 7 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md) · `comptime_failure` is [`mach.front.comptime.failure`](../comptime/failure.md) · `session` is [`mach.session`](../../session.md) · `ast_expr` is [`mach.front.ast.expr`](../ast/expr.md) · `resolve` is [`mach.front.resolve`](../resolve.md)</sub>

| Functions | |
| :--- | :--- |
| [`seed_builtins`](#seed_builtins) · [`infer_fun_sig`](#infer_fun_sig) · [`infer_nominal`](#infer_nominal) · [`infer_nominal_uni`](#infer_nominal_uni) · [`elaborate_nominal`](#elaborate_nominal) | |
| [`ensure_layout_ready`](#ensure_layout_ready) | elaborates every nominal `tid` holds by value, each once; the name of a nominal whose layout is still being decided when it is reached again, nil otherwise |
| [`prepare_nominal_recipe`](#prepare_nominal_recipe) · [`resolve_type_query`](#resolve_type_query) · [`answer_type_query`](#answer_type_query) | |
| [`offset_of_member_index`](#offset_of_member_index) | the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload |
| [`field_offset`](#field_offset) | a field descriptor's offset, from the checked type layout |
| [`resolve_layout_intrinsic`](#resolve_layout_intrinsic) · [`check_occurs_all`](#check_occurs_all) · [`check_uni_secrecy_all`](#check_uni_secrecy_all) | |
| [`declared_align`](#declared_align) | the alignment `d` declares, zero when it declares none or a refused one, which is reported here for a type, a function and a global alike |
| [`intrinsic_operand_type`](#intrinsic_operand_type) · [`resolve_type_ref`](#resolve_type_ref) · [`type_for_symbol`](#type_for_symbol) · [`def_underlying`](#def_underlying) · [`resolve_type_anon`](#resolve_type_anon) · [`ptr_to`](#ptr_to) · [`prim_or_error`](#prim_or_error) · [`intern_span_or_nil`](#intern_span_or_nil) · [`allocate_params`](#allocate_params) · [`params_free`](#params_free) | |

---

## Functions

### seed_builtins

```mach
pub fun seed_builtins(sc: *sema_context.SemaContext) err[fail.Fail]
```

### infer_fun_sig

```mach
pub fun infer_fun_sig(sc: *sema_context.SemaContext, f: *ast_decl.DeclFun) type_kind.TypeId
```

### infer_nominal

```mach
pub fun infer_nominal(sc: *sema_context.SemaContext, did: ast_id.DeclId, name_span: base_source.Span, r: *ast_decl.DeclRec, kind: type_kind.TypeKind) type_kind.TypeId
```

### infer_nominal_uni

```mach
pub fun infer_nominal_uni(sc: *sema_context.SemaContext, did: ast_id.DeclId, name_span: base_source.Span, u: *ast_decl.DeclUni, kind: type_kind.TypeKind) type_kind.TypeId
```

### elaborate_nominal

```mach
pub fun elaborate_nominal(sc: *sema_context.SemaContext, did: ast_id.DeclId, ty: type_kind.TypeId,
    fields_start: u32, fields_len: u32) intern.StrId
```

### ensure_layout_ready

```mach
pub fun ensure_layout_ready(sc: *sema_context.SemaContext, tid: type_kind.TypeId) intern.StrId
```

elaborates every nominal `tid` holds by value, each once; the name of a nominal whose
layout is still being decided when it is reached again, nil otherwise

### prepare_nominal_recipe

```mach
pub fun prepare_nominal_recipe(sc: *sema_context.SemaContext, ty: type_kind.TypeId) err[fail.Fail]
```

### resolve_type_query

```mach
pub fun resolve_type_query(sc: *sema_context.SemaContext, tid: u32, which: u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

### answer_type_query

```mach
pub fun answer_type_query(s: *session.Session, tid: u32, which: u8) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

### offset_of_member_index

```mach
pub fun offset_of_member_index(sc: *sema_context.SemaContext, c: *ast_expr.ExprCall, owner: type_kind.TypeId, span: base_source.Span, silent: bool) res[opt[u32], fail.Fail]
```

the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload

### field_offset

```mach
pub fun field_offset(sc: *sema_context.SemaContext, owner: u32, index: u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

a field descriptor's offset, from the checked type layout

### resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(sc: *sema_context.SemaContext, eid: u32) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

### check_occurs_all

```mach
pub fun check_occurs_all(sc: *sema_context.SemaContext) err[fail.Fail]
```

### check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *sema_context.SemaContext) err[fail.Fail]
```

### declared_align

```mach
pub fun declared_align(sc: *sema_context.SemaContext, d: *ast_decl.Decl) u32
```

the alignment `d` declares, zero when it declares none or a refused one, which is
reported here for a type, a function and a global alike

### intrinsic_operand_type

```mach
pub fun intrinsic_operand_type(sc: *sema_context.SemaContext, eid: ast_id.ExprId, span: base_source.Span) type_kind.TypeId
```

### resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *sema_context.SemaContext, tid: ast_id.TypeId) type_kind.TypeId
```

### type_for_symbol

```mach
pub fun type_for_symbol(sc: *sema_context.SemaContext, sym: *resolve.Symbol, span: base_source.Span) type_kind.TypeId
```

### def_underlying

```mach
pub fun def_underlying(sc: *sema_context.SemaContext, sym: *resolve.Symbol, span: base_source.Span) type_kind.TypeId
```

### resolve_type_anon

```mach
pub fun resolve_type_anon(sc: *sema_context.SemaContext, tid: ast_id.TypeId, fields_start: u32, fields_len: u32, kind: type_kind.TypeKind) type_kind.TypeId
```

### ptr_to

```mach
pub fun ptr_to(sc: *sema_context.SemaContext, base: type_kind.TypeId) type_kind.TypeId
```

### prim_or_error

```mach
pub fun prim_or_error(sc: *sema_context.SemaContext, kind: type_kind.TypeKind) type_kind.TypeId
```

### intern_span_or_nil

```mach
pub fun intern_span_or_nil(sc: *sema_context.SemaContext, span: base_source.Span) intern.StrId
```

### allocate_params

```mach
pub fun allocate_params(sc: *sema_context.SemaContext, count: u32) res[*type_kind.TypeId, fail.Fail]
```

### params_free

```mach
pub fun params_free(sc: *sema_context.SemaContext, params: *type_kind.TypeId, count: u32)
```

