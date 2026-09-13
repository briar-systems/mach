# std.allocator

## tag Error

```mach
pub tag Error: u8 {
    exhausted;
    overflow;
    invalid;
    release: i64;
}
```

why an allocation request or a release did not complete

the first case is the zero default, so a zero-initialized `res[_, Error]`
reads as a refused request.

exhausted: the backend refused the request, out of memory or over its bound
overflow: the byte count of the request does not fit usize
invalid: a nil allocator, an alignment that is zero or not a power of two,
           or another call that violates the interface contract
release: the backend reported a negative errno while releasing storage

## rec Allocator

```mach
pub rec Allocator;
```

backend-agnostic allocator interface

ctx: opaque context passed to callbacks.
fn_allocate: allocate (ctx, size, align) -> some(ptr), or none when refused.
fn_reallocate: reallocate (ctx, p, old_size, new_size, align) -> some(ptr), or none when refused.
fn_deallocate: deallocate (ctx, p, size, align) -> i64 (0 on success, negative errno on failure).

## fun allocate_raw

```mach
pub fun allocate_raw(a: *Allocator, size: usize, align: usize) res[ptr, Error];
```

allocate raw bytes using this allocator

a: allocator to use for this allocation.
size: number of bytes (0 returns nil).
align: requested alignment in bytes, a power of two.
ret: ptr to allocated memory, or the refusal

## fun reallocate_raw

```mach
pub fun reallocate_raw(a: *Allocator, p: ptr, old_size: usize, new_size: usize, align: usize) res[ptr, Error];
```

reallocate an allocation in-place if possible, otherwise may return a new pointer

a: allocator to use for this operation
p: existing pointer (or nil to allocate new)
old_size: previous size in bytes
new_size: desired size in bytes (0 frees the allocation)
align: alignment in bytes, a power of two
ret: ptr to reallocated memory (same as p if in-place), or the refusal

## fun deallocate_raw

```mach
pub fun deallocate_raw(a: *Allocator, p: ptr, size: usize, align: usize) err[Error];
```

deallocate an allocation previously returned by allocate_raw or reallocate_raw

a nil pointer is a no-op that succeeds. the backend's negative status is
carried as `release`, so a refused release keeps its native cause.

a: Allocator to use for this operation
p: pointer previously returned by allocate_raw or reallocate_raw
size: size in bytes that was allocated
align: alignment in bytes
ret: ok, `invalid` for a nil allocator, or the backend's status as `release`

## fun allocate

```mach
pub fun allocate[T](a: *Allocator, count: usize) res[*T, Error];
```

allocate count elements of type T

a: Allocator to use for this allocation
count: number of elements to allocate
ret: pointer to allocated memory (nil for count == 0), or the refusal

## fun zallocate

```mach
pub fun zallocate[T](a: *Allocator, count: usize) res[*T, Error];
```

allocate and zero-initialize count elements of T

a: Allocator to use for this allocation
count: number of elements to allocate
ret: pointer to allocated and zero-initialized memory (nil for count == 0), or the refusal

## fun reallocate

```mach
pub fun reallocate[T](a: *Allocator, p: *T, old_count: usize, new_count: usize) res[*T, Error];
```

reallocate an allocation of T elements

a: Allocator to use for this operation
p: existing pointer
old_count: previous number of elements
new_count: desired number of elements (0 frees)
ret: pointer to reallocated memory (same as p if in-place, nil for new_count == 0), or the refusal

## fun deallocate

```mach
pub fun deallocate[T](a: *Allocator, p: *T, count: usize) err[Error];
```

deallocate memory previously returned by allocate/zallocate for count
elements.

a: Allocator to use for this operation
p: pointer returned by allocate/zallocate (may be nil)
count: number of elements to deallocate
ret: ok on success (including the nil-pointer / count == 0 no-op), or the
       deallocator's negative status as `release`

