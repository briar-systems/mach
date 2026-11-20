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

A module-level `val` of integer, `bool` or float type whose initializer is a
compile-time constant **is** that value at every use site, in its own module and
in every module that imports it, however the reference is spelled (bare,
module-qualified, or through a `use` alias). No load is emitted, and arithmetic
over it folds and strength-reduces like any other literal. Its storage is still
emitted, so `?NAME` has an address. Every module-level initializer must have a
compile-time value (or be an address, below): one that has none, such as a call
or a read of another `var`, is rejected with `global.init_not_constant`, noting
the reason.

A constant whose value **is** an address, such as a `str`, whose value is a
pointer to its bytes, is never duplicated this way. It keeps one definition and every
reference reads it, so two references to one `str` constant compare pointer-equal
no matter which module they are in or how they are spelled.

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
has none.

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
in `val names: [_]str = [_]str{"a", "b", "c"};`. See
[types.md](types.md#inferred-length-_t).

## See also

- [literals.md](literals.md): numeric, string and char literal forms
- [types.md](types.md): the type grammar for the annotation
- [decorators.md](decorators.md#embedstr--compile-time-file-embedding): `#[embed]`, the other initializer exemption
