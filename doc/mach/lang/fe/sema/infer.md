# mach.lang.fe.sema.infer

## fun seed_builtins

```mach
pub fun seed_builtins(sc: *sema_context.SemaContext) err[fail.Fail];
```

## fun infer_decl

```mach
pub fun infer_decl(sc: *sema_context.SemaContext, did: ast_id.DeclId) type.TypeId;
```

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

## fun resolve_field_member

```mach
pub fun resolve_field_member(sc: *sema_context.SemaContext, owner: u32, index: u32, pick: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

descriptor members answer from the checked type layout during type checking; the context callback
answers every member that needs no layout and rejects storage questions about a payloadless case

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(sc: *sema_context.SemaContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

## val LAYOUT_REPORTED_MSG

```mach
pub val LAYOUT_REPORTED_MSG: str = "this layout intrinsic could not be measured
```

## val REACHES_DEPTH_MSG

```mach
pub val REACHES_DEPTH_MSG: str = "this type nests deeper than the recursive-type checker can walk, so it cannot be proven to have a finite size
```

## fun check_occurs_all

```mach
pub fun check_occurs_all(sc: *sema_context.SemaContext) err[fail.Fail];
```

## val RECURSIVE_SCRATCH_MSG

```mach
pub val RECURSIVE_SCRATCH_MSG: str = "internal: out of memory allocating the recursive-type checker's scratch table
```

## fun check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *sema_context.SemaContext) err[fail.Fail];
```

## fun infer_expr

```mach
pub fun infer_expr(sc: *sema_context.SemaContext, eid: ast_id.ExprId) type.TypeId;
```

## fun is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *sema_context.SemaContext, eid: ast_id.ExprId) bool;
```

## fun refuse_confined_flow

```mach
pub fun refuse_confined_flow(sc: *sema_context.SemaContext, eid: ast_id.ExprId, route: str);
```

refuses `eid` when it is a call to an `op` instruction whose result is confined to the
operands of another `op` in its block, reached here where it would be `route`

## fun field_seq_owner

```mach
pub fun field_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: token.Span) type.TypeId;
```

## fun case_seq_owner

```mach
pub fun case_seq_owner(sc: *sema_context.SemaContext, seq_eid: ast_id.ExprId, span: token.Span) type.TypeId;
```

`$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer
secrecy protects the active case, and a generic parameter defers to its instantiation

## fun type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *sema_context.SemaContext, ty: type.TypeId) bool;
```

## fun resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *sema_context.SemaContext, tid: ast_id.TypeId) type.TypeId;
```

## fun type_is_per_iteration

```mach
pub fun type_is_per_iteration(sc: *sema_context.SemaContext, tid: ast_id.TypeId) bool;
```

