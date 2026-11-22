# `val` and `var`: bindings

Bindings introduce named values. `val` is immutable and `var` is mutable. Both
require an explicit type, since a binding never takes its type from its
initializer.

## Grammar

```mach fragment
val NAME: TYPE = EXPR;              # immutable, initializer required
var NAME: TYPE;                     # mutable, zero-initialized
var NAME: TYPE = EXPR;              # mutable, explicit initializer
```

## Examples

```mach
val pi: f64 = 3.14159;
val n:  i64 = 42;

var counter: i64 = 0;
var buf:     [256]u8; # default-initialized to zero

fun bump() {
    counter = counter + 1; # var is reassignable
}
```

```mach error a `val` is immutable
val n: i64 = 42;

fun change() {
    n = 43; # ERROR: `n` is a val
}
```

## Immutability

Assigning to a `val` is a compile-time error, `assign.immutable`, for a
module-level `val` and a local one alike. So is writing a field or an element of
one (`v.field = x`, `v[i] = x`): the store lands in the same storage the binding
names. A `val` declared without an initializer is refused with
`binding.val_uninitialized`, apart from the two exemptions below.

A pointer is the exception that proves the rule. `val p: *T` binds the **address**
immutably, not the storage it addresses, so `@p = x`, `p.field = x` and `p[i] = x`
write the pointee and are legal.

The check ends where the address escapes. `?NAME` on a `val` yields a plain `*T`,
as there is deliberately no read-only pointer type, and writing through any pointer
that reaches a `val`'s storage is **undefined behaviour**: a module-level `val`
lives in read-only data, so the store typically faults, while a local one may
silently appear to work. The compiler does not diagnose it.

## Scope

`val` and `var` work at module top level and inside function bodies.

At module top level:

- `pub val NAME` exports the constant.
- `pub var NAME` exports the variable as a writable global.

Inside functions, they are local to the enclosing block.

A local's storage begins at its declaration and ends when control leaves the
block that declares it: by reaching the end of the block, by `brk` or `cnt` out
of it, or by `ret`. A declaration inside a loop makes the local anew on each pass
through it, so nothing it held carries from one pass to the next.

A pointer to a local may be used only while the local's storage lasts. Reading or
writing through one after its block was left is undefined behaviour, which the
compiler does not diagnose. Two locals whose storage never lasts at the same time
may occupy the same bytes of the frame, as an optimised build lays them out, so a
pointer kept past its block may read or write another local.

A module-level `val` of integer, `bool` or float type whose initializer is a
compile-time constant **is** that value at every use site, in its own module and
in every module that imports it, however the reference is spelled (bare,
module-qualified, or through a `use` alias). No load is emitted, and arithmetic
over it folds and strength-reduces like any other literal. Its storage is still
emitted, so `?NAME` has an address. Every module-level initializer must have a
compile-time value (or be an address, below): one that has none, such as a call
or a read of another `var`, is rejected with `global.init_not_constant`, noting
the reason.

A module-level `val` of pointer type whose initializer is a string literal, such as
a `str`, **is** the address of its bytes at every use site. Its module defines the
bytes once, under a symbol of their own beside the `val`, and a reference in any
module is that address, formed where it is used, not a load of the `val`'s storage.
So two references to one string constant compare pointer-equal no matter which
module they are in or how they are spelled. Its storage is still emitted, holding
the same address, so `?NAME` has an address and `@(?NAME)` reads the pointer back.
A `var` holding a string is read like any other `var`.

A module-level `val` or `var` of record, case, array or vector type is laid out
as constant data at compile time. Its initializer is a literal of that type, or
names a constant `val` (this module's or an imported one, bare or through a field,
an element or a window `A[start, n]` of one), and each member is built the same
way, down to leaves that are compile-time scalars, strings, `nil`, a function
named bare, or the address `?place` of a module-level function, `val` or `var`
or of a field or element of one. An address leaf keeps its symbol, so it crosses
modules like any other member. An initializer, or a leaf of one, that has no
compile-time value, such as a call or the address of a place behind a pointer,
is rejected at that leaf with `global.init_not_constant`, noting the reason it
has none. An operator over vectors is no such form: `val SUM: f32x4 = A + B;` is
refused with `comptime.operand_type`, saying that a vector operator is not
evaluated at compile time.

