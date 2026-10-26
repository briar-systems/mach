# mach.provider.git.checkout

materialize a verified object closure into a new Git-compatible checkout

## fun commit

```mach
pub fun commit(p: *pack.Pack, selected: *object.Id) res[object.Id, str];
```

## fun create

```mach
pub fun create(a: *A.Allocator, p: *pack.Pack, selected: *object.Id, destination: str) res[object.Id, str];
```

destination must not exist; failure removes only the directory this call created

