# mach.lang.mir.unit

## rec UnitModule

```mach
pub rec UnitModule;
```

## val UNIT_NONE

```mach
pub val UNIT_NONE: u32 = 0xFFFFFFFF
```

## rec Unit

```mach
pub rec Unit;
```

## fun root_mir

```mach
pub fun root_mir(u: *Unit) *lang_mir.MirModule;
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

