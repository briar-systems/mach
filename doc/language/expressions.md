# Expressions

Expressions evaluate to values. They appear on the right side of bindings,
as conditions, and as call arguments.

## Literals

See [literals.md](literals.md) — numeric, char, string forms.

## Names

A bare identifier references a name in scope. Module-qualified names use
the dot path:

```mach
counter             # local or module-level binding
core.add            # symbol from module `core`
```

## Record / array / union / vector literals

A type name followed by a brace-delimited initializer:

```mach
val p:    Point             = Point{ x: 1, y: 2 };
val a:    [3]i64            = [3]i64{10, 20, 30};
val u:    Number            = Number{ i: 99 };
val v:    f32x4             = f32x4{1.0, 2.0, 3.0, 4.0};
val pair: Pair[i64, u8]     = Pair[i64, u8]{ left: 5, right: 6u8 };
```

For generics, the type arguments appear in brackets before the body.

## Field / index access

```mach
val x:     i64 = p.x;
val first: i64 = a[0];
val lane:  f32 = v[2];
```

## Function calls

```mach
add(2, 3)
identity[i64](42)               # generic call: type args in [ ]
sum(3, 10i64, 20i64, 30i64)     # variadic
```

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
