# mach.lang.target.binding

a resolved target bound to the registry members it was resolved from: the
description every stage reads and the vtables only the backend follows

## rec Binding

```mach
pub rec Binding;
```

the vtables are static data their members declare, so a binding outlives the
registry it was resolved against

## fun reserved_gp_of

```mach
pub fun reserved_gp_of(arch: *isa.RegMachine, os: *lang_target_os.OsVTable, arch_id: u32) u32;
```

the reserved general registers of `arch` under `os`, resolved once with the binding

## fun backend_target

```mach
pub fun backend_target(b: *Binding) isa.BackendTarget;
```

the view a backend reads

