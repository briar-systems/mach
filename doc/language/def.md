# `def` — type alias

`def` introduces a new name for an existing type. The alias and the
underlying type are interchangeable; there is no nominal distinction.

## Grammar

```mach
def NAME: TYPE;
```

## Examples

```mach
pub def Age:    i64;                            # alias for a primitive
pub def BinOp:  fun(i64, i64) i64;              # alias for a function type
pub def Anon:   rec { x: i64; y: i64; };        # inline record
pub def Choice: uni { a: i64; b: f64; };        # inline union
```

Aliases may name any type: primitives, pointers, arrays, function types,
records, unions, or other aliases.

## Stdlib aliases

Mach has no compiler-known type aliases. Names like `usize` and `str` live
in stdlib as ordinary `def`s — `def usize: u64;` (or `u32`, target-
conditional via `$if`) and `def str: *u8;`. A module that wants the
shorthand imports the appropriate stdlib module.

## See also

- [types.md](types.md) — the type grammar `def` references
- [rec.md](rec.md), [uni.md](uni.md) — record / union forms commonly aliased
