# mach.lang.fe.sema.infer

## fun seed_builtins

```mach
pub fun seed_builtins(sc: *context.SemaContext) err[fail.Fail];
```

## fun infer_decl

```mach
pub fun infer_decl(sc: *context.SemaContext, did: id.DeclId) type.TypeId;
```

## fun prepare_nominal_recipe

```mach
pub fun prepare_nominal_recipe(sc: *context.SemaContext, ty: type.TypeId) err[fail.Fail];
```

## fun resolve_type_query

```mach
pub fun resolve_type_query(sc: *context.SemaContext, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun answer_type_query

```mach
pub fun answer_type_query(s: *session.Session, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun resolve_field_member

```mach
pub fun resolve_field_member(sc: *context.SemaContext, owner: u32, index: u32, pick: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

descriptor members answer from the checked type layout during type checking; the context callback
answers every member that needs no layout and rejects storage questions about a payloadless case

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(sc: *context.SemaContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
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
pub fun check_occurs_all(sc: *context.SemaContext) err[fail.Fail];
```

## val RECURSIVE_SCRATCH_MSG

```mach
pub val RECURSIVE_SCRATCH_MSG: str = "internal: out of memory allocating the recursive-type checker's scratch table
```

## fun check_uni_secrecy_all

```mach
pub fun check_uni_secrecy_all(sc: *context.SemaContext) err[fail.Fail];
```

## fun infer_expr

```mach
pub fun infer_expr(sc: *context.SemaContext, eid: id.ExprId) type.TypeId;
```

## fun is_field_type_operand

```mach
pub fun is_field_type_operand(sc: *context.SemaContext, eid: id.ExprId) bool;
```

## fun field_seq_owner

```mach
pub fun field_seq_owner(sc: *context.SemaContext, seq_eid: id.ExprId, span: token.Span) type.TypeId;
```

## fun case_seq_owner

```mach
pub fun case_seq_owner(sc: *context.SemaContext, seq_eid: id.ExprId, span: token.Span) type.TypeId;
```

`$cases(T)` enumerates the cases of one public tag; a secret shape is refused because outer
secrecy protects the active case, and a generic parameter defers to its instantiation

## fun type_is_generic_param

```mach
pub fun type_is_generic_param(sc: *context.SemaContext, ty: type.TypeId) bool;
```

## fun resolve_type_ref

```mach
pub fun resolve_type_ref(sc: *context.SemaContext, tid: id.TypeId) type.TypeId;
```

## fun type_is_per_iteration

```mach
pub fun type_is_per_iteration(sc: *context.SemaContext, tid: id.TypeId) bool;
```

