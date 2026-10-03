# mach.lang.target.members

every member of each target axis, the one place a member is named outside
its own modules: register_all adds these in order, and the catalog check
holds them against the catalog. the dwarf debug model is the backend's, so
register_all takes it from its caller

## val ISA_COUNT

```mach
pub val ISA_COUNT: u32 = 5
```

## val ISAS

```mach
pub val ISAS: [ISA_COUNT]*isa.IsaVTable = [ISA_COUNT]*isa.IsaVTable;
```

## val ABI_COUNT

```mach
pub val ABI_COUNT: u32 = 10
```

## val ABIS

```mach
pub val ABIS: [ABI_COUNT]*abi.AbiVTable = [ABI_COUNT]*abi.AbiVTable;
```

## val FORMAT_COUNT

```mach
pub val FORMAT_COUNT: u32 = 5
```

## val FORMATS

```mach
pub val FORMATS: [FORMAT_COUNT]*target_of.OfVTable = [FORMAT_COUNT]*target_of.OfVTable;
```

## val OS_COUNT

```mach
pub val OS_COUNT: u32 = 4
```

## val OSES

```mach
pub val OSES: [OS_COUNT]*lang_target_os.OsVTable = [OS_COUNT]*lang_target_os.OsVTable;
```

## val DEBUG_COUNT

```mach
pub val DEBUG_COUNT: u32 = 1
```

the debug models the target tree declares itself, beside the caller's dwarf

## val DEBUGS

```mach
pub val DEBUGS: [DEBUG_COUNT]*target_of.DebugVTable = [DEBUG_COUNT]*target_of.DebugVTable;
```

