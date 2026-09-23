# mach.lang.be.codegen.debug_input

## def DwarfRegFn

```mach
pub def DwarfRegFn: arch.DwarfRegFn
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
lanes each `piece_bytes` wide; `pieces` is 0 for a whole value

