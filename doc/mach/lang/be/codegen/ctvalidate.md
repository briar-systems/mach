# mach.lang.be.codegen.ctvalidate

## fun validate

```mach
pub fun validate(tgt: *target.Target, m: *mir.MirModule) err[fail.Fail];
```

## fun derive_secret_vregs

```mach
pub fun derive_secret_vregs(f: *mir.MirFunction, alloc: *A.Allocator) res[*bool, fail.Fail];
```

the per-vreg fixpoint the early walk reaches, seeded from MirVReg.secret:
what a post-allocation consumer compares its physical seeds against. one
bool per vreg, the caller frees vreg_count entries; nil for no vregs.

