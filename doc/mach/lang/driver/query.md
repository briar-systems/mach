# mach.lang.driver.query

## fun resolve_handle_encode

```mach
pub fun resolve_handle_encode(alloc: *A.Allocator, r: *resolve.ResolveResult) res[query.QueryOutput, fail.Fail];
```

## fun resolve_handle_decode

```mach
pub fun resolve_handle_decode(e: query.QueryView) *resolve.ResolveResult;
```

## fun output

```mach
pub fun output(s: *wire.Sink) res[query.QueryOutput, fail.Fail];
```

a grown fingerprint sink's bytes as a query output owned by the caller; the
sink is left empty

## fun ct_value_encode

```mach
pub fun ct_value_encode(s: *wire.Sink, value: *comptime.CTValue, origin: session.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision

## fun public_surface_encode

```mach
pub fun public_surface_encode(s: *wire.Sink, rr: *resolve.ResolveResult, origin: session.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

## fun typed_surface_encode

```mach
pub fun typed_surface_encode(s: *wire.Sink, surface: *sema_context.ModuleSema) err[fail.Fail];
```

## fun typed_surface_decode

```mach
pub fun typed_surface_decode(itn: *intern.Interner, a: *A.Allocator, bytes: *u8, len: u32,
path: intern.StrId, mid: session.ModuleId) res[sema_context.ModuleSema, fail.Fail];
```

