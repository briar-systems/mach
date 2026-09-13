# std.collections.vector

## rec Vector

```mach
pub rec Vector[T];
```

Resizable vector backed by an Allocator

a: allocator to use for allocations
data: pointer to the backing array
len: number of elements in the vector
cap: allocated capacity of the backing array

## fun init

```mach
pub fun init[T](a: *A.Allocator) Vector[T];
```

create an empty vector using the provided allocator

an empty vector owns nothing, so the value is relocatable until the first
growth and never fails to construct.

a: allocator to use for allocations
ret: a new empty Vector[T]

## fun dnit

```mach
pub fun dnit[T](vec: *Vector[T]) err[A.Error];
```

free the backing buffer and reset the vector fields

the elements are not visited: an element that owns storage is the caller's
to release before this runs.

vec: vector to deinitialize
ret: ok when the backing buffer was released (or there was none), or the
     allocator's refusal, in which case the vector is left as it was

## fun is_empty

```mach
pub fun is_empty[T](vec: *Vector[T]) bool;
```

check if the vector is empty

ret: true if the vector has no elements

## fun clear

```mach
pub fun clear[T](vec: *Vector[T]);
```

clear the vector contents but keep allocated capacity

vec: vector to clear

## fun reserve

```mach
pub fun reserve[T](vec: *Vector[T], additional: usize) res[usize, A.Error];
```

reserve space for additional elements, growing the backing array if necessary

a refused growth leaves the vector exactly as it was: the old buffer, length
and capacity stay valid and nothing was moved.

vec: vector to reserve space in
additional: number of extra elements to reserve space for
ret: the new capacity, or the allocator's refusal

## fun ensure

```mach
pub fun ensure[T](vec: *Vector[T], cap: usize) res[usize, A.Error];
```

ensure the vector has at least the given capacity, growing if necessary

vec: vector to ensure capacity for
cap: minimum capacity to ensure
ret: the new capacity, or the allocator's refusal

## fun push

```mach
pub fun push[T](vec: *Vector[T], value: T) res[usize, A.Error];
```

append a value to the vector

the value is copied into the vector's storage. a refused push leaves the
vector unchanged and the value still the caller's.

value: value to append
ret: the new length, or the allocator's refusal

## fun pop

```mach
pub fun pop[T](vec: *Vector[T]) opt[T];
```

remove and return the last element

the element is copied out and the slot is released; an element that owns
storage is now the caller's to release.

ret: the value, or none when the vector is empty

## fun get

```mach
pub fun get[T](vec: *Vector[T], index: usize) opt[*T];
```

get a pointer to the element at the given index

the pointer is into the vector's storage and is invalidated by any growth.

index: element index
ret: pointer to the element, or none when the index is out of range

