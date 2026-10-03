# mach.lang.target.resolved

the description of a resolved target: plain facts the front end, the
middle end and every backend read, importing nothing from the backend. the
member vtables the description was resolved from stay with the backend's
binding (target.binding)

## def AsmReturnsFn

```mach
pub def AsmReturnsFn: fun(str) bool
```

whether an asm body returns from the function that holds it

## def IntImmFitsFn

```mach
pub def IntImmFitsFn: fun(u64, u32) bool
```

whether one instruction materializes the integer `value`, read at `bits`
(at most 64): the rule the middle end hoists a loop's constants by

## rec Target

```mach
pub rec Target;
```

## fun layout_machine

```mach
pub fun layout_machine(tgt: *Target) layout.Machine;
```

the layout machine of a resolved target. total over what resolve produces,
which always selects an instruction set: a nil target is a caller defect

## fun dit_mode_guaranteed

```mach
pub fun dit_mode_guaranteed(tgt: *Target) bool;
```

whether the target guarantees the data-independent-timing mode a DIT_MODE
row needs: the os declares it per instruction set

## fun ct_mul_admitted

```mach
pub fun ct_mul_admitted(tgt: *Target) ct.CtMulMask;
```

the constant-time multiply cells this target admits: the one decision the
lowering gate, the oblivious validators and `$mach.build.ct_mul` all read

## fun ct_mul_dit_cells

```mach
pub fun ct_mul_dit_cells(tgt: *Target) ct.CtMulMask;
```

the cells the instruction set declares only under its data-independent-timing
mode, whether or not the os guarantees the mode: a secret multiply in one of
them is what makes a program need the mode at start, and what the refusal
names when the os declares nothing

