# mach.lang.me.lower.constnode

a published constant value laid out as the constant bytes of its type: a record's fields at
their offsets, a tag's case code and payload, an array's or a vector's elements at the
element stride, a string leaf as a pointer to its bytes, and an address as a relocation
against the symbol it points into

## fun aggregate

```mach
pub fun aggregate(ctx: *lower_context.LowerContext, store: *comptime_deep.Store, node: u32, ity: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

node `node` of `store` as a constant of the IR type `ity`

## fun terminated_bytes

```mach
pub fun terminated_bytes(bytes: View, ty: ir_type.IrTypeId) value.Value;
```

an interned string as constant data: every byte it holds, NULs included,
then the terminator the interner keeps after them

