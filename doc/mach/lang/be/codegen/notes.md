# mach.lang.be.codegen.notes

## val ASM_NOTE_INST

```mach
pub val ASM_NOTE_INST:  u8 = 0
```

## val ASM_NOTE_FUNC

```mach
pub val ASM_NOTE_FUNC:  u8 = 1
```

## val ASM_NOTE_BLOCK

```mach
pub val ASM_NOTE_BLOCK: u8 = 2
```

## val ASM_NOTE_BYTES

```mach
pub val ASM_NOTE_BYTES: u8 = 3
```

## val ASM_NOTE_BARRIER

```mach
pub val ASM_NOTE_BARRIER: u8 = 4
```

a declassify barrier: no bytes, the MirInstr names the register it downgrades

## val ASM_NOTE_BYTES_WIDTH

```mach
pub val ASM_NOTE_BYTES_WIDTH: usize = 4
```

## rec AsmNote

```mach
pub rec AsmNote;
```

## fun note_blank

```mach
pub fun note_blank() AsmNote;
```

## val NOTE_SEED_MAX

```mach
pub val NOTE_SEED_MAX: u32 = 16
```

## rec NoteSeeds

```mach
pub rec NoteSeeds;
```

the secrecy seeds a notification carries, per operand of the MirInstr it
encodes: the origin vreg each register or slot operand was rewritten from
and that vreg's declared secrecy. per operand, not per register: one
register can carry a dying secret source and a born public destination in
the same instruction, so a set keyed by register cannot say which

## fun note_seeds_clear

```mach
pub fun note_seeds_clear(out: *NoteSeeds);
```

## fun note_seeds

```mach
pub fun note_seeds(f: *mir.MirFunction, n: *AsmNote, out: *NoteSeeds);
```

every seed the walk needs at one notification. a note without a MirInstr
(prologue, epilogue, relaxation) carries no seed. operands past
NOTE_SEED_MAX are not described; no MIR instruction has that many.

