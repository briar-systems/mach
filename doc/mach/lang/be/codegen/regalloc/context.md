# mach.lang.be.codegen.regalloc.context

the per-function allocation state every phase reads and writes, its tables
and their lifetimes, and the register facts the phases share

## val REG_CLASS_GP

```mach
pub val REG_CLASS_GP: u32 = 0
```

## val RANGE_NIL

```mach
pub val RANGE_NIL: u32 = 0xFFFFFFFF
```

## val POS_MAX

```mach
pub val POS_MAX:   i64 = 0x7FFFFFFFFFFFFFFF
```

## rec FlatInstr

```mach
pub rec FlatInstr;
```

## rec Segment

```mach
pub rec Segment;
```

one live run of a value: inclusive instruction positions

## rec LiveRange

```mach
pub rec LiveRange;
```

a value's lifetime is its segments, in position order and never touching;
start and end_pos are their hull, live_len the positions they cover, and
use_weight over live_len is the density the eviction choice compares.
crosses_call says a segment meets a call anywhere, call_inside that one
holds a call strictly within it, where a caller-saved register cannot
carry the value

## rec BlockSpan

```mach
pub rec BlockSpan;
```

## rec CopyHint

```mach
pub rec CopyHint;
```

## rec Liveness

```mach
pub rec Liveness;
```

what liveness builds each round: the flat instruction index, each block's
span and loop weight, and every value's lifetime

## rec Scan

```mach
pub rec Scan;
```

the interval scan's working state: the intervals in hand, the register
file's occupancy, the fixed-register reservations and the copy hints

## rec Spill

```mach
pub rec Spill;
```

the per-value spill decisions, kept across rounds and sized by table_count

## rec Rewrite

```mach
pub rec Rewrite;
```

the reload registers the rewrite holds back at tight instructions

## rec Context

```mach
pub rec Context;
```

the facts every phase shares at the top, and each phase's own state below

## fun fail_for

```mach
pub fun fail_for(ctx: *Context, prefix: str, suffix: str, unnamed: str) fail.Fail;
```

the internal failure `prefix`, the function's name, then `suffix`; `unnamed`
is the text when the allocation owns no interner or the function no name

## fun fp_class_index

```mach
pub fun fp_class_index(tgt: *lang_target.Binding) u32;
```

## fun bank_count

```mach
pub fun bank_count(tgt: *lang_target.Binding, class: u32) u32;
```

## fun init

```mach
pub fun init(ctx: *Context, tgt: *lang_target.Binding, m: *lang_mir.MirModule, f: *lang_mir.MirFunction) err[fail.Fail];
```

a failed init leaves what it took for dnit to release

## fun vreg_tables_reserve

```mach
pub fun vreg_tables_reserve(ctx: *Context, need: u32) err[fail.Fail];
```

the per-vreg tables that outlive a round, sized for every value the
function has plus the pieces the rounds add

## fun round_tables_init

```mach
pub fun round_tables_init(ctx: *Context) err[fail.Fail];
```

the tables one round builds and the next rebuilds: sized for the values
the function has now, with every register free and no interval placed

## fun round_tables_dnit

```mach
pub fun round_tables_dnit(ctx: *Context);
```

## fun liveness_dnit

```mach
pub fun liveness_dnit(ctx: *Context);
```

## fun spill_dnit

```mach
pub fun spill_dnit(ctx: *Context);
```

## fun dnit

```mach
pub fun dnit(ctx: *Context);
```

## val MAX_REG_FILE

```mach
pub val MAX_REG_FILE: i32 = 32
```

## fun reserve_reg

```mach
pub fun reserve_reg(ctx: *Context, id: i32);
```

keeps general register `id` from allocation

## fun function_has_reg_shift

```mach
pub fun function_has_reg_shift(ctx: *Context) bool;
```

## fun block_at

```mach
pub fun block_at(ctx: *Context, p: u32) *lang_mir.MirBlock;
```

the block at position p of the order the phases visit, which is the block
array's own order until the scan order is built

## fun order_identity

```mach
pub fun order_identity(order: *u32, n: u32);
```

## fun flat_instr

```mach
pub fun flat_instr(ctx: *Context, pos: u32) *lang_mir.MirInstr;
```

## fun defect_note

```mach
pub fun defect_note(ctx: *Context, text: str);
```

## fun defect_check

```mach
pub fun defect_check(ctx: *Context) err[fail.Fail];
```

## fun build_vreg_slot_flags

```mach
pub fun build_vreg_slot_flags(ctx: *Context) err[fail.Fail];
```

## fun vreg_has_slot

```mach
pub fun vreg_has_slot(ctx: *Context, v: lang_mir.VRegId) bool;
```

## fun spill_vreg

```mach
pub fun spill_vreg(ctx: *Context, v: lang_mir.VRegId) err[fail.Fail];
```

a spill is a decision the rounds may still revisit; the slot itself is
allocated once they settle, in the order the decisions were made

## fun tight_tables_dnit

```mach
pub fun tight_tables_dnit(ctx: *Context);
```

## fun tight_width

```mach
pub fun tight_width(ctx: *Context) u32;
```

the general registers a mask can name

## fun is_spilled

```mach
pub fun is_spilled(ctx: *Context, v: lang_mir.VRegId) bool;
```

## fun is_fp_vreg

```mach
pub fun is_fp_vreg(ctx: *Context, v: lang_mir.VRegId) bool;
```

## fun is_storage_slot

```mach
pub fun is_storage_slot(ctx: *Context, v: lang_mir.VRegId) bool;
```

