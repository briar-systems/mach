# Expressions

Expressions evaluate to values. They appear on the right side of bindings,
as conditions, and as call arguments.

Reading an aggregate captures its value at that evaluation point. A later
argument, assignment destination expression, or `fin` body cannot change
the captured value by modifying its original storage. Call arguments evaluate
left to right. Assignment evaluates and captures the right side before
evaluating the destination on the left side.

## Literals

See [literals.md](literals.md) — numeric, char, string, and `nil` forms.

## Names

A bare identifier references a name in scope. Module-qualified names use
the dot path:

```mach
counter             # local or module-level binding
core.add            # symbol from module `core`
```

## Record / array / union / tag literals

A type name followed by a brace-delimited initializer:

```mach accept
use std.types.canonical.res;
use std.types.canonical.err;

rec Point { x: i64; y: i64; }
uni Number { i: i64; f: f64; }
rec Pair[T, U] { left: T; right: U; }
tag Reply: u8 { empty; value: i64; }
tag MyErr: u8 { bad; }

val p:    Point             = Point{ x: 1, y: 2 };
val a:    [3]i64            = [3]i64{10, 20, 30};
val u:    Number            = Number{ i: 99 };
val pair: Pair[i64, u8]     = Pair[i64, u8]{ left: 5, right: 6u8 };
val rep0: Reply             = Reply.empty{};
val rep1: Reply             = Reply.value{42};
val good: res[i64, MyErr]   = res[i64, MyErr].ok{42};
val done: err[MyErr]        = err[MyErr].ok{};
```

For generics, the type arguments appear in brackets before the body.

A tag value names its type and its case, and carries a positional payload only
when the case declares one. Omitting a required payload, supplying a payload to a
payloadless case, supplying more than one, naming the payload, or using the
withdrawn record-literal form is a compile error.

Vector literals (`f32x4{ 1.0, 2.0, 3.0, 4.0 }`) follow the same brace shape, but
require one positional initializer per lane. See [types.md](types.md#simd-vectors).

## Field, index, and tag access

```mach run "1 10"
use std.runtime;
use print: std.print;

rec Point { x: i64; y: i64; }

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val p: Point  = Point{ x: 1, y: 2 };
    val a: [3]i64 = [3]i64{10, 20, 30};
    val x:     i64 = p.x;            # record field
    val first: i64 = a[0];           # array index
    print.printlnf("{} {}", x, first);
    ret 0;
}
```

An index the compiler can fold is bounds-checked against a statically known
length, such as a fixed array length `N` or a vector lane count. See
[types.md](types.md).

For tagged values:

- `sel tag_val.case` tests whether that case is currently selected. It reads only the discriminator and is an ordinary `bool`.
- `tag_val.case` accesses the payload of that case. It is legal only inside a lexical guard for that place and case; an unguarded payload access is a compile error.
- `TypeName.case` alone is a case selector, not a value. It cannot be stored or passed.

See [tag.md](tag.md) for the guard rules. There is no other operator over a
tag: failure handling is an ordinary `if`/`or` chain over `sel`, and the
guard it opens is what makes the payload readable.

## Function calls

```mach
add(2, 3)
identity[i64](42)               # generic call: type args in [ ]
sum(3, 10i64, 20i64, 30i64)     # variadic pack call (see variadics.md)
```

A call to a pack-tailed function is monomorphized per distinct trailing
type-list; `g(va...)` forwards a whole pack — see
[variadics.md](variadics.md).

For comptime parameters, the value is passed positionally like a runtime
argument — the function signature determines whether it must be comptime:

```mach
checked_add(MODE_FAST, 1, 2)    # MODE_FAST is comptime-knowable
```

## Operators

See [operators.md](operators.md). Operators combine expressions into larger
expressions; precedence follows the usual C-family conventions.

## See also

- [statements.md](statements.md) — how expressions appear inside statements
- [fun.md](fun.md) — function declarations and signatures
