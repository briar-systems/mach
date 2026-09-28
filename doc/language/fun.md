# `fun` — function

A function takes typed arguments, optionally returns a typed value, and
has a body of statements.

## Grammar

```mach fragment
fun NAME(args) RET { ... }              # function with return type
fun NAME(args) { ... }                  # no return type
fun NAME[T](args) RET { ... }           # generic over type parameters
fun NAME($p: T, args) RET { ... }       # comptime value parameter
fun NAME(fixed, va: ...) RET { ... }    # variadic pack parameter
```

## Examples

```mach
rec Pair[T, U] { left: T; right: U; }
var counter: i64 = 0;

pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}

pub fun bump() { # no return value
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

```mach fragment
val x: i64 = identity[i64](42);
val p: Pair[i64, u8] = make_pair[i64, u8](1, 2u8);
```

The same spelling with no call after it **names the instance as a value**. Its
type is the instantiated signature, so it can be stored, passed, returned, and
addressed:

```mach fragment
val f: fun(i64) i64 = identity[i64];   # the i64 instance, as a value
val p: ptr          = ?identity[i64];  # its address
ret apply(identity[i64], 42);          # passed as a callback
```

A generic itself is a template rather than code, so a bare `identity` has no
address and cannot be a value; only an instance can.

### A body is checked at each instantiation

There are no constraints on `T`, so nothing about a type parameter is decided
before an instantiation supplies its argument. An operator, a cast, a `:~`
reinterpret, a literal beside a `T`-typed operand and a branch condition are all
resolved against the instance's concrete type, so an ordered or numeric algorithm
is writable without a comparator parameter:

```mach
fun maxof[T](a: T, b: T) T {
    if (a > b) { ret a; }
    ret b;
}

fun sum[T](p: *T, n: u64) T {
    var acc: T   = 0; # the literal takes `T`
    var i:   u64 = 0;
    for (i < n) {
        acc = acc + p[i];
        i   = i + 1;
    }
    ret acc;
}
```

A type that genuinely does not support the operator is refused at the
instantiation that asked for it, naming that type:

```
error[operator.operand_type]: `Point` has no ordering: `<`, `<=`, `>` and `>=` order integers and floats
  --> src/main.mach:12:5
   |
12 |     maxof[Point](p, q);
   |     ^^^^^^^^^^^^^^^^^^
   |
  --> src/main.mach:4:9
   |
 4 |     if (a > b) { ret a; }
   |         ----- in this generic body, checked against this instance's type arguments
```

The template itself reports nothing. It types the body so each instance and the
lowering have an expression table to read, and every judgement is the instance's,
which is what keeps one bad instantiation to one refusal at one site.

**A generic nothing instantiates is not checked.** `fun never_used[T](a: T) T {
ret a + a; }` compiles, because there is no type to decide `+` against and no
site to report at. The way to check a generic is to instantiate it, so a library
instantiates its own generics in its own tests.

## Comptime value parameters

A parameter marked with `$name: T` must be supplied with a value the
compiler can resolve at compile time. The function body can branch on the
comptime parameter via `$if`, producing different code per call-site
instantiation.

```mach fragment
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
use std.print;
use std.runtime;

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

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {}", sum(10i64, 20i64, 30i64), bias(1, 10i64, 20i64, 30i64));
    ret 0;
}
```

`va.len` folds to the element count. `g(va...)` forwards the whole pack to
another pack-tailed callee. See [variadics.md](variadics.md) for the full
reference.

## Tail calls

The release pipeline (`opt = 2`) treats a call whose result the function
returns unchanged, as in `ret f(a, b);`, as a tail call. A call to the function
itself becomes a loop: the recursion runs in the stack of one frame at any
depth. `ret f(n - 1) + x`, and the same with `*`, `|` or `^` on integers, is a
loop too, with the pending operands carried in an accumulator. A call to any
other function becomes a jump that hands the callee the caller's frame, so a
chain of such calls, mutual recursion included, also runs in constant stack.

A call stays a call when the function has a `fin` pending at the `ret`, when
the address of one of its locals escapes (to the callee or to any earlier
call), when it runs inline assembly or is `#[naked]` or `#[oblivious]`, when a
by-value argument is a copy the caller has to make, or when the callee takes
more stack arguments than the caller received. The debug pipeline (`opt = 0`)
makes no tail calls.

`-g` changes none of this: debug information is added to the same code. A
debugger or unwinder therefore never sees the frame a tail call left. A
backtrace taken inside the callee shows it directly below the caller's own
caller, and a self tail call shows as the one frame of the loop, its parameters
holding the current iteration's values. A local of the caller that holds the
call's result reads as optimized out, since the caller never receives it.

## See also

- [ext-fun.md](ext-fun.md) — body-less external functions
- [variadics.md](variadics.md) — variadic pack parameter reference
- [comptime-control.md](comptime-control.md) — `$if` inside function bodies, and
  the two regimes a generic body is checked under
- [operators.md](operators.md) — the operator table an instance is checked against
- [expressions.md](expressions.md) — function calls and generic
  instantiation
