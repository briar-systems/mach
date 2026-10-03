# mach.lang.fe.sema.variadic

whether a function body reaches a C variadic call, directly or through what it calls,
which decides how a pack spreads into one

## fun decl_body_spreads_pack_to_c_variadic

```mach
pub fun decl_body_spreads_pack_to_c_variadic(sc: *sema_context.SemaContext, origin: session.ModuleId, did: ast_id.DeclId) bool;
```

