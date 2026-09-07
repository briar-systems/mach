# `val` and `var` — bindings

Bindings introduce named values. `val` is immutable; `var` is mutable. Both
require an explicit type — Mach has no type inference.

## Grammar

```mach
val NAME: TYPE = EXPR;              # immutable; initializer required
var NAME: TYPE;                     # mutable; default-initialized
var NAME: TYPE = EXPR;              # mutable; explicit initializer
```

## Examples

```mach
val pi: f64 = 3.14159;
val n:  i64 = 42;

var counter: i64 = 0;
var buf:     [256]u8;               # default-initialized to zero

counter = counter + 1;              # var is reassignable
n = 43;                             # ERROR — `n` is a val
```

## Immutability

Assigning to a `val` is a compile-time error, at module scope and inside a function
alike. So is writing a field or an element of one (`v.field = x`, `v[i] = x`): the
store lands in the same storage the binding names.

A pointer is the exception that proves the rule — `val p: *T` binds the **address**
immutably, not the storage it addresses, so `@p = x`, `p.field = x` and `p[i] = x`
write the pointee and are legal.

The check ends where the address escapes. `?NAME` on a `val` yields a plain `*T` —
there is deliberately no read-only pointer type — and writing through any pointer
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
in every module that imports it — however the reference is spelled (bare,
module-qualified, or through a `use` alias). No load is emitted, and arithmetic
over it folds and strength-reduces like any other literal. Its storage is still
emitted, so `?NAME` has an address, and a `val` whose initializer is not a
compile-time constant keeps its load.

A constant whose value **is** an address — a `str`, whose value is a pointer to
its bytes — is never duplicated this way. It keeps one definition and every
reference reads it, so two references to one `str` constant compare pointer-equal
no matter which module they are in or how they are spelled.

## `ext` — foreign data imports

`ext val` / `ext var` declares a binding whose storage lives in another object,
imported by name — the data analogue of [`ext fun`](ext-fun.md). It is a
forward reference the linker resolves, so it is **storage-less** and carries no
initializer:

```mach
ext var errno: i32;                        # imported mutable datum
#[symbol("environ")] ext var env: **u8;    # renamed import
#[library("libfoo.so")] ext val foo_flags: u32;  # library-pinned import
```

- No initializer. `ext val x: T = ...;` is an error — the definition, and its
  value, live in the providing object (mirrors `ext fun`'s absent body).
- The symbol name (the target's C spelling of the identifier), the `symbol` and
  `library` decorators, and the static/dynamic linking inputs all work exactly as
  for [`ext fun`](ext-fun.md).
- On a dynamic target the reference is emitted GOT-indirect so the loader binds
  it to the runtime definition. ELF uses a dynamic pointer relocation,
  `GLOB_DAT` on x86-64 and ARM64 or `R_RISCV_64` on RV64. An ordinary
  cross-module reference to a `val`/`var` defined elsewhere in the same artifact
  stays directly addressed. Executed dynamic-import resolution is proven on the
  native ELF legs.

## `#[embed(...)]` — the other exemption to "requires an initializer"

A `val` carrying `#[embed("path")]` also carries no initializer — the named
file's content **is** the initializer, read at compile time. It is the second
(and only other) exemption to `val`'s initializer requirement, alongside `ext`
above; unlike `ext`, the binding still owns real storage, placed in read-only
data. See [decorators.md](decorators.md#embedstr--compile-time-file-embedding)
for the full rule set.

## No inference

Every binding declares its type. An untyped numeric literal is checked
against the binding's declared type; it does not participate in inferring
that type.

```mach
val n: i64 = 42;                    # ok — 42 conforms to i64
val x       = 42;                   # ERROR — a binding declares its type
val y       = 42i64;                # ERROR — a suffix is not an annotation
```

The annotation is required whatever the initializer is: a typed suffix
gives the literal a type, it does not give the binding one. Suffixes earn
their keep where there is no annotation to read from, such as the elements
of a pack tail. See [literals.md](literals.md#typed-suffixes).

## See also

- [literals.md](literals.md) — numeric / string / char literal forms
- [types.md](types.md) — the type grammar for the annotation
- [decorators.md](decorators.md#embedstr--compile-time-file-embedding) — `#[embed]`, the other initializer exemption
