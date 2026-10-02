# mach.lang.me.lower.decl

## fun lower_decl

```mach
pub fun lower_decl(ctx: *lower_context.LowerContext, did: ast_id.DeclId) err[fail.Fail];
```

## fun lower_fwd

```mach
pub fun lower_fwd(ctx: *lower_context.LowerContext, did: ast_id.DeclId) err[fail.Fail];
```

a `fwd` in a root-project module puts the re-exported declaration on the
library's export surface. the object records the linkage name and the link
exports whichever module defines it, so a re-export reaches a dependency's
definition without that dependency knowing anything about this library

## fun lower_fun

```mach
pub fun lower_fun(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl) err[fail.Fail];
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

## fun fun_has_comptime_param

```mach
pub fun fun_has_comptime_param(ctx: *lower_context.LowerContext, d: *ast_decl.Decl) bool;
```

## fun fun_has_pack_param

```mach
pub fun fun_has_pack_param(ctx: *lower_context.LowerContext, d: *ast_decl.Decl) bool;
```

## fun lower_global

```mach
pub fun lower_global(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, is_mut: bool) err[fail.Fail];
```

