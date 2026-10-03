# mach.lang.target.catalog.subsystem

the windows subsystem catalog: every subsystem, its spelling and its
fingerprint tag

## def Subsystem

```mach
pub def Subsystem: u8
```

## val CONSOLE

```mach
pub val CONSOLE: Subsystem = 0
```

## val GUI

```mach
pub val GUI:     Subsystem = 1
```

## val VERSION

```mach
pub val VERSION: u8 = 1
```

## fun from_name

```mach
pub fun from_name(name: str) opt[Subsystem];
```

## fun name_for

```mach
pub fun name_for(s: Subsystem) str;
```

## fun fingerprint_tag

```mach
pub fun fingerprint_tag(s: Subsystem) u8;
```

