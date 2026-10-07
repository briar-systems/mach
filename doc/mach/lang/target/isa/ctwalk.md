# mach.lang.target.isa.ctwalk

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
pub fun emitted_mul_cell(mi: *isa_inst.Inst, e: *isa_effect.InstEffects) ct.CtMulCell;
```

the multiply cell an emitted instruction realizes: its explicit operands
in order, the first one's width

## fun walk

```mach
pub fun walk(f: *lang_mir.MirFunction, ns: *isa_encode.AsmNote, count: u32, arch: *isa.RegMachine,
tgt: *isa.BackendTarget, alloc: *A.Allocator) err[Refusal];
```

validates one oblivious function's notification stream. a non-oblivious
function is accepted without a walk; a stream with no effect description
is refused, never walked as public

