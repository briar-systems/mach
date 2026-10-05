# mach.lang.me.ir.cfg

control-flow surgery every pass shares: cloning a region of blocks, listing
clones into their blocks and pointing their operands at each other, and
moving an edge from one block to another in a terminator or a phi

## rec CloneMap

```mach
pub rec CloneMap;
```

where each source block and instruction went: indexed by the source id, the
clone's id or ir_id.NONE

## fun region_index

```mach
pub fun region_index(blocks: *ir_id.BlockId, len: u32, blk: ir_id.BlockId) u32;
```

the position of `blk` in `blocks`, 0 when it is absent

## fun region_clone

```mach
pub fun region_clone(m: *me_ir.Module, fn: *me_ir.Function, body: *ir_id.BlockId, len: u32, out_clones: *ir_id.BlockId, alloc: *A.Allocator) err[fail.Fail];
```

the `len` blocks at `body` copied within `fn` as new blocks, each clone at the
same position of `out_clones`: their phis, instructions and terminators,
with every edge and value inside the region pointing at the copies; the
clone maps are drawn from `alloc`

## fun clones_list

```mach
pub fun clones_list(m: *me_ir.Module, src_fn: *me_ir.Function, dst_fn: *me_ir.Function, map: *CloneMap, src: ir_id.BlockId) err[fail.Fail];
```

the clones of block `src` of `src_fn`'s phis, instructions and terminator
listed in order in its clone in `dst_fn`

## fun clones_remap

```mach
pub fun clones_remap[T](dst_fn: *me_ir.Function, map: *CloneMap, ctx: *T, other: fun(*T, value.Value) value.Value);
```

every clone's operands in `dst_fn` pointed at the clones: a block operand at
its block's clone, an instruction at its clone, and any other value through
`other`, each left as it is where the map has no clone

## fun terminator_redirect

```mach
pub fun terminator_redirect(fn: *me_ir.Function, blk: ir_id.BlockId, from: ir_id.BlockId, to: ir_id.BlockId);
```

block `blk`'s terminator branching to `to` wherever it branched to `from`

## fun phi_pred_relabel

```mach
pub fun phi_pred_relabel(fn: *me_ir.Function, blk: ir_id.BlockId, from: ir_id.BlockId, to: ir_id.BlockId);
```

block `blk`'s phis taking from `to` the values they took from `from`

