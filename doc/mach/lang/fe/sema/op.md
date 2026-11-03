# mach.lang.fe.sema.op

an `#[intrinsic]` function names an instruction of an owner's operation catalog: mirl's
capability op catalog and masc's SPIR-V rows state them. mach holds no table of
its own, so an op of the selected target is refused until mach reads theirs

## fun validate_op

```mach
pub fun validate_op(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator);
```

