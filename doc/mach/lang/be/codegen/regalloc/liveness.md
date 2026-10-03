# mach.lang.be.codegen.regalloc.liveness

instruction positions, live ranges and their segments: a value's lifetime
is the runs where it is live, with holes where it is dead, weighted by the
loop depth of its uses

## fun flatten

```mach
pub fun flatten(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun use_weight_at

```mach
pub fun use_weight_at(ctx: *regalloc_context.Context, pos: u32) i64;
```

## fun build_loop_depth

```mach
pub fun build_loop_depth(ctx: *regalloc_context.Context) err[fail.Fail];
```

a use's weight by its block's span depth over the scan order, which is the
reverse postorder: an edge from a block at or after its target in the scan
closes a loop, and every block the scan places from the target through that
source counts it, so a block laid out between a loop's header and its latch
weighs as inside the loop whether it lies in the loop or not. this is the
allocator's own weight, not the loop nesting mir.flow derives

## fun build_ranges

```mach
pub fun build_ranges(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun instr_reads_vreg

```mach
pub fun instr_reads_vreg(mi: *lang_mir.MirInstr, v: lang_mir.VRegId) bool;
```

## fun is_two_address_op

```mach
pub fun is_two_address_op(op: lang_mir.MirOpcode) bool;
```

## fun defines_by_clobbering

```mach
pub fun defines_by_clobbering(mi: *lang_mir.MirInstr, d: u32, y: u32) bool;
```

whether `mi` defines `d` and, sharing d's register with `y`, would read y
after writing d: a two-address op runs as `d = l; d op= r`, so a right
operand or an address register is read after the write; a commutative op
is swapped onto its right operand instead and reads it first

## fun extend

```mach
pub fun extend(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun covers

```mach
pub fun covers(ctx: *regalloc_context.Context, lr: *regalloc_context.LiveRange, pos: i64) bool;
```

## fun dies_at

```mach
pub fun dies_at(ctx: *regalloc_context.Context, v: u32, pos: i64) bool;
```

the value v holds is not needed after pos: a segment ends there

## fun seg_from_ending_at

```mach
pub fun seg_from_ending_at(ctx: *regalloc_context.Context, v: u32, pos: i64) i64;
```

## fun intersects

```mach
pub fun intersects(ctx: *regalloc_context.Context, a: *regalloc_context.LiveRange, b: *regalloc_context.LiveRange) bool;
```

## fun build_block_spans

```mach
pub fun build_block_spans(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun instr_defines

```mach
pub fun instr_defines(mi: *lang_mir.MirInstr) bool;
```

## fun mark_call_crossing

```mach
pub fun mark_call_crossing(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun seg_interior

```mach
pub fun seg_interior(ctx: *regalloc_context.Context, lr: *regalloc_context.LiveRange, pos: i64) bool;
```

