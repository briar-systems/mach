# mach.lang.target.resolved

## rec Target

```mach
pub rec Target;
```

a resolved target borrows every vtable from the registry it was resolved
against; the borrow is live exactly while that registry stays published

## fun live

```mach
pub fun live(tgt: *Target) err[fail.Fail];
```

the borrow check every backend entry runs before following a vtable:
a target resolved from a registry that has since been released is refused

## fun layout_machine

```mach
pub fun layout_machine(tgt: *Target) layout.Machine;
```

## fun dit_mode_guaranteed

```mach
pub fun dit_mode_guaranteed(tgt: *Target) bool;
```

whether the target guarantees the data-independent-timing mode a DIT_MODE
row needs: the os declares it per instruction set (#3508)

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

the cells the instruction set declares only under PSTATE.DIT, whether or not
the os guarantees the mode: a secret multiply in one of them is what makes a
program need DIT at start, and what the refusal names when the os declares
nothing (#3508)

## fun backend_target

```mach
pub fun backend_target(tgt: *Target) isa.BackendTarget;
```

