# mach.lang.fe.sema.infer

typing expressions: each expression's type, the checks on its operands, and the instances
its calls ask for

## fun infer_decl

```mach
pub fun infer_decl(sc: *sema_context.SemaContext, did: ast_id.DeclId) type.TypeId;
```

## fun infer_expr

```mach
pub fun infer_expr(sc: *sema_context.SemaContext, eid: ast_id.ExprId) type.TypeId;
```

## fun is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun refuse_confined_flow

```mach
pub fun refuse_confined_flow(sc: *sema_context.SemaContext, eid: ast_id.ExprId, route: str);
```

refuses `eid` when it is a call to an `op` instruction whose result is confined to the
operands of another `op` in its block, reached here where it would be `route`

## fun field_seq_owner

```mach
pub fun field_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: lang_source.Span) type.TypeId;
```

## fun case_seq_owner

```mach
pub fun case_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: lang_source.Span) type.TypeId;
```

`$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer
secrecy protects the active case, and a generic parameter defers to its instantiation

## fun type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *sema_context.SemaContext, ty: type.TypeId) bool;
```

