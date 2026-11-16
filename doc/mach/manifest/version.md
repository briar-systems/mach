[mach](../README.md) / [manifest](README.md) / **version**

# mach.manifest.version

semver versions, and the version ranges a manifest states

a range is an intersection of clauses that each name their operator, so it denotes one
interval plus the releases whose pre-releases it names. a parsed value borrows its
pre-release text from the string it was parsed from.

<sub>source: [`src/manifest/version.mach`](../../../src/manifest/version.mach) · imported by 10 modules</sub>

<sub>`Bound` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Version`](#version) | `rec` |  |
| [`Range`](#range) | `rec` | every clause narrows one interval, so a range is an interval plus the releases whose pre-releases it names |
| [`RangeError`](#rangeerror) | `rec` |  |

| On `Version` | |
| :--- | :--- |
| [`parse_version`](#parse_version) | a full version, as a tag or a manifest names one; build metadata is ignored |
| [`compare`](#compare) | |

| On `Range` | |
| :--- | :--- |
| [`parse_range`](#parse_range) | clause is 1-based in an error; a missing component of a partial version is 0 |
| [`satisfies`](#satisfies) | a pre-release satisfies a range only when a clause names a pre-release of that release |

---

## Version

```mach
pub rec Version {
    major:   u64;
    minor:   u64;
    patch:   u64;
    pre:     *u8;
    pre_len: usize;
}
```

### parse_version

```mach
pub fun parse_version(text: str) res[Version, str]
```

a full version, as a tag or a manifest names one; build metadata is ignored

### compare

```mach
pub fun compare(a: *Version, b: *Version) i32
```

## Range

```mach
pub rec Range {
    lower:     Bound;
    upper:     Bound;
    empty:     bool;
    pre_named: [MAX_RANGE_CLAUSES]Version;
    pre_count: u32;
}
```

every clause narrows one interval, so a range is an interval plus the
releases whose pre-releases it names

### parse_range

```mach
pub fun parse_range(text: str) res[Range, RangeError]
```

clause is 1-based in an error; a missing component of a partial version is 0

### satisfies

```mach
pub fun satisfies(r: *Range, v: *Version) bool
```

a pre-release satisfies a range only when a clause names a pre-release of that release

## RangeError

```mach
pub rec RangeError {
    reason: str;
    clause: u32;
}
```

