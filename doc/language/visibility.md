# Visibility — `pub` and `ext`

Two declaration modifiers control how a symbol is seen.

## `pub`

Marks a declaration as part of its module's public surface. Other modules
that `use` this module can reference `pub`-marked symbols by name; symbols
without `pub` are file-private.

```mach error no symbol `helper` exported by `example.lib`
# file: src/lib.mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}

# private: only callable inside this file
fun helper() i64 {
    ret 1;
}

pub rec Point { x: i64; y: i64; }
pub val MAX: i64 = 100;

# file: src/main.mach
use example.lib;

fun sum() i64 {
    ret lib.add(lib.MAX, 1); # fine: pub
}

fun peek() i64 {
    ret lib.helper(); # error: helper is not exported
}
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

## What a shared library exports

A `kind = "shared"` artifact exports exactly the declarations marked
[`#[export]`](decorators.md#export--a-shared-librarys-export), in any project of
the build, and nothing else. `pub` states a module's interface to other Mach
modules and never exports, so a `pub fun` is reachable from your code and absent
from the library's export table. Every other definition, with or without `pub`,
and every definition the compiler synthesizes, such as a generic or pack
instance, is hidden: other modules in the same link resolve it normally, and no
consumer of the linked library can bind to it.

`#[symbol("name")]` chooses the linker name, not the visibility. A declaration
with a `#[symbol]` name and no `#[export]` is still hidden, so a C program cannot
link against it. A function a C caller links against is `#[export]`.

A `fwd` re-export is a declaration of Mach surface and exports nothing from a
library. A generic, comptime-parameter or pack declaration has instances rather
than one definition, so the way to export one instantiation is an `#[export]`
non-generic wrapper around it.

A shared library that exports nothing is built: its export table is empty.

An executable exports nothing at all, so `pub` makes no difference to one.

## See also

- [fun.md](fun.md) — regular function declarations
- [ext-fun.md](ext-fun.md) — full reference for `ext fun`
- [val-var.md](val-var.md#ext--foreign-data-imports) — `ext val` / `ext var`
- [decorators.md](decorators.md) — `#[symbol]` and the other decorators
