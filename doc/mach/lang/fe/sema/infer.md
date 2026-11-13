# mach.lang.fe.sema.infer

typing expressions: each expression's type, the checks on its operands, and the instances
its calls ask for

## fun infer_decl

```mach
pub fun infer_decl(sc: *sema_context.SemaContext, did: ast_id.DeclId) type_kind.TypeId;
```

## fun infer_expr

```mach
pub fun infer_expr(sc: *sema_context.SemaContext, eid: ast_id.ExprId) type_kind.TypeId;
```

## fun is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun timed_of

```mach
pub fun timed_of(sc: *sema_context.SemaContext, c: *comptime_scope.ComptimeCtx, a: *ast.Ast, d: *ast_decl.Decl) u8;
```

the constant-time mode `#[constant_time]` gives declaration `d` of syntax `a`, read under the decorator scope `c`

## fun field_seq_owner

```mach
pub fun field_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: lang_source.Span) type_kind.TypeId;
```

## fun case_seq_owner

```mach
pub fun case_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: lang_source.Span) type_kind.TypeId;
```

`$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer
secrecy protects the active case, and a generic parameter defers to its instantiation

## fun type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *sema_context.SemaContext, ty: type_kind.TypeId) bool;
```

