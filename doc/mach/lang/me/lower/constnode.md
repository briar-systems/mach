# mach.lang.me.lower.constnode

a published constant value laid out as the constant bytes of its type: a record's fields at
their offsets, a tag's case code and payload, an array's or a vector's elements at the
element stride, and a string leaf as a pointer to its bytes

## fun aggregate

```mach
pub fun aggregate(ctx: *lower_context.LowerContext, store: *comptime_deep.Store, node: u32, ity: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

node `node` of `store` as a constant of the IR type `ity`

## fun write_bits

```mach
pub fun write_bits(blob: *u8, base: u32, bits: wide.Wide, size: u32);
```

little-endian bytes of a constant: the low limb first, then the high limb
once the leaf is wider than 8 bytes

