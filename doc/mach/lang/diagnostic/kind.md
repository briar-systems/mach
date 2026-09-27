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

## val SCALARIZE

```mach
pub val SCALARIZE:             Kind = 10
```

## val EXPECT_UNFULFILLED

```mach
pub val EXPECT_UNFULFILLED:    Kind = 11
```

## rec Spec

```mach
pub rec Spec;
```

key:    the dotted key the kind is printed and selected by
source: the kind is decided by the source alone, whatever the target, goal or
        profile, so an `#[expect]` of it that nothing fulfils is reported

## val COUNT

```mach
pub val COUNT: usize       = 11
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

## fun key_of

```mach
pub fun key_of(k: Kind) str;
```

the key of `k`, nil for NONE or a kind no row declares

## fun named

```mach
pub fun named(key: str) opt[Kind];
```

the kind whose key is exactly `key`

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

## fun set_union

```mach
pub fun set_union(s: *KindSet, other: *KindSet);
```

## rec Selection

```mach
pub rec Selection;
```

what one key or family selects from the table

warnings: the warning rows it covers
errors:   how many error rows it covers
source:   a covered warning row is decided by the source alone

## fun select

```mach
pub fun select(selector: str) Selection;
```

the rows `selector` covers; nothing at all when it is unknown

## fun selection_unknown

```mach
pub fun selection_unknown(s: *Selection) bool;
```

the selection covers no row

## fun selection_empty

```mach
pub fun selection_empty(s: *Selection) bool;
```

the selection covers no warning row

