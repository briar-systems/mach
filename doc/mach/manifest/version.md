[mach](../README.md) / [manifest](README.md) / **version**

# mach.manifest.version

semver versions, and the version ranges a manifest states

a range is an intersection of clauses that each name their operator, so it denotes one
interval plus the releases whose pre-releases it names. a parsed value borrows its
pre-release text from the string it was parsed from.

Source: [`src/manifest/version.mach`](../../../src/manifest/version.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Version`](#version) | `rec` | a semver version |
| [`Range`](#range) | `rec` | every clause narrows one interval, so a range is an interval plus the releases whose pre-releases it names |
| [`RangeError`](#rangeerror) | `rec` | why a range does not parse |

| On `Version` | |
| :--- | :--- |
| [`parse_version`](#parse_version) | a full version, as a tag or a manifest names one; build metadata is ignored |
| [`compare`](#compare) | the semver precedence of `a` against `b`: below zero, zero or above zero |

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

a semver version

| Field | Type | |
| :--- | :--- | :--- |
| `major` | `u64` | the major component |
| `minor` | `u64` | the minor component |
| `patch` | `u64` | the patch component |
| `pre` | `*u8` | the pre-release identifiers after the `-`, borrowed from the parsed text, nil for a release |
| `pre_len` | `usize` | the bytes of `pre`, 0 for a release |

### parse_version

```mach
pub fun parse_version(text: str) res[Version, str]
```

a full version, as a tag or a manifest names one; build metadata is ignored

### compare

```mach
pub fun compare(a: *Version, b: *Version) i32
```

the semver precedence of `a` against `b`: below zero, zero or above zero

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

why a range does not parse

| Field | Type | |
| :--- | :--- | :--- |
| `reason` | `str` | what is wrong, a static string |
| `clause` | `u32` | the 1-based clause it concerns |

