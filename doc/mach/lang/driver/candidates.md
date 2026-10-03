# mach.lang.driver.candidates

## rec Release

```mach
pub rec Release;
```

one release: its version (whose pre-release text borrows from `text`), the tag that
names it and the commit it points at, all owned by the list's allocator

## rec CandidateSource

```mach
pub rec CandidateSource;
```

## fun releases_free

```mach
pub fun releases_free(a: *A.Allocator, list: *Vector[Release]);
```

