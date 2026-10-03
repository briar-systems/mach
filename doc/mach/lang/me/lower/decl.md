# mach.lang.me.lower.decl

## fun lower_decl

```mach
pub fun lower_decl(ctx: *lower_context.LowerContext, did: ast_id.DeclId) err[fail.Fail];
```

## fun lower_instance

```mach
pub fun lower_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId) err[fail.Fail];
```

## fun lower_value_instance

```mach
pub fun lower_value_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId) err[fail.Fail];
```

## fun lower_pack_instance

```mach
pub fun lower_pack_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl,
name: intern.StrId, types: *type.TypeId, type_len: u32) err[fail.Fail];
```

