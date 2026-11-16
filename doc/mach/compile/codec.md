[mach](../README.md) / [compile](README.md) / **codec**

# mach.compile.codec

the driver's query encodings: handle products, fingerprint outputs and comptime values

a handle product carries a pointer to a value the query engine holds as published,
and its read stops the compiler when the product is shorter than its encoding.

Source: [`src/compile/codec.mach`](../../../src/compile/codec.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.wire`](../base/wire.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.front.resolve`](../front/resolve.md), [`mach.session.module`](../session/module.md), [`mach.session.query`](../session/query.md).

| Functions | |
| :--- | :--- |
| [`handle_bytes`](#handle_bytes) | the bytes of a handle product, which its encoder wrote `need` bytes long and the query engine hands back only as published, so the read is total: a product shorter than its encoding is a broken invariant of the driver, and it stops the compiler |
| [`resolve_handle_encode`](#resolve_handle_encode), [`resolve_handle_decode`](#resolve_handle_decode) | |
| [`output`](#output) | a grown fingerprint sink's bytes as a query output owned by the caller; the sink is left empty |
| [`ct_value_encode`](#ct_value_encode) | a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision |

---

## Functions

### handle_bytes

```mach
pub fun handle_bytes(value: *u8, value_len: u32, need: usize) *u8
```

the bytes of a handle product, which its encoder wrote `need` bytes long and the query
engine hands back only as published, so the read is total: a product shorter than its
encoding is a broken invariant of the driver, and it stops the compiler

### resolve_handle_encode

```mach
pub fun resolve_handle_encode(alloc: *std.allocator.Allocator, r: *mach.front.resolve.ResolveResult) res[mach.session.query.QueryOutput, mach.base.fail.Fail]
```

### resolve_handle_decode

```mach
pub fun resolve_handle_decode(e: mach.session.query.QueryView) *mach.front.resolve.ResolveResult
```

### output

```mach
pub fun output(s: *mach.base.wire.Sink) res[mach.session.query.QueryOutput, mach.base.fail.Fail]
```

a grown fingerprint sink's bytes as a query output owned by the caller; the
sink is left empty

### ct_value_encode

```mach
pub fun ct_value_encode(s: *mach.base.wire.Sink, value: *mach.front.comptime.value.CTValue, origin: mach.session.module.StableModuleId,
    definition_revision: mach.session.query.Revision) err[mach.base.fail.Fail]
```

a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision

