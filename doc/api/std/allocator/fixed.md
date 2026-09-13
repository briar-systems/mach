# std.allocator.fixed

## rec FixedState

```mach
pub rec FixedState;
```

fixed-buffer bump allocator state

buffer: caller-provided backing buffer
capacity: capacity of buf in bytes
offset: bytes consumed so far (including alignment padding)

## fun make

```mach
pub fun make(a: *A.Allocator, state: *FixedState, buf: *u8, cap: usize) err[A.Error];
```

initialize a fixed-buffer Allocator over a caller-provided buffer

the buffer and state must outlive the Allocator. allocations are drawn from
buf until it is exhausted, after which allocation fails rather than growing.

a: Allocator to initialize
state: FixedState to initialize (must outlive the Allocator)
buf: backing buffer the allocator bumps within
cap: capacity of buf in bytes
ret: ok, or `invalid` when any pointer is nil

## fun reset

```mach
pub fun reset(state: *FixedState);
```

reclaim every allocation at once by rewinding to the start of the buffer

state: FixedState to reset

## fun remaining

```mach
pub fun remaining(state: *FixedState) usize;
```

unused capacity remaining in the buffer

state: FixedState to query
ret: bytes left before allocation fails (before any alignment padding)

