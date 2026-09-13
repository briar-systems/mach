# mach.lang.me.lower.decl

## fun lower_decl

```mach
pub fun lower_decl(ctx: *context.LowerContext, did: id.DeclId) err[fail.Fail];
```

## fun lower_fun

```mach
pub fun lower_fun(ctx: *context.LowerContext, did: id.DeclId, d: *decl.Decl, is_test: bool) err[fail.Fail];
```

## fun lower_instance

```mach
pub fun lower_instance(ctx: *context.LowerContext, did: id.DeclId, d: *decl.Decl, name: intern.StrId) err[fail.Fail];
```

## fun lower_value_instance

```mach
pub fun lower_value_instance(ctx: *context.LowerContext, did: id.DeclId, d: *decl.Decl, name: intern.StrId) err[fail.Fail];
```

## fun lower_pack_instance

```mach
pub fun lower_pack_instance(ctx: *context.LowerContext, did: id.DeclId, d: *decl.Decl,
name: intern.StrId, types: *type.TypeId, type_len: u32) err[fail.Fail];
```

## fun fun_has_comptime_param

```mach
pub fun fun_has_comptime_param(ctx: *context.LowerContext, d: *decl.Decl) bool;
```

## fun fun_has_pack_param

```mach
pub fun fun_has_pack_param(ctx: *context.LowerContext, d: *decl.Decl) bool;
```

## fun lower_global

```mach
pub fun lower_global(ctx: *context.LowerContext, did: id.DeclId, d: *decl.Decl, is_mut: bool) err[fail.Fail];
```

