# mach.lang.target.isa.x64.encode

## fun encode_x64

```mach
pub fun encode_x64(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *codegen_mir.MirModule) res[codegen_encode.EncoderOutput, fail.Fail];
```

## fun encode_x64_asm

```mach
pub fun encode_x64_asm(alloc: *A.Allocator, tgt: *isa.BackendTarget, m: *codegen_mir.MirModule,
out: *io_writer.Writer) res[codegen_encode.EncoderOutput, fail.Fail];
```

## fun reads_const_operand

```mach
pub fun reads_const_operand(mi: *codegen_mir.MirInstr, index: u32) bool;
```

the scalar float arithmetic and compare read their second source, a
constant there included, as the r/m operand: a constant is one pool
reference inside the instruction (#4184). a compare's second source is its
right operand, or its left where the condition swaps them

## fun asm_returns

```mach
pub fun asm_returns(body: str) bool;
```

## fun asm_writes_sp

```mach
pub fun asm_writes_sp(body: str) bool;
```

## fun asm_clobbers

```mach
pub fun asm_clobbers(body: str, gp_out: *u32, fp_out: *u32);
```

## fun asm_ct_scan

```mach
pub fun asm_ct_scan(body: str, secrets: *ct.AsmSecret, n_secret: u32,
mul: ct.CtMulMask, trust_shift: bool, alloc: *A.Allocator) err[ct.AsmRefusal];
```

