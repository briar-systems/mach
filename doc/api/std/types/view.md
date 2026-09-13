# std.types.view

## rec View

```mach
pub rec View;
```

a borrowed view over len chars starting at data

data: pointer to the first character (not owned)
len: number of characters in the view

## fun view

```mach
pub fun view(data: *char, len: usize) View;
```

construct a View over `len` chars starting at `data`

data: pointer to the first character
len: number of characters
ret: the view

## fun view_eq_str

```mach
pub fun view_eq_str(v: View, s: str) bool;
```

compare a view against a null-terminated string for byte equality

v: the view
s: null-terminated string to compare against
ret: true iff `s` has exactly `v.len` bytes and every byte matches

## fun view_index_char

```mach
pub fun view_index_char(v: View, c: char) opt[usize];
```

find the first occurrence of a character within the view

v: the view to search
c: character to find
ret: the index of the first occurrence, or none

## fun view_contains_char

```mach
pub fun view_contains_char(v: View, c: char) bool;
```

check whether the view contains a character

v: the view to search
c: character to find
ret: true if c appears within v

