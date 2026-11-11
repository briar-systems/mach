# mach.lang.driver.codec

## fun handle_bytes

```mach
pub fun handle_bytes(value: *u8, value_len: u32, need: usize) *u8;
```

the bytes of a handle product, which its encoder wrote `need` bytes long and the query
engine hands back only as published, so the read is total: a product shorter than its
encoding is a broken invariant of the driver, and it stops the compiler

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
pub fun ct_value_encode(s: *wire.Sink, value: *comptime_value.CTValue, origin: module.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision

