# mach.lang.target.isa.riscv.encode

## fun frame_reserved_top

```mach
pub fun frame_reserved_top(st: *encode.EncodeState, f: *mir.MirFunction) i64;
```

## fun frame_total

```mach
pub fun frame_total(st: *encode.EncodeState, f: *mir.MirFunction) u32;
```

## fun encode_riscv64

```mach
pub fun encode_riscv64(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule) res[encode.EncoderOutput, fail.Fail];
```

## fun encode_riscv64_asm

```mach
pub fun encode_riscv64_asm(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule,
out: *writer.Writer) res[encode.EncoderOutput, fail.Fail];
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

## fun inst_effects

```mach
pub fun inst_effects(mi: *isa.Inst, e: *ct.InstEffects);
```

the effect description of one emitted riscv64 instruction: the closed class
row plus the operand roles of the notification shape (build_inst) and the
implicit effects. x0 is never written; the walk holds it constant

## fun asm_ct_scan

```mach
pub fun asm_ct_scan(body: str, secret_names: *str, n_secret: u32,
trust_mul: bool, trust_shift: bool, alloc: *A.Allocator) err[fail.Fail];
```

