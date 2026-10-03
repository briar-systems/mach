# mach.lang.be.codegen.regalloc.rewrite

the rewrite of virtual registers onto physical ones: the reload registers
it will need are guaranteed first, then each instruction is rewritten with
its reloads, stores and two-address form, and the callee-saved registers it
used are recorded

## fun guarantee_reload_registers

```mach
pub fun guarantee_reload_registers(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

a spilled operand is loaded into a reload register: one the target
reserves, else a pool register nothing holds at that instruction. an
instruction whose reloads the reserved ones cannot all carry is tight, and
where too few pool registers are free there, one is freed by spilling what
holds it, the least dense holder first as the scan's eviction chooses and
never an operand of that instruction. a spill adds reloads where the
spilled value is read, so the pass repeats until every instruction is met;
the tables of the last pass are what the rewrite borrows from

## fun asm_clobbers_at

```mach
pub fun asm_clobbers_at(tgt: *lang_target.Binding, ctx: *regalloc_context.Context, pos: u32, gp: *u32, fp: *u32) bool;
```

## fun fp_scratch_callee_mask

```mach
pub fun fp_scratch_callee_mask(tgt: *lang_target.Binding) u32;
```

## fun function_uses_fp_scratch

```mach
pub fun function_uses_fp_scratch(f: *lang_mir.MirFunction) bool;
```

## fun asm_callee_mask

```mach
pub fun asm_callee_mask(tgt: *lang_target.Binding, f: *lang_mir.MirFunction) u32;
```

## fun operand_bank_matches

```mach
pub fun operand_bank_matches(op: *lang_mir.MirOperand) bool;
```

## fun record_callee_mask

```mach
pub fun record_callee_mask(tgt: *lang_target.Binding, ctx: *regalloc_context.Context);
```

## fun run

```mach
pub fun run(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun slot_source_index

```mach
pub fun slot_source_index(tgt: *lang_target.Binding, ctx: *regalloc_context.Context, mi: *lang_mir.MirInstr, has_def: bool) u32;
```

the first spilled source operand, read from its slot; the destination is
never one, and RANGE_NIL when no source is spilled

