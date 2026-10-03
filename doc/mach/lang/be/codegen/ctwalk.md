# mach.lang.be.codegen.ctwalk

## rec IsaEffects

```mach
pub rec IsaEffects;
```

the per-ISA half of the effect description, declared beside the encoder

## fun isa_effects_none

```mach
pub fun isa_effects_none() IsaEffects;
```

## rec Refusal

```mach
pub rec Refusal;
```

a refusal names the notification it stopped at; the detail is allocated
with the walk's allocator and released by refusal_free
kind: the diagnostic kind the refusal is reported as

## fun refusal_free

```mach
pub fun refusal_free(alloc: *A.Allocator, r: *Refusal);
```

## fun emitted_mul_cell

```mach
pub fun emitted_mul_cell(mi: *isa.Inst, e: *ct.InstEffects) ct.CtMulCell;
```

the multiply cell an emitted instruction realizes: its explicit operands
in order, the first one's width

## fun walk

```mach
pub fun walk(f: *codegen_mir.MirFunction, ns: *notes.AsmNote, count: u32, eff: *IsaEffects,
tgt: *isa.BackendTarget, alloc: *A.Allocator) err[Refusal];
```

validates one oblivious function's notification stream. a non-oblivious
function is accepted without a walk; a stream with no effect description
is refused, never walked as public

