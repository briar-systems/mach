# Visibility — `pub` and `ext`

Two declaration modifiers control how a symbol is seen.

## `pub`

Marks a declaration as part of its module's public surface. Other modules
that `use` this module can reference `pub`-marked symbols by name; symbols
without `pub` are file-private.

```mach
pub fun add(a: i64, b: i64) i64 { ret a + b; }
fun helper() { ... }            # private: only callable inside this file
pub rec Point { x: i64; y: i64; }
pub val MAX: i64 = 100;
```

Applies to: `fun`, `rec`, `uni`, `def`, `val`, `var`, `ext fun`, `ext val`,
`ext var`.

`fwd` always publishes and does not take an explicit `pub` modifier.

## `ext`

Declares a function or a data binding whose definition lives in another
object, as a forward reference the linker resolves. An `ext fun` has no body
and follows the C ABI; an `ext val` / `ext var` has no initializer and no
storage of its own.

```mach
#[symbol("write")]
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;

ext var errno: i32;
```

- The C ABI is the contract; argument and return types must be
  representable in C.
- Use the `#[symbol("real_name")]` decorator to override the linker name.

There are no body-less functions outside of `ext fun`. Regular forward
declarations do not exist.

## See also

- [fun.md](fun.md) — regular function declarations
- [ext-fun.md](ext-fun.md) — full reference for `ext fun`
- [val-var.md](val-var.md#ext--foreign-data-imports) — `ext val` / `ext var`
- [decorators.md](decorators.md) — `#[symbol]` and the other decorators
