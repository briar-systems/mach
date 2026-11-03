# mach.lang.fe.sema.op

an `#[intrinsic]` function names an instruction of an owner's operation catalog: mirl's
capability op catalog of the target holds masc's rows of each set. a declaration for the
selected target is read against that catalog where a call to it is lowered, and mirl's
verifier holds the instruction to its row; one for any other target is inert

## fun validate_op

```mach
pub fun validate_op(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator);
```

an instruction has no body of its own: each call is the instruction

## fun target_is_selected

```mach
pub fun target_is_selected(sc: *sema_context.SemaContext, tgt: str) bool;
```

whether an `#[intrinsic]` naming the target `tgt` is an instruction of the selected one

