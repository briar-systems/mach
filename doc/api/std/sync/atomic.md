# std.sync.atomic

## fun load

```mach
pub fun load(ptr: *i64) i64;
```

atomic 64-bit load with sequential consistency

ptr: address to load from
ret: loaded value

## fun store

```mach
pub fun store(ptr: *i64, v: i64);
```

atomic 64-bit store with sequential consistency

ptr: address to store to
v: value to store

## fun cas

```mach
pub fun cas(ptr: *i64, expected: *i64, desired: i64) bool;
```

atomic compare-and-swap
if *ptr == *expected, stores desired and returns true.
otherwise, writes actual value to *expected and returns false.

ptr: address of the atomic variable
expected: pointer to expected value (updated on failure)
desired: value to store on success
ret: true if the swap succeeded

## fun fetch_add

```mach
pub fun fetch_add(ptr: *i64, v: i64) i64;
```

atomic fetch-and-add. returns the old value

ptr: address of the atomic variable
v: value to add
ret: previous value

## fun fetch_sub

```mach
pub fun fetch_sub(ptr: *i64, v: i64) i64;
```

atomic fetch-and-subtract. returns the old value

ptr: address of the atomic variable
v: value to subtract
ret: previous value

## fun exchange

```mach
pub fun exchange(ptr: *i64, v: i64) i64;
```

atomic exchange. returns the old value

ptr: address of the atomic variable
v: new value
ret: previous value

## fun fence

```mach
pub fun fence();
```

full memory barrier

## fun spin_hint

```mach
pub fun spin_hint();
```

cpu yield hint for spin loops

