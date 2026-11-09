# mach.lang.diagnostic.annotation

## rec Related

```mach
pub rec Related;
```

another place a report concerns: the other edge of a cycle, the other claim of a collision

loc: where it is
label: what it is, nil to show the place without one

## rec Edit

```mach
pub rec Edit;
```

one rewrite of a fix: the bytes at `loc` become `replacement`

## rec Fix

```mach
pub rec Fix;
```

a machine-applicable rewrite of the input, applied whole

label: what the rewrite does, as a fix line names it; nil is no fix
edits: the rewrites, none overlapping another

## fun fix_none

```mach
pub fun fix_none() Fix;
```

a fix carrying no rewrite

## fun fix_present

```mach
pub fun fix_present(fx: *Fix) bool;
```

whether the fix carries a rewrite

## fun related_dnit

```mach
pub fun related_dnit(a: *A.Allocator, r: *Related);
```

release a related place's label, allocated through `a`

## fun fix_dnit

```mach
pub fun fix_dnit(a: *A.Allocator, fx: *Fix);
```

release a fix's label, each edit's replacement and the edit storage, allocated through `a`

## fun fix_copy

```mach
pub fun fix_copy(src: *Fix, a: *A.Allocator) res[Fix, A.Error];
```

a copy of `src` owning its label, its edits and their replacements through `a`, released with
`fix_dnit`

