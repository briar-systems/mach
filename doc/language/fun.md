# `fun` — function

A function takes typed arguments, optionally returns a typed value, and
has a body of statements.

## Grammar

```mach
fun NAME(args) RET { ... }              # function with return type
fun NAME(args) { ... }                  # no return type
fun NAME[T](args) RET { ... }           # generic over type parameters
fun NAME($p: T, args) RET { ... }       # comptime value parameter
fun NAME(a, b, ...) RET { ... }         # variadic (trailing ...)
```

## Examples

```mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}

pub fun bump() {                        # no return value
    counter = counter + 1;
}

pub fun identity[T](value: T) T {
    ret value;
}

pub fun make_pair[T, U](a: T, b: U) Pair[T, U] {
    var p: Pair[T, U];
    p.left  = a;
    p.right = b;
    ret p;
}

pub fun sum(count: i64, ...) i64 {
    var ap: va_list;
    va_start(?ap);
    var total: i64 = 0;
    var i:     i64 = 0;
    for (i < count) {
        total = total + va_arg[i64](?ap);
        i = i + 1;
    }
    va_end(?ap);
    ret total;
}
```

## Generic type parameters

Generic functions take type parameters in brackets `[T]`. There are no
constraints; any type may be substituted. The compiler monomorphizes per
unique type instantiation.

Call sites supply the types explicitly:

```mach
val x: i64 = identity[i64](42);
val p: Pair[i64, u8] = make_pair[i64, u8](1, 2u8);
```

## Comptime value parameters

A parameter marked with `$name: T` must be supplied with a value the
compiler can resolve at compile time. The function body can branch on the
comptime parameter via `$if`, producing different code per call-site
instantiation.

```mach
pub fun pick_op($mode: Mode, a: i64, b: i64) i64 {
    $if (mode == MODE_FAST) {
        ret a + b;
    }
    $or (mode == MODE_SAFE) {
        # extra logic here
        ret a + b;
    }
}
```

Comptime value parameters apply to function parameters only — not record
fields, not other contexts.

## Variadic functions

A function ending in `...` accepts a variable number of trailing arguments.
The body accesses them through `va_list` / `va_start` / `va_arg` / `va_end`,
matching C's convention.

## See also

- [ext-fun.md](ext-fun.md) — body-less external functions
- [comptime-control.md](comptime-control.md) — `$if` inside function bodies
- [expressions.md](expressions.md) — function calls and generic
  instantiation
