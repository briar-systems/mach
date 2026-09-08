# Expressions

Tag, canonical type, and `try` descriptions include the accepted v5 contract.
Implementation is incomplete at base commit `fc5c9e7e`. See
[tag.md](tag.md#implementation-status) and [try.md](try.md#implementation-status).

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

```mach
val p:    Point             = Point{ x: 1, y: 2 };
val a:    [3]i64            = [3]i64{10, 20, 30};
val u:    Number            = Number{ i: 99 };
val pair: Pair[i64, u8]     = Pair[i64, u8]{ left: 5, right: 6u8 };
val rep0: Reply             = Reply{empty};
val rep1: Reply             = Reply{value: 42};
val good: res[i64, MyErr]   = res[i64, MyErr]{ok: 42};
val done: err[MyErr]        = err[MyErr]{ok};
```

For generics, the type arguments appear in brackets before the body.

A tag literal requires selecting exactly one case. Selecting zero cases, selecting
multiple cases, omitting a required payload, or supplying a payload to a
payloadless case is a compile error.

Vector literals (`f32x4{ 1.0, 2.0, 3.0, 4.0 }`) follow the same brace shape, but
require one positional initializer per lane. See [types.md](types.md#simd-vectors).

## Field, index, and tag access

```mach
val x:     i64 = p.x;            # record field
val first: i64 = a[0];           # array index
```

An index the compiler can fold is bounds-checked against a statically known
length, such as a fixed array length `N` or a vector lane count. See
[types.md](types.md).

For tagged values:

- `TypeName.case` is a case selector used in comparisons. It is not a value and cannot be stored or passed.
- `tag_val.case` accesses the payload of that case. It requires a current compiler proof that the case is active. Reading an unproved payload is a compile error.

See [tag.md](tag.md) for proof tracking and alias invalidation rules.

## `try` expressions

A `try` expression performs explicit, visible failure handling for canonical
`res[T, E]`, `opt[T]`, and `err[E]` values:

```mach
val number: i64 = try parse(input) or (error: ParseError) {
    ret res[i64, ParseError]{err: error};
};
```

On success, `try` extracts the active payload. On failure, it binds the error
(if applicable) and executes a mandatory terminating failure block. A failed
`try` skips the remainder of the enclosing expression and does not initialize its
destination.

In assignments, the right hand side evaluates and captures before the destination
place on the left hand side is evaluated.

Ordinary user-defined tags do not acquire an automatic `try` convention. See
[try.md](try.md) for complete failure handling rules.

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
