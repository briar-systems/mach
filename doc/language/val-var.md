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
