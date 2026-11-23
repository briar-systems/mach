# `def`: type alias

`def` introduces a new name for an existing type. The alias and the
underlying type are interchangeable, with no nominal distinction between them:
a value of the alias passes, assigns and returns as the type it names. The one
exception is an alias of a function type marked `#[constant_time]`, which names
a function type of its own (see
[A marked alias of a function type](#a-marked-alias-of-a-function-type)).

## Grammar

```mach fragment
def NAME: TYPE;
#[DECORATOR(...)] def NAME;
```

## Examples

```mach
pub def Age:    i64; # alias for a primitive
pub def BinOp:  fun(i64, i64) i64; # alias for a function type
pub def Anon:   rec { x: i64; y: i64; }; # inline record
pub def Choice: uni { a: i64; b: f64; }; # inline union
```

Aliases may name any type: primitives, pointers, arrays, function types,
records, unions, tags, or other aliases. `def` is a module-scope declaration,
and a `def` inside a function body is a syntax error. An alias of a tag
constructs, tests and copies as the tag:

```mach
use std.types.result.res;

tag ParseError: u8 { invalid; overflow; }
def R: res[i64, ParseError];

fun parse(x: i64) R {
    if (x < 0) { ret R.err{ParseError.invalid{}}; }
    ret R.ok{x};
}
```

## An alias cannot name itself

An alias is another name for the type it names, never a type of its own, so a
definition that reaches its own name, directly or through other aliases, never
ends. Unlike a record, which is a type with a name of its own, so that
`rec Node { next: *Node; }` refers to it, an alias is refused when it names
itself behind a pointer or a function type just as when it names itself bare.
The refusal is `def.cycle`, at the name that closes the cycle, naming each alias
on it:

```
error[def.cycle] `Visit` names itself through its own definition (Visit -> Visit). an alias is another name for the type it names and never a type of its own, so its definition never ends, behind a pointer or not. declare a `rec` to give the type a name it can refer to
```

A type that must refer to itself gets its name from a record:

```mach
rec Walker { visit: fun(*Walker) u32; }
```

## A marked alias of a function type

`#[constant_time("soft")]` or `#[constant_time("hard")]` on an alias of a
function type marks the type with that constant-time mode. The alias then names
the marked type, not the type written after the colon: the one place an alias
is not the type it is written as.

```mach
#[constant_time("soft")]
def Seal: fun(*^u8, u64) ^u64;
```

A value of `Seal` holds only a function declared `#[constant_time]` in that
mode or a stronger one, with the same signature, and a constant-time function
may pass secrets through it, trusted as a direct call of such a function is.
The mode is part of the type as `...` is, so two marked aliases of one
signature and one mode name the same type, and an alias of `Seal` is `Seal`. The
rules, conversions included, are in
[secrecy.md](secrecy.md#function-types-marked-constant-time).

The alias must state a function type, which must not be C-variadic and must not
carry a mode already: `#[constant_time]` on `def Word: u64;`, on an alias of a
C-variadic function type or on an alias of `Seal` is refused as
`decorator.misplaced`.

## A `def` with no type

A `def` that states no type is a declaration whose type something else
supplies: a decorator that defines it. `#[handle(...)]` declares a type the
owning target mints ([types.md](types.md#handles)), and `#[abi_type(...)]` a C
type whose layout the target declares ([types.md](types.md#abi-types)):

```mach fragment
#[abi_type("va_list")]
pub def VaList;
```

Without such a decorator, `def NAME;` names nothing and is refused at the
declaration (`def.no_type`). The refusal is the only diagnostic it produces:
a use of the name is not reported again.

## Stdlib aliases

Every type alias is an ordinary `def`, and the compiler knows none by name.
Names like `usize` and `str` live in stdlib: `std.types.size` declares
`def usize: u64;` (`u32` or `u16` on a target with a narrower pointer, chosen
with `$if` on `$mach.build.pointer_width`), and `std.types.string` declares
`def str: *char;`, where `char` is `def char: u8;` from `std.types.char`. A
module that wants the shorthand imports it, as in `use std.types.size.usize;`.

## See also

- [types.md](types.md) - the type grammar def references
- [rec.md](rec.md), [uni.md](uni.md), [tag.md](tag.md) - aggregate forms commonly aliased
