# std.collections.sort

## tag SearchPosition

```mach
pub tag SearchPosition: u8 {
    insertion: usize;
    found:     usize;
}
```

where a binary search landed

both outcomes carry a position, so this is a closed alternative and not a
failure: `insertion` is where the target would be inserted to keep the array
sorted, `found` is where an equal element is. `insertion` is declared first
so that a zero-initialized position reads as absent at index zero rather
than as a match.

insertion: the insertion position for an absent target
found: the index of a matching element

## fun swap

```mach
pub fun swap[T](a: *T, b: *T);
```

exchange the values at two pointers

a: pointer to the first element
b: pointer to the second element

## fun reverse

```mach
pub fun reverse[T](data: *T, len: usize);
```

reverse an array in place

data: pointer to the array
len: number of elements

## fun sort

```mach
pub fun sort[T](data: *T, len: usize, cmp: fun(*T, *T) i64);
```

in-place shell sort with Ciura gap sequence

data: pointer to the array
len: number of elements
cmp: comparator returning negative if a < b, zero if equal, positive if a > b

## fun is_sorted

```mach
pub fun is_sorted[T](data: *T, len: usize, cmp: fun(*T, *T) i64) bool;
```

check if an array is sorted according to the comparator

data: pointer to the array
len: number of elements
cmp: comparator
ret: true if sorted in non-descending order

## fun binary_search

```mach
pub fun binary_search[T](data: *T, len: usize, target: *T, cmp: fun(*T, *T) i64) SearchPosition;
```

search for a target in a sorted array

data: pointer to the sorted array
len: number of elements
target: pointer to the value to find
cmp: comparator
ret: found{index} for a match, insertion{position} otherwise

