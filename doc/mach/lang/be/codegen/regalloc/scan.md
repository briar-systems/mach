# mach.lang.be.codegen.regalloc.scan

the linear scan over live segments: argument and definition reservations,
hinted and free register choice, and eviction of the least dense value
under pressure

## fun less_dense

```mach
pub fun less_dense(a: *regalloc_context.LiveRange, b: *regalloc_context.LiveRange) bool;
```

a's loop-weighted use density is below b's

## fun run

```mach
pub fun run(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun instr_defines_preg

```mach
pub fun instr_defines_preg(mi: *lang_mir.MirInstr) bool;
```

## fun arg_reg_busy_at

```mach
pub fun arg_reg_busy_at(ctx: *regalloc_context.Context, id: u32, pos: u32) bool;
```

## fun sort_by_start

```mach
pub fun sort_by_start(ctx: *regalloc_context.Context, order: *u32, count: u32) err[fail.Fail];
```

