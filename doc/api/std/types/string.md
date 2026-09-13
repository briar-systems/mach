# std.types.string

## def str

```mach
pub def str: *char
```

a pointer to a null-terminated sequence of char

## tag StrError

```mach
pub tag StrError: u8 {
    alloc: A.Error;
    bounds;
}
```

why an allocating string constructor did not produce a string

the first case is the zero default, so a zero-initialized `res[str, StrError]`
reads as an allocation refusal.

alloc: the allocator refused, with its reason
bounds: a slice does not lie within its string

## rec OwnedString

```mach
pub rec OwnedString;
```

a string that owns its storage, with the extent needed to release it

a plain owned `str` from this module is released with `str_len + 1` bytes,
because that is what its constructor reserved. a producer whose buffer may
be larger than its final text (an environment value read into a probe, a
path normalized in place) cannot be released by length, so it hands the
caller this record instead. the record holds no pointer to itself and may
be copied into its final storage; exactly one copy is released.

data: the null-terminated text, or nil when nothing is owned
len: the logical length, excluding the terminator
extent: the allocated byte count, at least len + 1 when data is not nil
a: the allocator that owns the extent

## fun owned_adopt

```mach
pub fun owned_adopt(a: *A.Allocator, data: str, len: usize, extent: usize) OwnedString;
```

take ownership of a buffer an allocator already served

`data` must be `extent` bytes served by `a`, null-terminated within its
first `len + 1` bytes. nothing is allocated.

a: the allocator that served data
data: the buffer to own
len: the logical length of the text in data
extent: the byte count a served
ret: the owning record

## fun owned_dup

```mach
pub fun owned_dup(a: *A.Allocator, s: str) res[OwnedString, A.Error];
```

an owned copy of a string whose extent is exactly its length plus one

a: allocator for the copy
s: string to copy (nil copies as empty)
ret: the owning record, or the allocator's refusal

## fun owned_release

```mach
pub fun owned_release(o: *OwnedString) err[A.Error];
```

release an owned string's extent and leave the record owning nothing

releasing a record that owns nothing succeeds. on a refused release the
record is left as it was, still owning the extent.

o: the record to release
ret: ok, or the allocator's refusal

## fun str_len

```mach
pub fun str_len(s: str) usize;
```

calculate the length of a null-terminated string

ret: length of the string (not including null terminator)

## fun str_empty

```mach
pub fun str_empty(s: str) bool;
```

check if the string is empty or nil

ret: true if the string has zero length or is nil

## fun str_equals

```mach
pub fun str_equals(s: str, other: str) bool;
```

compare two null-terminated strings for equality

other: the other string to compare against
ret: true if the strings are equal

## fun str_region_equals

```mach
pub fun str_region_equals(s: str, start: usize, len: usize, other: str) bool;
```

compare a region of a string against a pattern

start: byte offset into s
len: number of bytes to compare
other: null-terminated string to compare against
ret: true if s[start..start+len] equals other exactly

## fun str_compare

```mach
pub fun str_compare(s: str, other: str) i64;
```

compare two null-terminated strings lexically

other: the other string to compare against
ret: <0 if s < other, 0 if equal, >0 if s > other

## fun str_starts_with

```mach
pub fun str_starts_with(s: str, prefix: str) bool;
```

check if the string starts with the given prefix

prefix: the prefix to check
ret: true if the string starts with prefix

## fun str_ends_with

```mach
pub fun str_ends_with(s: str, suffix: str) bool;
```

check if the string ends with the given suffix

suffix: the suffix to check
ret: true if the string ends with suffix

## fun str_index_of

```mach
pub fun str_index_of(s: str, sub: str) opt[usize];
```

find the first index of a substring

sub: the substring to find
ret: the index of the first occurrence, or none

## fun str_index_of_from

```mach
pub fun str_index_of_from(s: str, sub: str, from: usize) opt[usize];
```

find the first index of a substring at or after a starting offset

sub: the substring to find
from: byte offset to begin scanning at
ret: the index of the first occurrence at or after from, or none

## fun str_last_index_of

```mach
pub fun str_last_index_of(s: str, sub: str) opt[usize];
```

find the last index of a substring

sub: the substring to find
ret: the index of the last occurrence, or none

## fun str_contains

```mach
pub fun str_contains(s: str, sub: str) bool;
```

check if the string contains the given substring

sub: the substring to check
ret: true if the string contains sub

## fun str_find

```mach
pub fun str_find(s: str, sub: str) opt[str];
```

