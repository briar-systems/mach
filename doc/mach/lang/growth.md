# mach.lang.growth

the compiler's one growth routine for arrays it keeps as a pointer and a
capacity beside its own count. std's Vector owns an array whole; these are
for the tables whose count and capacity are fields of a larger record,
usually a compact u32.

reserve doubles the capacity, starting at MINIMUM, until it holds what is
needed, and never past what the capacity's type can count. resize sets the
capacity exactly, growing or shrinking. a refusal leaves the array as it
was: the old storage and capacity stay valid and nothing moved. a capacity
that cannot count what is needed, or a byte size that overflows, refuses
with overflow; otherwise the refusal is the allocator's.

## val MINIMUM

```mach
pub val MINIMUM: usize = 8
```

the first capacity reserve gives an empty array

## fun reserve

```mach
pub fun reserve[T](a: *A.Allocator, data: **T, cap: *u32, needed: usize) err[A.Error];
```

grow the array at `data`, of `cap` elements, so it holds at least `needed`

## fun reserve_usize

```mach
pub fun reserve_usize[T](a: *A.Allocator, data: **T, cap: *usize, needed: usize) err[A.Error];
```

reserve for an array whose capacity is a usize

## fun resize

```mach
pub fun resize[T](a: *A.Allocator, data: **T, cap: *u32, count: usize) err[A.Error];
```

give the array at `data`, of `cap` elements, exactly `count`; zero releases it

## fun resize_usize

```mach
pub fun resize_usize[T](a: *A.Allocator, data: **T, cap: *usize, count: usize) err[A.Error];
```

resize for an array whose capacity is a usize

