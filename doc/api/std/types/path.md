# std.types.path

## def Path

```mach
pub def Path: str
```

a null-terminated filesystem path

## fun separator

```mach
pub fun separator() u8;
```

platform path separator character, emitted by construction

ret: the preferred separator byte for the target

## fun is_separator

```mach
pub fun is_separator(c: u8) bool;
```

check whether c separates path components on the target

windows accepts both '/' and '\'; posix treats only '/' as a separator.

c: byte to test
ret: true when c is a separator

## fun root

```mach
pub fun root(p: Path) view.View;
```

borrow the indivisible root prefix without changing its spelling
unanchored relative, empty and nil paths have zero length
drive-relative paths expose their drive anchor and every view retains p
separator-only paths expose one separator rather than an empty UNC share

## fun has_separator

```mach
pub fun has_separator(p: Path) bool;
```

check whether p contains any path separator

ret: true when at least one byte of p separates components

## fun is_empty

```mach
pub fun is_empty(p: Path) bool;
```

check whether a path is empty or nil

ret: true if the path is nil or zero-length

## fun is_abs

```mach
pub fun is_abs(p: Path) bool;
```

check whether a path is absolute

a path is absolute when it begins with a separator (a posix or UNC root)
or, on windows, with a drive-letter root 'X:\' / 'X:/'. a bare drive
reference 'C:' or 'C:foo' is drive-relative and is not absolute.

ret: true when p is absolute

## fun is_root

```mach
pub fun is_root(p: Path) bool;
```

check whether a path is a root unit

true when p is only separators, or (on windows) exactly a drive or UNC
root followed by any trailing separators.

ret: true when p is exactly a root unit

## fun seg_count

```mach
pub fun seg_count(p: Path) usize;
```

count the net directory depth of a path

walks p's is_separator-delimited segments: a real segment adds one level, a
'.' segment contributes nothing, and a '..' segment climbs one level, clamped
so the depth never falls below zero. empty runs (leading, trailing, or
repeated separators) contribute nothing. the result is how far p descends
below its anchor - the count of '..' needed to climb from p back to that
anchor - which is why '..' decrements and the floor holds a relative climb at
the root.

p: path to measure
ret: the net segment depth, zero for a nil, empty, root, or fully-climbing path

## fun filename

```mach
pub fun filename(p: Path) str;
```

return the final component of a path

returns a pointer into p (no allocation). returns nil for empty paths
and empty string for paths ending in a separator.

p: path to extract filename from
ret: pointer into p at the filename, or nil

## fun extension

```mach
pub fun extension(p: Path) str;
```

return the file extension without the leading dot

returns a pointer into p (no allocation). returns nil if there is no
extension (no dot, or dot is first char of filename).

p: path to extract extension from
ret: pointer into p at the extension, or nil

## fun stem

```mach
pub fun stem(a: *allocator.Allocator, p: Path) res[Path, allocator.Error];
```

return the filename without its extension

"foo/bar.txt" → "bar", "foo/.hidden" → ".hidden", "foo/bar" → "bar"

a: allocator for the result
p: path to extract stem from
ret: the stem, or the allocator's refusal

## fun clone

```mach
pub fun clone(a: *allocator.Allocator, p: Path) res[Path, allocator.Error];
```

duplicate a path using the provided allocator

a: allocator for the new path
p: path to clone (nil returns nil)
ret: the cloned path, or the allocator's refusal

## fun join

```mach
pub fun join(a: *allocator.Allocator, left: Path, right: Path) res[Path, allocator.Error];
```

join two path segments with the platform separator

a: allocator for the resulting path
left: left path segment
right: right path segment
ret: the joined path, or the allocator's refusal

## fun parent

```mach
pub fun parent(a: *allocator.Allocator, p: Path) res[Path, allocator.Error];
```

return the parent directory of a path

a: allocator for the resulting path
p: path to get parent of
ret: the parent path, or the allocator's refusal

## fun resolve

```mach
pub fun resolve(a: *allocator.Allocator, base: Path, p: Path) res[Path, allocator.Error];
```

resolve p against base: an absolute p is cloned verbatim, a relative p is
joined onto base

a: allocator for the result
base: base directory for a relative p
p: path to resolve
ret: the resolved path, or the allocator's refusal

## fun clean

```mach
pub fun clean(a: *allocator.Allocator, p: Path) res[Path, allocator.Error];
```

lexically normalize a path: collapse `.`, resolve safe `..`, and squash
separator runs, without touching the filesystem (mach#2998, mach-lsp#141)

PURELY LEXICAL, and that is the contract rather than a limitation. No `stat`, no
symlink resolution, no current-directory lookup - two spellings that clean to the
same bytes name the same path *as written*, which is exactly the identity an editor
overlay needs: the compiler composes `<root>/./src/f.mach` from a manifest's
`src = "./src"` while the editor supplies `<root>/src/f.mach`, and the buffer is
missed unless those two compare equal. Resolving symlinks would be a different
(and I/O-bearing) question.

THE ROOT IS PRESERVED, NEVER CLIMBED THROUGH. `root_len` already knows what a root
is on this target - a POSIX `/`, a drive `C:\`, a UNC `\\server\share` - so a `..`
that would escape one is dropped, the way every real path resolver behaves. A
RELATIVE path keeps its leading `..` segments, because there is nothing above it to
cancel them against yet; `../a/../b` cleans to `../b`, not `b`.

SEPARATORS COME OUT NATIVE, and go in either way on windows. `is_separator` accepts
both `/` and `\` there, which is what lets a compiler-produced forward-slash
spelling and an editor's backslash one meet; the output uses `separator()` so the
result is one canonical spelling rather than whichever the caller happened to pass.

EMBEDDED NUL IS NOT REPRESENTABLE. `Path` is a null-terminated `str`, so a path ends
at its first NUL by construction and there is nothing to reject - stated here
because the question is a real one for a byte-oriented normalizer and the answer is
a property of the type rather than of this function.

An empty result is spelled `.`, so the return is always a usable path.

a: allocator for the returned path
p: the path to normalize
ret: the cleaned path, or the allocator's refusal