find the first occurrence of a substring and return a pointer to it

sub: the substring to find
ret: pointer into s at the first occurrence, or none

## fun str_find_last

```mach
pub fun str_find_last(s: str, sub: str) opt[str];
```

find the last occurrence of a substring and return a pointer to it

sub: the substring to find
ret: pointer into s at the last occurrence, or none

## fun str_index_char

```mach
pub fun str_index_char(s: str, c: char) opt[usize];
```

find the first occurrence of a character in a string

c: character to find
ret: the index of the first occurrence, or none

## fun str_last_index_char

```mach
pub fun str_last_index_char(s: str, c: char) opt[usize];
```

find the last occurrence of a character in a string

c: character to find
ret: the index of the last occurrence, or none

## fun str_contains_char

```mach
pub fun str_contains_char(s: str, c: char) bool;
```

check if a string contains a character

c: character to find
ret: true if c appears in s

## fun str_find_char

```mach
pub fun str_find_char(s: str, c: char) opt[str];
```

find the first occurrence of a character and return a pointer to it

c: character to find
ret: pointer into s at the first occurrence, or none

## fun str_find_last_char

```mach
pub fun str_find_last_char(s: str, c: char) opt[str];
```

find the last occurrence of a character and return a pointer to it

c: character to find
ret: pointer into s at the last occurrence, or none

## fun str_copy

```mach
pub fun str_copy(a: *A.Allocator, s: str) res[str, StrError];
```

create a copy of a string using the given allocator

a: allocator to use for the copy
s: string to copy
ret: the copied string, or the allocator's refusal

## fun str_copy_slice

```mach
pub fun str_copy_slice(a: *A.Allocator, s: str, start: usize, len: usize) res[str, StrError];
```

create a copy of a slice of a string using the given allocator

a: allocator to use for the copy
s: string to copy from
start: starting index of the slice
len: length of the slice
ret: the copied slice, `bounds` when the slice does not lie within s, or
       the allocator's refusal

## fun str_join

```mach
pub fun str_join(a: *A.Allocator, va: ...) res[str, StrError];
```

join any number of strings into one newly allocated string

nil arguments contribute nothing, matching str_len/str_empty. a
zero-argument call returns an allocated empty string. every argument
must be a str, enforced at compile time.

a: allocator for the result
va: strings to concatenate, in order
ret: the joined string, or the allocator's refusal

## fun str_trim

```mach
pub fun str_trim(a: *A.Allocator, s: str) res[str, StrError];
```

trim leading and trailing whitespace from a string, returning a new allocated string

a: allocator to use for the new string
s: string to trim
ret: the trimmed string, or the allocator's refusal

## fun str_trim_right

```mach
pub fun str_trim_right(a: *A.Allocator, s: str) res[str, StrError];
```

trim trailing whitespace from a string, returning a new allocated string

a: allocator to use for the new string
s: string to trim
ret: the trimmed string, or the allocator's refusal

## fun str_trim_left

```mach
pub fun str_trim_left(a: *A.Allocator, s: str) res[str, StrError];
```

trim leading whitespace from a string, returning a new allocated string

a: allocator to use for the new string
s: string to trim
ret: the trimmed string, or the allocator's refusal

## fun str_to_lower

```mach
pub fun str_to_lower(a: *A.Allocator, s: str) res[str, StrError];
```

lowercase a string (ASCII), returning a new allocated string

a: allocator for the result
s: string to lowercase
ret: the lowercased string, or the allocator's refusal

## fun str_to_upper

```mach
pub fun str_to_upper(a: *A.Allocator, s: str) res[str, StrError];
```

uppercase a string (ASCII), returning a new allocated string

a: allocator for the result
s: string to uppercase
ret: the uppercased string, or the allocator's refusal

## fun str_repeat

```mach
pub fun str_repeat(a: *A.Allocator, s: str, n: usize) res[str, StrError];
```

repeat a string n times, returning a new allocated string

a: allocator for the result
s: string to repeat
n: number of repetitions (0 yields an empty string)
ret: the repeated string, or the allocator's refusal (`overflow` when len * n does not fit)

## fun str_replace

```mach
pub fun str_replace(a: *A.Allocator, s: str, old: str, new: str) res[str, StrError];
```

replace every occurrence of a substring, returning a new allocated string

occurrences are found left to right and do not overlap. a nil or empty
`old` yields a plain copy of `s`.

a: allocator for the result
s: string to replace within
old: substring to replace
new: replacement substring
ret: the replaced string, or the allocator's refusal

