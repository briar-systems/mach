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

## fun backend_target

```mach
pub fun backend_target(tgt: *Target) isa.BackendTarget;
```

