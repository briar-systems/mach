# mach.lang.me.ir.body

## val MAX_INSTRUCTIONS

```mach
pub val MAX_INSTRUCTIONS:   u32   = 1024
```

## val MAX_BYTES

```mach
pub val MAX_BYTES:          usize = 262144
```

## val SMALL_INSTRUCTIONS

```mach
pub val SMALL_INSTRUCTIONS: u32   = 25
```

## rec Budget

```mach
pub rec Budget;
```

## fun charge

```mach
pub fun charge(b: *Budget, instructions: u32, bytes: usize) bool;
```

## fun live_instructions

```mach
pub fun live_instructions(fn: *ir.Function) u32;
```

## fun own_constant

```mach
pub fun own_constant(a: *A.Allocator, original: value.Value) res[value.Value, fail.Fail];
```

constant payloads belong to the destination module arena

## fun extract

```mach
pub fun extract(dst: *ir.Module, src: *ir.Module, tgt: *target.Target, scratch: *A.Allocator, recursive: *bool) err[fail.Fail];
```

## rec Slot

```mach
pub rec Slot;
```

one attached body: which declaration of the importer it stands in for,
and that declaration while it does

## rec Available

```mach
pub rec Available;
```

the store owns its storage module and its slot table explicitly and
releases them through `alloc`; a store opened by `available_init` also
owns the module home that allocator comes from

## fun available_init

```mach
pub fun available_init(a: *Available, name: intern.StrId) err[fail.Fail];
```

## fun available_init_over

```mach
pub fun available_init_over(a: *Available, name: intern.StrId, alloc: *A.Allocator) err[fail.Fail];
```

a store over the caller's allocator, which then sees every request the
cross-module clone makes

## fun detach

```mach
pub fun detach(a: *Available, dst: *ir.Module);
```

## fun available_dnit

```mach
pub fun available_dnit(a: *Available, dst: *ir.Module);
```

## fun attach

```mach
pub fun attach(a: *Available, dst: *ir.Module) err[fail.Fail];
```

## fun contains

```mach
pub fun contains(a: *Available, ix: u32) bool;
```

## fun acquire

```mach
pub fun acquire(a: *Available, dst: *ir.Module, provider: *ir.Module, name: intern.StrId, tgt: *target.Target, scratch: *A.Allocator) err[fail.Fail];
```

## fun growth_cost

```mach
pub fun growth_cost(fn: *ir.Function, out: *Budget) bool;
```

