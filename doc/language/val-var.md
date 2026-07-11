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
```

## Scope

`val` and `var` work at module top level and inside function bodies.

At module top level:

- `pub val NAME` exports the constant.
- `pub var NAME` exports the variable as a writable global.

Inside functions, they are local to the enclosing block.

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
- The `symbol` and `library` decorators and the static/dynamic linking inputs
  work exactly as for [`ext fun`](ext-fun.md).
- On a dynamic target the reference is emitted GOT-indirect so the loader binds
  it to the runtime definition (ELF: an `R_*_GLOB_DAT` GOT slot); an ordinary
  cross-module reference to a `val`/`var` defined elsewhere in the same artifact
  stays directly addressed. Executed dynamic-import resolution is proven on the
  native ELF legs.

## No inference

Every binding declares its type. An untyped numeric literal is checked
against the binding's declared type; it does not participate in inferring
that type.

```mach
val n: i64 = 42;                    # ok — 42 conforms to i64
val x       = 42;                   # ERROR — no type to check against
```

If the surrounding context doesn't constrain the literal's type, use a
typed suffix (`42i64`).

## See also

- [literals.md](literals.md) — numeric / string / char literal forms
- [types.md](types.md) — the type grammar for the annotation
