# `fun` — function

A function takes typed arguments, optionally returns a typed value, and
has a body of statements.

## Grammar

```mach
fun NAME(args) RET { ... }              # function with return type
fun NAME(args) { ... }                  # no return type
fun NAME[T](args) RET { ... }           # generic over type parameters
fun NAME($p: T, args) RET { ... }       # comptime value parameter
fun NAME(fixed, va: ...) RET { ... }    # variadic pack parameter
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

## Variadic packs

A function with a trailing named pack parameter (`va: ...`) accepts a
variable number of trailing arguments. The compiler monomorphizes the
function once per distinct call-site type-list; the pack is consumed by
`$each a in va` at compile time — there is no runtime `va_list`.

```mach
pub fun sum(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        t = t + a;
    }
    ret t;
}

# leading fixed params are allowed before the pack
pub fun bias(base: i64, va: ...) i64 {
    var t: i64 = base;
    $each a in va { t = t + a; }
    ret t;
}
```

`va.len` folds to the element count. `g(va...)` forwards the whole pack to
another pack-tailed callee. See [variadics.md](variadics.md) for the full
reference.

## See also

- [ext-fun.md](ext-fun.md) — body-less external functions
- [variadics.md](variadics.md) — variadic pack parameter reference
- [comptime-control.md](comptime-control.md) — `$if` inside function bodies
- [expressions.md](expressions.md) — function calls and generic
  instantiation
