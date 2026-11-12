# mach.lang.me.lower.decl

## fun lower_decl

```mach
pub fun lower_decl(ctx: *lower_context.LowerContext, did: ast_id.DeclId) err[fail.Fail];
```

## fun lower_instance

```mach
pub fun lower_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId, defining: lower_context.Defining, copy: opt[Type]) err[fail.Fail];
```

an instance bound as `defining` states, or with `copy` the class copy of it taking its
pointers in the classes `copy` states, under the name `name` each way. a generic and a
comptime-parameter instance are lowered alike: the substitution or the bound values are the
scope's

## fun lower_pack_instance

```mach
pub fun lower_pack_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl,
name: intern.StrId, types: *type_kind.TypeId, type_len: u32, defining: lower_context.Defining, copy: opt[Type]) err[fail.Fail];
```

## fun lower_class_copy

```mach
pub fun lower_class_copy(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId, sig: Type) err[fail.Fail];
```

a copy of the function `did` whose pointer parameters take the classes `sig` states,
local to the module whose call asked for it

