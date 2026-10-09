# mach.lang.listing

a directory listing read by name, for the sites that look for sources,
manifests or dependency slots in it

## fun names

```mach
pub fun names(a: *A.Allocator, dir: str, entries: *Vector[fs.Listed]) res[Vector[str], fail.Fail];
```

the UTF-8 names of `entries` in its order, borrowed from the listing, which
keeps them. an entry with no UTF-8 spelling, which only a windows directory
can hold, is never a name these sites look for, so it refuses the listing
with a failure naming `dir` rather than being passed over

a: allocator for the vector of names
dir: the directory listed, as the failure names it
entries: the read_dir listing
ret: the names, released with collections_vector.dnit, or the refusal

## fun unspellable

```mach
pub fun unspellable(a: *A.Allocator, dir: str) fail.Fail;
```

the failure for a directory holding an entry with no UTF-8 spelling

