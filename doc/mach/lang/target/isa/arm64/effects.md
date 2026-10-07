# mach.lang.target.isa.arm64.effects

what each arm64 instruction reads, writes and leaks, for the constant-time checks

## fun asm_ct_class

```mach
pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass;
```

## fun inst_effects

```mach
pub fun inst_effects(mi: *isa_inst.Inst, e: *isa_effect.InstEffects);
```

the effect description of one emitted aarch64 instruction: the closed class
row plus the operand roles of the translated form record (dst, src1, src2,
src3 as to_inst lays them out) and the implicit effects