A scalar read through such a constant is a compile-time constant too: a field, a
nested member, or an element of a record or array `val`, this module's or an
imported one. It takes its value from the constant itself, so it may initialize a
scalar `val` and size an array alike. A read past the end of an array, or of the
payload of a case the constant does not hold, is rejected with that reason, and
so is a read of a leaf that has no compile-time value, such as an address. A
case payload is read under its `sel` guard like any other (see
[tag.md](tag.md#payload-places-and-guards)), so a module-level initializer that
reads one is refused with `tag.payload_unguarded`. A comptime read of one, in a
`$if` gate or an array length, takes the same value. A declaring gate, one in
declaration scope whose arms declare something, is decided before types exist and
reads only integer constants (see
[comptime-control.md](comptime-control.md#when-a-declaration-scope-if-is-decided)),
so a read through a record or array constant there is refused with
`gate.const_not_integer`. A gate in a function body, or one whose arms declare
nothing, reads it.
The index of an element, read or addressed as `?A[i]`, is any compile-time
scalar: a literal, a named constant, or a read through another constant.

### Address arithmetic in an initializer

A module-level integer, or an integer leaf of a constant, may hold a number made
of addresses, which the link fixes:

```mach
var start: [64]u8;
var end:   u8;

val BASE: u64 = (?start)::u64;              # the address as an integer
val PAST: u64 = (?start[8])::u64 + 16;      # an address with a constant added
val BACK: u64 = (?start)::u64 - 4;          # or taken away
val SPAN: u32 = ((?end)::u64 - (?start)::u64)::u32; # the distance, in 4 bytes
val TAIL: u64 = (?end)::u64 - (?start)::u64 + 8; # with a constant added or taken away
```

Each address is `(?place)::T` or `(f)::T` for a function `f`, cast to an
integer as wide as a pointer (a cast of an address to another width is refused,
naming both widths), where `?place` is any address a constant may hold
(above). The constant added is any compile-time integer that fits in an `i64`,
written after the address, or before it when added. The whole may be cast to
another integer, which is the width the link fills. That width must be 4 or 8
bytes: a narrower one is refused at the initializer, naming that reason. A number
that does not fit the width is refused by the link. Pointer arithmetic itself
stays refused: `?start + 1` is not one of these forms.

An address with a constant is one absolute relocation of the integer's width,
the constant its addend, so a position-independent image relocates it at load
like any other address. A distance is one relocation that subtracts a symbol,
which the link resolves to a number: no loader relocates it, so a `val` holding
one lies in read-only data. Both symbols must be defined in the image being
linked. A distance names data on wasm, where a function's address is an index
into its table: a function there takes no constant and lies in no distance.


## `ext`: foreign data imports

`ext val` / `ext var` declares a binding whose storage lives in another object,
imported by name. It is the data analogue of [`ext fun`](ext-fun.md), a
forward reference the linker resolves, so it is **storage-less** and carries no
initializer:

```mach
ext var errno:     i32; # imported mutable datum
#[symbol("environ")]
ext var env:       **u8; # renamed import
#[library("foo")]
ext val foo_flags: u32; # library-pinned import
```

- No initializer. `ext val x: T = ...;` is refused with `ext.data_initializer`,
  since the definition, and its value, live in the providing object (mirroring
  `ext fun`'s absent body).
- The symbol name (the target's C spelling of the identifier), the `symbol` and
  `library` decorators, and the static/dynamic linking inputs all work exactly as
  for [`ext fun`](ext-fun.md).
- On a dynamic target the reference is emitted GOT-indirect so the loader binds
  it to the runtime definition. ELF uses a dynamic pointer relocation,
  `GLOB_DAT` on x86-64 and ARM64 or `R_RISCV_64` on RV64. Mach-O binds the
  `__GOT` slot through dyld, and on arm64 the object carries the reference as
  the `GOT_LOAD_PAGE21`/`GOT_LOAD_PAGEOFF12` pair. A cell another object of the
  same image defines is reached through a linker-owned slot instead. An ordinary
  cross-module reference to a `val`/`var` defined elsewhere in the same artifact
  stays directly addressed. Executed dynamic-import resolution is proven on the
  native ELF legs.

## `#[embed(...)]`: the other exemption to "requires an initializer"

A `val` carrying `#[embed("path")]` also carries no initializer: the named
file's content **is** the initializer, read at compile time. It is the second
(and only other) exemption to `val`'s initializer requirement, alongside `ext`
above. Unlike an `ext` binding, it still owns real storage, placed in read-only
data. See [decorators.md](decorators.md#embedstr--compile-time-file-embedding)
for the full rule set.

## No inference

Every binding declares its type. An untyped numeric literal is checked
against the binding's declared type. It does not participate in inferring
that type.

```mach error a binding needs an explicit type
val n: i64 = 42;                    # ok: 42 conforms to i64
val x       = 42;                   # ERROR: a binding declares its type
val y       = 42i64;                # ERROR: a suffix is not an annotation
```

The annotation is required whatever the initializer is: a typed suffix
gives the literal a type, it does not give the binding one. Suffixes earn
their keep where there is no annotation to read from, such as the elements
of a pack tail. See [literals.md](literals.md#typed-suffixes). A missing
annotation is refused with `binding.untyped`.

The one part of an annotation an initializer may supply is an array length:
`[_]T` takes its length from the array literal that initializes the binding, as
in `val names: [_]str = [_]str{"a", "b", "c"};`, or from the byte count of a
string literal, as in `val tag: [_]u8 = "abc";` (a `[3]u8`). See
[types.md](types.md#inferred-length-_t).

## See also

- [literals.md](literals.md): numeric, string and char literal forms
- [types.md](types.md): the type grammar for the annotation
- [decorators.md](decorators.md#embedstr--compile-time-file-embedding): `#[embed]`, the other initializer exemption
