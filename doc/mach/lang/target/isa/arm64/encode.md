# mach.lang.target.isa.arm64.encode

## fun encode_arm64

```mach
pub fun encode_arm64(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule) res[encode.EncoderOutput, fail.Fail];
```

## fun encode_arm64_asm

```mach
pub fun encode_arm64_asm(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *mir.MirModule,
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

the effect description of one emitted aarch64 instruction: the closed class
row plus the operand roles of the translated form record (dst, src1, src2,
src3 as to_inst lays them out) and the implicit effects

## fun asm_ct_scan

```mach
pub fun asm_ct_scan(body: str, secret_names: *str, n_secret: u32,
trust_mul: bool, trust_shift: bool, alloc: *A.Allocator) err[fail.Fail];
```

