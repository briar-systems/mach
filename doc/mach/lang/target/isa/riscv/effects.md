# mach.lang.target.isa.riscv.effects

what each riscv instruction reads, writes and leaks, for the constant-time checks

## fun asm_ct_class

```mach
pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass;
```

## fun describe

```mach
pub fun describe(mi: *isa_inst.Inst, e: *isa_effect.InstEffects);
```

the effect description of one emitted riscv64 instruction: the closed class
row plus the operand roles of the notification shape (build_inst) and the
implicit effects. x0 is never written; the walk holds it constant

