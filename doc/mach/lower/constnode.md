[mach](../README.md) / [lower](README.md) / **constnode**

# mach.lower.constnode

a published constant value laid out as the constant bytes of its type: a record's fields at
their offsets, a tag's case code and payload, an array's or a vector's elements at the
element stride, a string leaf as a pointer to its bytes, and an address as a relocation
against the symbol it points into. the bytes become one mirl constant: runs of bytes
between the addresses, each address at its offset, or under logical addressing, which
reinterprets no memory, a constant of the type itself, read member by member from them

Source: [`src/lower/constnode.mach`](../../../src/lower/constnode.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.comptime.deep`](../front/comptime/deep.md), [`mach.lower.context`](context.md), [`mach.lower.value`](value.md).

| Functions | |
| :--- | :--- |
| [`aggregate`](#aggregate) | node `node` of `store` as a constant of the mirl type `ity`, which a global of `ity` is initialised with |
| [`bytes_constant`](#bytes_constant) | bytes as a constant of their own: a byte array, which a string leaf and an embedded file are |
| [`string_global`](#string_global) | the read-only global holding an interned string's bytes, NULs included, and the terminator the interner keeps after them; its address is the value |
| [`string_bytes`](#string_bytes) | the bytes the string `val` linked as `name` points at, the global its module defines beside it under the name `bytes_name` gives, so a reference to the val is the address of the bytes, the same address in every module |

---

## Functions

### aggregate

```mach
pub fun aggregate(ctx: *mach.lower.context.LowerContext, store: *mach.front.comptime.deep.Store, node: mach.front.comptime.deep.Node, ity: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

node `node` of `store` as a constant of the mirl type `ity`, which a global of `ity` is
initialised with

### bytes_constant

```mach
pub fun bytes_constant(ctx: *mach.lower.context.LowerContext, data: *u8, len: u64) res[mach.lower.value.Value, mach.base.fail.Fail]
```

bytes as a constant of their own: a byte array, which a string leaf and an embedded file are

### string_global

```mach
pub fun string_global(ctx: *mach.lower.context.LowerContext, text: mach.base.intern.StrId) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the read-only global holding an interned string's bytes, NULs included, and the terminator
the interner keeps after them; its address is the value. the bytes are never written, so
every use of one text in a module shares one global

### string_bytes

```mach
pub fun string_bytes(ctx: *mach.lower.context.LowerContext, name: mach.base.intern.StrId, text: mach.base.intern.StrId, linkage: Linkage, defines: bool, span: mach.base.source.Span) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the bytes the string `val` linked as `name` points at, the global its module defines beside it under the name
`bytes_name` gives, so a reference to the val is the address of the bytes, the same address in every module

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `name` | the val's linkage name |
| `text` | the string |
| `linkage` | the val's linkage, which the bytes share |
| `defines` | this module defines the val, so it defines the bytes, at `span`; any other declares them |
| **returns** | the address of the bytes |

