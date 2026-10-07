# mach.lang.target.isa.x64.effects

what each x64 instruction reads, writes and leaks, for the constant-time checks

## fun asm_ct_class

```mach
pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass;
```

the flags and latency row of an opcode, as the inline-asm scan and the
effect walk read it: the class column of the one description table

## fun inst_effects

```mach
pub fun inst_effects(mi: *isa_inst.Inst, e: *isa_effect.InstEffects);
```

the effect description of one emitted x86-64 instruction: the opcode's
row (form, shape, memory, class) projected onto the operands present, in
intel order, plus the implicit effects the form carries. every opcode
notifiable_opcode admits has a row; the two pseudo entries and anything
outside the catalog describe as unknown

