# mach.lang.driver.candidates

## rec Release

```mach
pub rec Release;
```

one release: its version (whose pre-release text borrows from `text`), the tag that
names it and the commit it points at, all owned by the list's allocator

## def ReleasesFn

```mach
pub def ReleasesFn: fun(ptr, str, *Vector[Release]) err[outcome.Fail]
```

every release reachable at `url`, appended to `out` in no particular order

## def ManifestAtFn

```mach
pub def ManifestAtFn: fun(ptr, str, *Release) res[manifest.Manifest, outcome.Fail]
```

the manifest a release ships; err when it cannot be read or its [project].version
disagrees with the release

## rec CandidateSource

```mach
pub rec CandidateSource;
```

## fun release_free

```mach
pub fun release_free(a: *A.Allocator, r: *Release);
```

## fun releases_free

```mach
pub fun releases_free(a: *A.Allocator, list: *Vector[Release]);
```

