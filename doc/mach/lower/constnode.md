[mach](../README.md) / [lower](README.md) / **constnode**

# mach.lower.constnode

a published constant value laid out as the constant bytes of its type: a record's fields at
their offsets, a tag's case code and payload, an array's or a vector's elements at the
element stride, a string leaf as a pointer to its bytes, and an address as a relocation
against the symbol it points into. the bytes become one mirl constant: runs of bytes
between the addresses, each address at its offset

<sub>source: [`src/lower/constnode.mach`](../../../src/lower/constnode.mach) · imported by 2 modules</sub>

<sub>`lower_context` is [`mach.lower.context`](context.md) · `comptime_deep` is [`mach.front.comptime.deep`](../front/comptime/deep.md) · `value` is [`mach.lower.value`](value.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `intern` is [`mach.base.intern`](../base/intern.md)</sub>

| Functions | |
| :--- | :--- |
| [`aggregate`](#aggregate) | node `node` of `store` as a constant of the mirl type `ity`, which a global of `ity` is initialised with |
| [`bytes_constant`](#bytes_constant) | bytes as a constant of their own: a byte array, which a string leaf and an embedded file are |
| [`string_global`](#string_global) | the read-only global holding an interned string's bytes, NULs included, and the terminator the interner keeps after them; its address is the value |

---

## Functions

### aggregate

```mach
pub fun aggregate(ctx: *lower_context.LowerContext, store: *comptime_deep.Store, node: u32, ity: Type) res[value.Value, fail.Fail]
```

node `node` of `store` as a constant of the mirl type `ity`, which a global of `ity` is
initialised with

### bytes_constant

```mach
pub fun bytes_constant(ctx: *lower_context.LowerContext, data: *u8, len: u64) res[value.Value, fail.Fail]
```

bytes as a constant of their own: a byte array, which a string leaf and an embedded file are

### string_global

```mach
pub fun string_global(ctx: *lower_context.LowerContext, text: intern.StrId) res[value.Value, fail.Fail]
```

the read-only global holding an interned string's bytes, NULs included, and the terminator
the interner keeps after them; its address is the value. the bytes are never written, so
every use of one text in a module shares one global

