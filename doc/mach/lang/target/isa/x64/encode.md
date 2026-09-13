# mach.lang.target.isa.x64.encode

## fun encode_x64

```mach
pub fun encode_x64(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule) res[enc.EncoderOutput, fail.Fail];
```

## fun encode_x64_asm

```mach
pub fun encode_x64_asm(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule,
out: *writer.Writer) res[enc.EncoderOutput, fail.Fail];
```

## fun asm_returns

```mach
pub fun asm_returns(body: str) bool;
```

## fun asm_clobbers

```mach
pub fun asm_clobbers(body: str, gp_out: *u32, fp_out: *u32);
```

## fun asm_ct_class

```mach
pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass;
```

the flags and latency row of an opcode, as the inline-asm scan and the
effect walk read it: the class column of the one description table

## val X64_OPCODE_COUNT

```mach
pub val X64_OPCODE_COUNT: u32 = 135
```

the machine opcodes a notification can carry: every x64.Opcode except the
two pseudo entries, the inline-asm block marker and the data byte, which
reach the stream as a MIR pseudo and an ASM_NOTE_BYTES note respectively

## fun notifiable_opcode

```mach
pub fun notifiable_opcode(op: u16) bool;
```

## fun inst_effects

```mach
pub fun inst_effects(mi: *isa.Inst, e: *ct.InstEffects);
```

the effect description of one emitted x86-64 instruction: the opcode's
row (form, shape, memory, class) projected onto the operands present, in
intel order, plus the implicit effects the form carries. every opcode
notifiable_opcode admits has a row; the two pseudo entries and anything
outside the catalog describe as unknown

## val PROBED_ROW_COUNT

```mach
pub val PROBED_ROW_COUNT: usize = 26
```

## rec ProbedRow

```mach
pub rec ProbedRow;
```

## fun probed_rows

```mach
pub fun probed_rows(out: *ProbedRow) u32;
```

## fun asm_ct_scan

```mach
pub fun asm_ct_scan(body: str, secret_names: *str, n_secret: u32,
trust_mul: bool, trust_shift: bool, alloc: *A.Allocator) err[fail.Fail];
```

