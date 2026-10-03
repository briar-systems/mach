# mach.lang.mir.debug

## def DwarfRegFn

```mach
pub def DwarfRegFn: catalog_arch.DwarfRegFn
```

## rec DebugTarget

```mach
pub rec DebugTarget;
```

## rec DebugProgram

```mach
pub rec DebugProgram;
```

## rec ModuleDebug

```mach
pub rec ModuleDebug;
```

what a whole-module emitter reads to write a module-shaped debug model into its module

## fun no_module_debug

```mach
pub fun no_module_debug() ModuleDebug;
```

## rec LineRow

```mach
pub rec LineRow;
```

## rec InlinePc

```mach
pub rec InlinePc;
```

## rec VarLoc

```mach
pub rec VarLoc;
```

piece: the lane of a legalized value this location describes, of `pieces`
lanes each `piece_bytes` wide but the last, `piece_last_bytes` wide when that
is not 0; `pieces` is 0 for a whole value

## val VARLOC_NONE

```mach
pub val VARLOC_NONE:  u8 = 0
```

## val VARLOC_REG

```mach
pub val VARLOC_REG:   u8 = 1
```

## val VARLOC_FRAME

```mach
pub val VARLOC_FRAME: u8 = 2
```

## val VARLOC_CMP

```mach
pub val VARLOC_CMP:   u8 = 3
```

## val VARLOC_IMM

```mach
pub val VARLOC_IMM:   u8 = 4
```

## val CMPREL_EQ

```mach
pub val CMPREL_EQ: u8 = 0
```

## val CMPREL_NE

```mach
pub val CMPREL_NE: u8 = 1
```

## val CMPREL_LT

```mach
pub val CMPREL_LT: u8 = 2
```

## val CMPREL_LE

```mach
pub val CMPREL_LE: u8 = 3
```

