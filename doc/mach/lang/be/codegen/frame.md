# mach.lang.be.codegen.frame

## val DEFAULT_STACK_ALIGN

```mach
pub val DEFAULT_STACK_ALIGN: u32 = 16
```

## fun run

```mach
pub fun run(tgt: *target.Target, m: *mir.MirModule) err[fail.Fail];
```

## fun omits_frame

```mach
pub fun omits_frame(func: *mir.MirFunction, fp: mir.PRegId, sp: mir.PRegId) bool;
```

## fun body_writes_sp

```mach
pub fun body_writes_sp(m: *isa.RegMachine, func: *mir.MirFunction, sp: mir.PRegId) bool;
```

whether the body may move the stack pointer. the encoders write sp only in
the prologue and epilogue, outgoing arguments live in the fixed frame, and no
instruction set allocates stack dynamically, so the writers are an inline-asm
block that writes sp and an instruction naming sp as a register operand

## fun sp_fixed

```mach
pub fun sp_fixed(frame: *mir.MirFrame) bool;
```

the declared frame property: sp == fp - base_dist for the whole body

## fun slot_offset

```mach
pub fun slot_offset(frame: *mir.MirFrame, v: u32) opt[i64];
```

## fun slot_sp_offset

```mach
pub fun slot_sp_offset(frame: *mir.MirFrame, v: u32) opt[i64];
```

## fun slots_from_sp

```mach
pub fun slots_from_sp(frame: *mir.MirFrame) bool;
```

## fun slot_extent

```mach
pub fun slot_extent(frame: *mir.MirFrame, v: u32, disp: *i64, size: *u32) bool;
```

the byte extent an encoder addresses a slot at: the displacement from the
slot base register (the stack pointer when slots_from_sp, else the frame
pointer) and the slot's size. false when v owns no slot

