# Functions

A function takes typed parameters and may return one value. Generics, compile-time parameters and variadic packs are all forms of `fun`.

## Declaring a function

```mach
use std.print;

fun add(a: i64, b: i64) i64 {
    ret a + b;
}

fun greet(name: *u8) {
    print.printf("hello, {}\n", name);
}
```

A parameter is `name: Type`. The result type follows the parameter list, and a function that returns nothing leaves it out.

- Every path through a function with a result ends in `ret`. Falling off the end is refused.
- Parameters are locals, and assigning to one changes the function's own copy. Arguments are copied in, arrays included.
- There are no methods, no overloads and no default arguments. An operation on a value is a function that takes it: `vector.push[i64](?v, 9)`.
- A call is always a call. Recursion is never turned into a loop, so its depth is bounded by the stack.

## Function values

A function's name is a value of its function type. `nil` is a function value that points at nothing.

```mach fragment
def BinOp: fun(i64, i64) i64;

fun apply(op: BinOp, a: i64, b: i64) i64 {
    ret op(a, b);
}

fun demo() i64 {
    ret apply(add, 3, 4);
}
```

## Generics

Type parameters go in brackets after the name. Every call names its type arguments. Nothing is inferred.

```mach
fun larger[T](a: T, b: T) T {
    if (a > b) { ret a; }
    ret b;
}

fun demo() i64 {
    ret larger[i64](3, 9);
}
```

There are no constraints on `T`. The body may use any operator, cast or field on it, and each instance is checked against the types it was given. An instance that does not fit is refused where it was asked for, with a second position inside the body:

```text
error[operator.operand_type] `Point` has no ordering. `<`, `<=`, `>` and `>=` order integers and floats
```

`larger[i64]` with no call after it is that instance as a value, so a generic can be stored, passed and addressed. A bare `larger` is not a value. The compiler makes one copy of the code per distinct set of type arguments.

## Comptime parameters

A parameter written `$name: T` takes an argument the compiler knows at build time. The body names it without the `$`, and `$if` can choose code by it. Each distinct value is its own instance.

```mach
fun shift($left: u1, n: u64, by: u64) u64 {
    $if (left == 1) { ret n << by; }
    $or             { ret n >> by; }
}

fun demo(n: u64) u64 {
    ret shift(1, n, 4);
}
```

A comptime parameter can size a type inside the body, as in `var xs: [count]i64;`, but not a type in the function's own signature.

## Variadic packs

A last parameter written `name: ...` takes any number of arguments of any types. It is a compile-time list, not a runtime one: `$each` unrolls the body once per argument, at that argument's own type.

```mach
fun sum(va: ...) i64 {
    var total: i64 = 0;
    $each a in va {
        $if ($type_of(a) == i64) { total = total + a; }
        $or                      { total = total + a::i64; }
    }
    ret total;
}

fun demo() i64 {
    ret sum(1i64, 2u8, 3i32);
}
```

| Form | Meaning |
|---|---|
| `$each a in va { }` | Run the block once per argument |
| `va.len` | How many arguments the pack holds |
| `g(va...)` | Pass the whole pack on to another pack function |

Each call with a different list of types is its own instance, so a pack costs nothing at run time. A bare literal in a pack is `i64` or `f64`, so suffix any other. C-style variadics exist only on [`ext fun`](ext-fun.md#c-variadic-functions).

The full rules are in the reference for [`fun`](../language/fun.md) and [variadic packs](../language/variadics.md).
