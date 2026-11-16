[mach](../README.md) / [compile](README.md) / **codec**

# mach.compile.codec

the driver's query encodings: handle products, fingerprint outputs and comptime values

a handle product carries a pointer to a value the query engine holds as published,
and its read stops the compiler when the product is shorter than its encoding.

<sub>source: [`src/compile/codec.mach`](../../../src/compile/codec.mach) · imported by 5 modules</sub>

<sub>`A` is `std.allocator` · `resolve` is [`mach.front.resolve`](../front/resolve.md) · `query` is [`mach.session.query`](../session/query.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `wire` is [`mach.base.wire`](../base/wire.md) · `comptime_value` is [`mach.front.comptime.value`](../front/comptime/value.md) · `module` is [`mach.session.module`](../session/module.md)</sub>

| Functions | |
| :--- | :--- |
| [`handle_bytes`](#handle_bytes) | the bytes of a handle product, which its encoder wrote `need` bytes long and the query engine hands back only as published, so the read is total: a product shorter than its encoding is a broken invariant of the driver, and it stops the compiler |
| [`resolve_handle_encode`](#resolve_handle_encode) · [`resolve_handle_decode`](#resolve_handle_decode) | |
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
pub fun resolve_handle_encode(alloc: *A.Allocator, r: *resolve.ResolveResult) res[query.QueryOutput, fail.Fail]
```

### resolve_handle_decode

```mach
pub fun resolve_handle_decode(e: query.QueryView) *resolve.ResolveResult
```

### output

```mach
pub fun output(s: *wire.Sink) res[query.QueryOutput, fail.Fail]
```

a grown fingerprint sink's bytes as a query output owned by the caller; the
sink is left empty

### ct_value_encode

```mach
pub fun ct_value_encode(s: *wire.Sink, value: *comptime_value.CTValue, origin: module.StableModuleId,
    definition_revision: query.Revision) err[fail.Fail]
```

a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision

