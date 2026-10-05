# mach.lang.me.ir.body

## rec Cost

```mach
pub rec Cost;
```

what a body adds to a host it is copied into: its counted instructions and
the bytes it carries

## fun charge

```mach
pub fun charge(total: *Cost, add: Cost, limit: Cost) bool;
```

whether `add` fits beside `total` within `limit`, charged to `total` when it
does

## rec Offer

```mach
pub rec Offer;
```

the bodies a store offers for inlining: those `eligible` admits, while their
costs fit `limit` together. the policy is the inliner's

## fun live_instructions

```mach
pub fun live_instructions(fn: *me_ir.Function) u32;
```

## fun own_constant

```mach
pub fun own_constant(a: *A.Allocator, original: value.Value) res[value.Value, fail.Fail];
```

constant payloads belong to the destination module arena

## fun extract

```mach
pub fun extract(dst: *me_ir.Module, src: *me_ir.Module, tgt: *resolved.Target, scratch: *A.Allocator, recursive: *bool, offer: *Offer) err[fail.Fail];
```

copies into `dst` every body of `src` that `offer` admits, `recursive` naming
the functions left out whatever their cost

## rec Available

```mach
pub rec Available;
```

the store owns its storage module and its slot table explicitly and
releases them through `alloc`; a store opened by `available_init` also
owns the module home that allocator comes from

## fun available_init

```mach
pub fun available_init(a: *Available, name: intern.StrId, backing: *A.Allocator, offer: Offer) err[fail.Fail];
```

## fun detach

```mach
pub fun detach(a: *Available, dst: *me_ir.Module);
```

## fun available_dnit

```mach
pub fun available_dnit(a: *Available, dst: *me_ir.Module);
```

## fun attach

```mach
pub fun attach(a: *Available, dst: *me_ir.Module) err[fail.Fail];
```

## fun contains

```mach
pub fun contains(a: *Available, ix: u32) bool;
```

## fun acquire

```mach
pub fun acquire(a: *Available, dst: *me_ir.Module, provider: *me_ir.Module, name: intern.StrId, tgt: *resolved.Target, scratch: *A.Allocator) err[fail.Fail];
```

## fun growth_cost

```mach
pub fun growth_cost(fn: *me_ir.Function, limit: Cost, out: *Cost) bool;
```

what inlining `fn` in its own module adds to the host, false past `limit`

