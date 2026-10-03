# mach.lang.target.binding

a resolved target bound to the registry members it was resolved from: the
description every stage reads and the vtables only the backend follows

## rec Binding

```mach
pub rec Binding;
```

a binding borrows every vtable from the registry it was resolved against;
the borrow is live exactly while that registry stays published

## fun live

```mach
pub fun live(b: *Binding) err[fail.Fail];
```

the borrow check every backend entry runs before following a vtable:
a binding resolved from a registry that has since been released is refused

## fun backend_target

```mach
pub fun backend_target(b: *Binding) res[isa.BackendTarget, fail.Fail];
```

the view a backend reads, refused for a binding that is no longer live

