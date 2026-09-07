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

## Record / array / union literals

A type name followed by a brace-delimited initializer:

```mach
val p:    Point             = Point{ x: 1, y: 2 };
val a:    [3]i64            = [3]i64{10, 20, 30};
val u:    Number            = Number{ i: 99 };
val pair: Pair[i64, u8]     = Pair[i64, u8]{ left: 5, right: 6u8 };
```

For generics, the type arguments appear in brackets before the body.

Vector literals (`f32x4{ 1.0, 2.0, 3.0, 4.0 }`) follow the same shape, one
initializer per lane — see [types.md](types.md#simd-vectors).

## Field / index access

```mach
val x:     i64 = p.x;            # record field
val first: i64 = a[0];           # array index
```

An index the compiler can fold is bounds-checked against a statically known
length — a fixed array's `N`, a vector's lane count — see
[types.md](types.md).

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
