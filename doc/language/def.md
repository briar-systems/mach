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
records, unions, tags, or other aliases. `def` is a module-scope declaration;
there is no function-scope alias. An alias of a tag constructs, tests and
copies as the tag:

```mach
use std.types.result.res;

tag ParseError: u8 { invalid; overflow; }
def R: res[i64, ParseError];

fun parse(x: i64) R {
    if (x < 0) { ret R.err{ParseError.invalid{}}; }
    ret R.ok{x};
}
```

## Stdlib aliases

Mach has no compiler-known type aliases. Names like `usize` and `str` live
in stdlib as ordinary `def`s — `def usize: u64;` (or `u32`, target-
conditional via `$if`) and `def str: *u8;`. A module that wants the
shorthand imports the appropriate stdlib module.

## See also

- [types.md](types.md) - the type grammar def references
- [rec.md](rec.md), [uni.md](uni.md), [tag.md](tag.md) - aggregate forms commonly aliased
