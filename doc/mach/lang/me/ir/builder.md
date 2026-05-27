# mach.lang.me.ir.builder

IR construction helper. Maintains an insertion cursor (current
function + current block) and exposes one method per
[`InstrKind`](instruction.md#instrkind) for emitting instructions.
The lowerer ([`me.lower`](../lower.md)) and the optimisation passes
that re-emit instructions are the only intended callers.

Source is `new/lang/me/ir/builder.mach` (currently empty).

## Types

### `Builder`

```mach
pub rec Builder {
    m:       *ir.Module;
    fn:      *ir.Function;
    block:   ir.BlockId;
}
```

| Field | Type                                  | Description                                                |
|-------|---------------------------------------|------------------------------------------------------------|
| m     | [`*ir.Module`](../ir.md#module)       | Module being built.                                        |
| fn    | [`*ir.Function`](../ir.md#function)   | Function currently being emitted into.                     |
| block | [`ir.BlockId`](id.md#blockid)         | Block instructions land in.                                |

## Functions

### `init`, `set_function`, `set_block`

```mach
pub fun init(m: *ir.Module) Builder
pub fun set_function(b: *Builder, fn: *ir.Function)
pub fun set_block(b: *Builder, blk: ir.BlockId)
```

Cursor moves. `init` returns a builder with `fn = nil` and an
undefined `block`; callers must `set_function` + `set_block` before
emitting.

### `new_block`

```mach
pub fun new_block(b: *Builder) Result[ir.BlockId, str]
```

Allocates a fresh empty block in the current function and returns
its id. Does not move the cursor.

### `emit_*`

```mach
pub fun emit_add(b: *Builder, lhs: value.Value, rhs: value.Value) Result[value.Value, str]
pub fun emit_sub(b: *Builder, lhs: value.Value, rhs: value.Value) Result[value.Value, str]
# ... one per InstrKind ...
pub fun emit_alloca(b: *Builder, ty: ir_type.IrTypeId, count: value.Value) Result[value.Value, str]
pub fun emit_load(b: *Builder, ptr: value.Value, ty: ir_type.IrTypeId) Result[value.Value, str]
pub fun emit_store(b: *Builder, val: value.Value, ptr: value.Value) Result[bool, str]
pub fun emit_call(b: *Builder, callee: value.Value, args: *value.Value, arg_count: u32) Result[value.Value, str]
pub fun emit_br(b: *Builder, target: ir.BlockId) Result[bool, str]
pub fun emit_cbr(b: *Builder, cond: value.Value, then_b: ir.BlockId, else_b: ir.BlockId) Result[bool, str]
pub fun emit_ret(b: *Builder, val: value.Value) Result[bool, str]
pub fun emit_phi(b: *Builder, ty: ir_type.IrTypeId) Result[value.Value, str]
pub fun phi_add_incoming(b: *Builder, phi: value.Value, source: ir.BlockId, val: value.Value) Result[bool, str]
```

Each emitter appends an [`Instruction`](instruction.md#instruction)
to the current block (or to the block's `phis` list for `emit_phi`),
records the predecessor / successor edges for terminators, and
returns a [`Value`](value.md#value) handle for the result (or
`ok(true)` for void-producing emitters).

### `emit_terminator_chain`

The terminator emitters (`emit_br` / `emit_cbr` / `emit_ret` /
`emit_unreachable`) also automatically update
[`Block.preds`](../ir.md#block) on the target blocks. Emitters
refuse to add a second terminator to a block; callers must
`new_block` + `set_block` first.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.instruction`](instruction.md),
[`mach.lang.me.ir.value`](value.md),
[`mach.lang.me.ir.type`](type.md).
