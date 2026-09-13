# std.types.semver

## tag SemverError

```mach
pub tag SemverError: u8 {
    empty;
    syntax: usize;
    alloc:  allocator.Error;
}
```

why a version string did not parse

the first case is the zero default.

empty: the input is nil or empty
syntax: the byte offset of the first byte that violates SemVer 2.0.0 (a
        missing separator or number is reported at the offset where one was
        expected)
alloc: the allocator refused a prerelease or build copy, with its reason

## rec Semver

```mach
pub rec Semver;
```

a semantic version per SemVer 2.0.0

major: major version number
minor: minor version number
patch: patch version number
prerelease: optional prerelease identifier (e.g. "alpha.1")
build: optional build metadata

## fun semver_parse

```mach
pub fun semver_parse(input: str, a: *allocator.Allocator) res[Semver, SemverError];
```

parse a version string into a Semver

input: version string (major.minor.patch[-prerelease][+build])
a: allocator for prerelease/build substring copies
ret: the parsed version, or why it did not parse

## fun semver_is_valid

```mach
pub fun semver_is_valid(v: *Semver) bool;
```

validate that a Semver is well-formed

## fun semver_compare

```mach
pub fun semver_compare(v: *Semver, other: *Semver) i64;
```

compare two semvers for precedence (ignores build metadata)

v: first version
other: version to compare against
ret: <0 if v < other, 0 if equal, >0 if v > other

## fun semver_is_less

```mach
pub fun semver_is_less(v: *Semver, other: *Semver) bool;
```

check if v is less than other

## fun semver_is_greater

```mach
pub fun semver_is_greater(v: *Semver, other: *Semver) bool;
```

check if v is greater than other

## fun semver_equals

```mach
pub fun semver_equals(v: *Semver, other: *Semver) bool;
```

check if two versions are equal (ignoring build metadata)

## fun semver_is_stable

```mach
pub fun semver_is_stable(v: *Semver) bool;
```

check whether v is a stable release

## fun semver_is_prerelease

```mach
pub fun semver_is_prerelease(v: *Semver) bool;
```

check whether v is a prerelease

## fun semver_has_build

```mach
pub fun semver_has_build(v: *Semver) bool;
```

check whether v has build metadata

