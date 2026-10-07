# mach.lang.fe.sema.generics

the instantiation of a generic the sema context names, and the checks each instance owes
its type annotations

## fun instantiate

```mach
pub fun instantiate(
sc: *sema_context.SemaContext,
sym: *resolve.Symbol,
args: *type.TypeId,
arg_count: u32,
span: lang_source.Span) res[type.TypeId, fail.Fail];
```

## fun check_arity

```mach
pub fun check_arity(
sc: *sema_context.SemaContext,
sym: *resolve.Symbol,
arg_count: u32,
span: lang_source.Span) bool;
```

## fun check_annotation

```mach
pub fun check_annotation(sc: *sema_context.SemaContext, ast_tid: ast_id.TypeId);
```

the checks an instance owes each type annotation it resolves, which no check of the
generic declaration can answer before its parameters are known

