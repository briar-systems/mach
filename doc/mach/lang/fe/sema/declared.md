# mach.lang.fe.sema.declared

the types declarations and type nodes declare: resolving type nodes, typing nominals and
laying them out, and the occurs and secrecy checks over them

## fun seed_builtins

```mach
pub fun seed_builtins(sc: *sema_context.SemaContext) err[fail.Fail];
```

## fun infer_fun_sig

```mach
pub fun infer_fun_sig(sc: *sema_context.SemaContext, f: *ast_decl.DeclFun) type.TypeId;
```

## fun infer_nominal

```mach
pub fun infer_nominal(sc: *sema_context.SemaContext, did: ast_id.DeclId, name_span: lang_source.Span, r: *ast_decl.DeclRec, kind: type.TypeKind) type.TypeId;
```

## fun infer_nominal_uni

```mach
pub fun infer_nominal_uni(sc: *sema_context.SemaContext, did: ast_id.DeclId, name_span: lang_source.Span, u: *ast_decl.DeclUni, kind: type.TypeKind) type.TypeId;
```

## fun elaborate_nominal

```mach
pub fun elaborate_nominal(sc: *sema_context.SemaContext, did: ast_id.DeclId, ty: type.TypeId,
fields_start: u32, fields_len: u32) intern.StrId;
```

## fun ensure_layout_ready

```mach
pub fun ensure_layout_ready(sc: *sema_context.SemaContext, tid: type.TypeId) intern.StrId;
```

elaborates every nominal `tid` holds by value, each once; the name of a nominal whose
layout is still being decided when it is reached again, nil otherwise

## fun prepare_nominal_recipe

```mach
pub fun prepare_nominal_recipe(sc: *sema_context.SemaContext, ty: type.TypeId) err[fail.Fail];
```

## fun resolve_type_query

```mach
pub fun resolve_type_query(sc: *sema_context.SemaContext, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun answer_type_query

```mach
pub fun answer_type_query(s: *session.Session, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun offset_of_member_index

```mach
pub fun offset_of_member_index(sc: *sema_context.SemaContext, c: *ast_expr.ExprCall, owner: type.TypeId, span: lang_source.Span, silent: bool) opt[u32];
```

the member named by `$offset_of(T, m)`: a record field, or a tag case that owns a payload

## fun field_offset

```mach
pub fun field_offset(sc: *sema_context.SemaContext, owner: u32, index: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

a field descriptor's offset, from the checked type layout

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(sc: *sema_context.SemaContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun check_occurs_all

```mach
pub fun check_occurs_all(sc: *sema_context.SemaContext) err[fail.Fail];
```

## fun check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *sema_context.SemaContext) err[fail.Fail];
```

## fun declared_align

```mach
pub fun declared_align(sc: *sema_context.SemaContext, d: *ast_decl.Decl) u32;
```

the alignment `d` declares, zero when it declares none or a refused one, which is
reported here for a type, a function and a global alike

## fun intrinsic_operand_type

```mach
pub fun intrinsic_operand_type(sc: *sema_context.SemaContext, eid: ast_id.ExprId, span: lang_source.Span) type.TypeId;
```

## fun resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *sema_context.SemaContext, tid: ast_id.TypeId) type.TypeId;
```

## fun type_for_symbol

```mach
pub fun type_for_symbol(sc: *sema_context.SemaContext, sym: *resolve.Symbol, span: lang_source.Span) type.TypeId;
```

## fun def_underlying

```mach
pub fun def_underlying(sc: *sema_context.SemaContext, sym: *resolve.Symbol, span: lang_source.Span) type.TypeId;
```

## fun resolve_type_anon

```mach
pub fun resolve_type_anon(sc: *sema_context.SemaContext, tid: ast_id.TypeId, fields_start: u32, fields_len: u32, kind: type.TypeKind) type.TypeId;
```

## fun ptr_to

```mach
pub fun ptr_to(sc: *sema_context.SemaContext, base: type.TypeId) type.TypeId;
```

## fun prim_or_error

```mach
pub fun prim_or_error(sc: *sema_context.SemaContext, kind: type.TypeKind) type.TypeId;
```

## fun intern_span_or_nil

```mach
pub fun intern_span_or_nil(sc: *sema_context.SemaContext, span: lang_source.Span) intern.StrId;
```

## fun allocate_params

```mach
pub fun allocate_params(sc: *sema_context.SemaContext, count: u32) res[*type.TypeId, fail.Fail];
```

## fun free_params

```mach
pub fun free_params(sc: *sema_context.SemaContext, params: *type.TypeId, count: u32);
```

