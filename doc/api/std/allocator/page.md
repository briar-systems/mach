# std.allocator.page

## fun make

```mach
pub fun make(a: *allocator.Allocator) err[allocator.Error];
```

initialize a page-backed Allocator

a: Allocator to initialize
ret: ok, or `invalid` when a is nil

