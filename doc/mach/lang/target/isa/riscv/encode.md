# mach.lang.target.isa.riscv.encode

## val HOOKS

```mach
pub val HOOKS: isa_encode.EncodeHooks = isa_encode.EncodeHooks;
```

the hooks the shared encode driver runs this encoder through

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

