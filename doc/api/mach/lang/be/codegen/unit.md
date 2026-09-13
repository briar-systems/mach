# mach.lang.be.codegen.unit

## def UnitModule

```mach
pub def UnitModule: unit_input.UnitModule
```

## val UNIT_NONE

```mach
pub val UNIT_NONE: u32 = 0xFFFFFFFF
```

## def Unit

```mach
pub def Unit: unit_input.Unit
```

## rec IrSet

```mach
pub rec IrSet;
```

## fun ir_set_empty

```mach
pub fun ir_set_empty() IrSet;
```

## fun root_ir

```mach
pub fun root_ir(u: *Unit) *ir.Module;
```

## fun root_mir

```mach
pub fun root_mir(u: *Unit) *mir.MirModule;
```

## fun defined_index

```mach
pub fun defined_index(m: *ir.Module, name: intern.StrId) u32;
```

## fun defined_global_index

```mach
pub fun defined_global_index(m: *ir.Module, name: intern.StrId) u32;
```

## fun mir_index

```mach
pub fun mir_index(u: *Unit, mi: u32, name: intern.StrId) u32;
```

## fun init

```mach
pub fun init(a: *A.Allocator, n: u32, u: *Unit) err[fail.Fail];
```

## fun dnit

```mach
pub fun dnit(a: *A.Allocator, u: *Unit);
```

