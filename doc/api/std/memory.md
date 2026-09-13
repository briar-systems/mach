# std.memory

## fun range_valid

```mach
pub fun range_valid(data: *u8, len: usize) bool;
```

validate a public byte range without forming an exclusive end

## fun range_scaled

```mach
pub fun range_scaled(count: usize, item_size: usize, output: *usize) bool;
```

calculate a byte count without multiplication overflow

## fun ranges_overlap

```mach
pub fun ranges_overlap(first: *u8, first_len: usize, second: *u8,
second_len: usize) bool;
```

conservatively reject malformed ranges and compare inclusive ends

## fun raw_fill

```mach
pub fun raw_fill(p: ptr, value: u8, size: usize);
```

fill a block of raw memory with a byte value

byte-fills a head prologue to word alignment, stores the value splatted
across whole machine words through the aligned body, then byte-fills the
tail. word size is derived from usize, so the path adapts to 32/64-bit.

p: pointer to the start of the memory block
value: byte value to fill with
size: number of bytes to fill

## fun raw_copy

```mach
pub fun raw_copy(dst: ptr, src: ptr, size: usize);
```

copy a block of raw memory from source to destination

when src and dst share the same alignment offset modulo the word size, the
copy runs a byte head to reach alignment, whole-word loads/stores through
the aligned body, then a byte tail. mismatched alignment falls back to a
plain byte loop. word size is derived from usize (32/64-bit adaptive).

dst: pointer to the destination memory block
src: pointer to the source memory block
size: number of bytes to copy

## fun raw_move

```mach
pub fun raw_move(dst: ptr, src: ptr, size: usize);
```

copy a block of raw memory, correctly handling overlapping regions

copies backward when dst > src to avoid corruption from overlap.
for non-overlapping regions, raw_copy is preferred.

kept byte-wise: the backward (dst > src) branch is not trivially safe
word-wise (it needs end-alignment reasoning and inverted head/tail handling
that risks corrupting overlapping bytes), and accelerating only the forward
branch would couple correctness to raw_copy's unspecified forward-only order.
overlap correctness wins over the throughput here.

dst: pointer to the destination memory block
src: pointer to the source memory block
size: number of bytes to copy

## fun raw_zero

```mach
pub fun raw_zero(p: ptr, size: usize);
```

zero out a block of raw memory

p: pointer to the start of the memory block
size: number of bytes to zero out

## fun fill

```mach
pub fun fill[T](p: *T, value: T, count: usize);
```

fill a block of memory with a value for type T

p: pointer to the start of the memory block
value: value to fill with
count: number of elements to fill

## fun copy

```mach
pub fun copy[T](dst: *T, src: *T, count: usize);
```

copy a block of memory from a source to a destination

dst: pointer to the destination memory block
src: pointer to the source memory block
count: number of elements to copy

## fun move

```mach
pub fun move[T](dst: *T, src: *T, count: usize);
```

copy a typed block of memory, correctly handling overlapping regions

delegates to raw_move for the actual byte-level overlapping copy.
for non-overlapping regions, copy[T] is preferred.

dst: pointer to the destination memory block
src: pointer to the source memory block
count: number of elements to copy

## fun zero

```mach
pub fun zero[T](p: *T, count: usize);
```

zero out a block of memory for a number of elements of type T

p: pointer to the start of the memory block
count: number of elements to zero out

## fun raw_equal

```mach
pub fun raw_equal(a: ptr, b: ptr, n: usize) bool;
```

compare two blocks of raw memory byte-wise

when a and b are co-aligned modulo the word size, a byte head reaches
alignment, whole words are compared through the body, then a byte tail;
mismatched alignment falls back to a plain byte loop. word size is derived
from usize (32/64-bit adaptive).

nil contract: n==0 is vacuously true regardless of pointer values.
equal pointers (including nil==nil) are trivially true without dereference.
if exactly one pointer is nil and n>0, the result is false.

a: pointer to the first memory block
b: pointer to the second memory block
n: number of bytes to compare
ret: true if the first n bytes of a and b are identical

## fun equal

```mach
pub fun equal[T](a: *T, b: *T, n: usize) bool;
```

compare n typed elements at a and b for byte-wise equality

delegates to raw_equal; see raw_equal for nil handling.
returns false if the byte count would overflow usize.

a: pointer to the first element
b: pointer to the second element
n: number of elements to compare
ret: true if the first n elements of a and b are byte-identical

