# std.allocator.arena

## rec Chunk

```mach
pub rec Chunk;
```

header for each backing allocation

prev: previous chunk in the linked list
cap: data capacity in bytes (data follows the header)

## rec Arena

```mach
pub rec Arena;
```

bump allocator state with chunked growth

backing: allocator used for chunk allocations
buf: current chunk data pointer (cached for fast-path)
cap: current chunk data capacity (cached)
off: current allocation offset within current chunk
last_off: offset of most recent allocation (for rollback/resize)
last_size: size of most recent allocation (for rollback/resize)
chunk: current chunk header
first_chunk: first chunk (for reset/dnit)
default_cap: minimum chunk capacity for growth

## fun init

```mach
pub fun init(ar: *Arena, backing: *allocator.Allocator, cap: usize) err[allocator.Error];
```

initialize an Arena with a backing allocator and optional initial capacity

ar: Arena to initialize
backing: allocator used to provide backing storage
cap: initial chunk capacity in bytes (0 = lazy allocation on first use)
ret: ok, `invalid` for a nil arena or backing allocator, or the refusal
         of the first chunk

## fun dnit

```mach
pub fun dnit(ar: *Arena) err[allocator.Error];
```

free all chunks and zero the Arena state

every chunk is released even when one release fails; the first failure is
what is reported, so a combined failure stays observable.

ar: Arena to destroy
ret: ok if all backing frees succeeded, or the first negative errno as `release`

## fun make

```mach
pub fun make(a: *allocator.Allocator, ar: *Arena) err[allocator.Error];
```

wire up an Allocator backed by this Arena

a: Allocator to initialize
ar: initialized Arena (must outlive the Allocator)
ret: ok, or `invalid` when either pointer is nil

## fun reset

```mach
pub fun reset(ar: *Arena);
```

reset the Arena to empty, freeing all chunks except the first

existing allocations become invalid after reset.

ar: Arena to reset

