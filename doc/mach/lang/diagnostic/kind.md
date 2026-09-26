# mach.lang.diagnostic.kind

## def Kind

```mach
pub def Kind: u8
```

## def Level

```mach
pub def Level: u8
```

## val LEVEL_WARNING

```mach
pub val LEVEL_WARNING: Level = 0
```

## val LEVEL_ERROR

```mach
pub val LEVEL_ERROR:   Level = 1
```

## val NONE

```mach
pub val NONE: Kind = 0
```

a diagnostic that names no row: an error or note not yet given a kind

## val UNUSED_IMPORT

```mach
pub val UNUSED_IMPORT:         Kind = 1
```

## val DEPRECATED

```mach
pub val DEPRECATED:            Kind = 2
```

## val DOCLINT

```mach
pub val DOCLINT:               Kind = 3
```

## val FWD_INSTANCES

```mach
pub val FWD_INSTANCES:         Kind = 4
```

## val DEBUG_DROPPED

```mach
pub val DEBUG_DROPPED:         Kind = 5
```

## val TARGET_SKIPPED

```mach
pub val TARGET_SKIPPED:        Kind = 6
```

## val NATIVE_FALLBACK

```mach
pub val NATIVE_FALLBACK:       Kind = 7
```

## val NOT_OBLIVIOUS

```mach
pub val NOT_OBLIVIOUS:         Kind = 8
```

## val INEXACT_FLOAT_LITERAL

```mach
pub val INEXACT_FLOAT_LITERAL: Kind = 9
```

## rec Spec

```mach
pub rec Spec;
```

## val COUNT

```mach
pub val COUNT: usize       = 9
```

## fun at

```mach
pub fun at(i: usize) *Spec;
```

the row at `i` in declaration order, nil past the end

## fun spec

```mach
pub fun spec(k: Kind) *Spec;
```

the row of `k`, nil for NONE or a kind no row declares

## fun named

```mach
pub fun named(name: str) opt[Kind];
```

the kind a manifest spells as `name`

## rec KindSet

```mach
pub rec KindSet;
```

a set of kinds, one bit for every value a Kind can hold

## fun set_empty

```mach
pub fun set_empty() KindSet;
```

## fun set_add

```mach
pub fun set_add(s: *KindSet, k: Kind);
```

## fun set_has

```mach
pub fun set_has(s: *KindSet, k: Kind) bool;
```

