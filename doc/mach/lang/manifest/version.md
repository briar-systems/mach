# mach.lang.manifest.version

## rec Version

```mach
pub rec Version;
```

## val MAX_RANGE_CLAUSES

```mach
pub val MAX_RANGE_CLAUSES: u32 = 16
```

## rec Bound

```mach
pub rec Bound;
```

one closed or open end of the interval a range denotes

## rec Range

```mach
pub rec Range;
```

every clause narrows one interval, so a range is an interval plus the
releases whose pre-releases it names

## rec RangeError

```mach
pub rec RangeError;
```

## def ClauseOp

```mach
pub def ClauseOp: u8
```

## fun parse_version

```mach
pub fun parse_version(text: str) res[Version, str];
```

a full version, as a tag or a manifest names one; build metadata is ignored

## fun compare

```mach
pub fun compare(a: *Version, b: *Version) i32;
```

## fun parse_range

```mach
pub fun parse_range(text: str) res[Range, RangeError];
```

clause is 1-based in an error; a missing component of a partial version is 0

## fun satisfies

```mach
pub fun satisfies(r: *Range, v: *Version) bool;
```

a pre-release satisfies a range only when a clause names a pre-release of that release

