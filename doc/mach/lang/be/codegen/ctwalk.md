# mach.lang.be.codegen.ctwalk

## def DescribeFn

```mach
pub def DescribeFn: fun(*isa.Inst, *ct.InstEffects)
```

## def MnemonicFn

```mach
pub def MnemonicFn: fun(u16) opt[str]
```

absent for an opcode the ISA does not spell, which the render names by tag

## def RegNameFn

```mach
pub def RegNameFn:  fun(i32, u8) str
```

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

## fun refusal_free

```mach
pub fun refusal_free(alloc: *A.Allocator, r: *Refusal);
```

## fun walk

```mach
pub fun walk(f: *mir.MirFunction, ns: *notes.AsmNote, count: u32, eff: *IsaEffects,
tgt: *isa.BackendTarget, alloc: *A.Allocator) err[Refusal];
```

validates one oblivious function's notification stream. a non-oblivious
function is accepted without a walk; a stream with no effect description
is refused, never walked as public

