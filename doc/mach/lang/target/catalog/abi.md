# mach.lang.target.catalog.abi

the calling convention catalog: every abi id, its spelling and its
fingerprint tag, and how a convention passes a value

## val UNKNOWN

```mach
pub val UNKNOWN: u32 = 0
```

## val SYSV

```mach
pub val SYSV:    u32 = 1
```

## val WIN64

```mach
pub val WIN64:   u32 = 2
```

## val AAPCS64

```mach
pub val AAPCS64: u32 = 3
```

## val LP64

```mach
pub val LP64:    u32 = 4
```

## val LP64F

```mach
pub val LP64F:   u32 = 5
```

## val LP64D

```mach
pub val LP64D:   u32 = 6
```

## val ILP32

```mach
pub val ILP32:  u32 = 7
```

## val ILP32F

```mach
pub val ILP32F: u32 = 8
```

## val ILP32D

```mach
pub val ILP32D: u32 = 9
```

## val SPIRV

```mach
pub val SPIRV:  u32 = 10
```

## val VERSION

```mach
pub val VERSION: u8 = 1
```

## fun id_for

```mach
pub fun id_for(name: str) u32;
```

## fun name_for

```mach
pub fun name_for(id: u32) str;
```

## fun count

```mach
pub fun count() usize;
```

## fun name_at

```mach
pub fun name_at(index: usize) str;
```

## fun fingerprint_tag

```mach
pub fun fingerprint_tag(id: u32) u8;
```

## def Passing

```mach
pub def Passing: u8
```

## val PASSING_CARRIERS

```mach
pub val PASSING_CARRIERS: Passing = 0
```

## val PASSING_VALUES

```mach
pub val PASSING_VALUES:   Passing = 1
```

