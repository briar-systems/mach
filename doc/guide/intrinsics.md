# Intrinsics

An intrinsic is a function built into the compiler, written `$name(args)`. All but one are answered at compile time: they measure a type, ask what kind it is, or list its parts.

## The set

```mach
rec Point {
    x: i64;
    y: i64;
    z: u8;
}

val SIZE:   u64 = $size_of(Point);
val OFFSET: u64 = $offset_of(Point, z);
val NAME:   *u8 = $type_name(*Point);
```

| Intrinsic | Operand | Gives |
|---|---|---|
| `$size_of(T)` | A type | Its size in bytes |
| `$align_of(T)` | A type | Its alignment in bytes |
| `$length_of(T)` | An array or vector type | Its element count |
| `$offset_of(T, name)` | A record, union or tag, and a field or payload case | The byte offset of that field |
| `$type_of(x)` | An expression | Its type, compared with `==` or `!=` against a type in a `$if` |
| `$type_name(T)` | A type | Its spelling as a `*u8` string, here `"*Point"` |
| `$type_id(T)` | A type | A `u64` that identifies it |
| `$pointee_of(T)` | A pointer type `*U` | The type `U` |
| `$discriminant_of(T)` | A tag | The integer type that stores its case |
| `$is_record(T)`, `$is_union(T)`, `$is_tag(T)` | A type | Whether it was declared with `rec`, `uni` or `tag` |
| `$is_pointer(T)` | A type | Whether it is `ptr` or a `*U` |
| `$is_integer(T)`, `$is_float(T)` | A type | Whether it is an integer or a float of any width |
| `$is_secret(T)` | A type | Whether it is a `^U` |
| `$holds_secret(T)` | A type | Whether any part of it is secret |
| `$fields(T)` | A record | Its fields, for `$each` |
| `$cases(T)` | A tag | Its cases, for `$each` |
| `$zeroize(p, n)` | A pointer and a count | No value. Zeroes `n` values at `p` at run time, and no optimization removes the write |

- A type operand is any type: `*Point`, `[4]u8`, `Box[i64]`, a generic's `T`. A binding there means its own type, so `$length_of(LOGO)` counts the bytes of an [embedded](decorators.md) file.
- `$size_of`, `$align_of`, `$length_of` and `$offset_of` give an untyped integer, as a literal does. It takes the binding's type, and a value that does not fit is refused.
- `$pointee_of` and `$discriminant_of` give a type, so they are written where a type is: `var d: $discriminant_of(Reply);`.
- The `$is_*` predicates and `$holds_secret` are `$if` conditions only, never run-time values.
- A predicate looks at the outermost type. `^Point` is a secret and not a record, and `$fields(^Point)` is refused.
- Inside a generic, every intrinsic is answered once per instance.

## Record fields

`$each f in $fields(T)` copies its block once per field. `v.[f]` is that field of `v`, at the field's own type.

```mach
use std.print;

fun dump[T](v: *T) {
    $each f in $fields(T) {
        print.printlnf("{} at {} = {}", f.name, f.offset, v.[f]);
    }
}

fun total[T](v: *T) f64 {
    var sum: f64 = 0.0;
    $each f in $fields(T) {
        $if ($is_integer(f.type) || $is_float(f.type)) {
            sum = sum + v.[f]::f64;
        }
    }
    ret sum;
}
```

For `Point{x: 3, y: 4, z: 5}`, `dump` prints `x at 0 = 3`, `y at 8 = 4` and `z at 16 = 5`, and `total` gives 12.

| Form | Meaning |
|---|---|
| `f.name` | The field's name, a `*u8` string |
| `f.type` | The field's type, usable as a type operand, a type argument and in `==` |
| `f.offset` | The field's byte offset |
| `v.[f]` | The field on `v`, readable, assignable and addressable |

Passing `f.type` as a type argument is how a walk descends into nested records: `total[f.type](?v.[f])` under `$if ($is_record(f.type))`. std's `std.derive` is written this way.

## Tag cases

`$each c in $cases(T)` copies its block once per case, in declaration order.

```mach
tag Reply: u8 {
    empty;
    code: i32;
    text: *u8;
}

fun case_name[T](v: *T) *u8 {
    $each c in $cases(T) {
        if (sel v.[c]) { ret c.name; }
    }
    ret "?";
}

fun payload_bytes[T](v: *T) u64 {
    $each c in $cases(T) {
        $if (c.has_payload) {
            if (sel v.[c]) { ret $size_of(c.type); }
        }
    }
    ret 0;
}
```

| Form | Meaning |
|---|---|
| `c.name` | The case's name, a `*u8` string |
| `c.code` | The discriminator value stored for the case |
| `c.has_payload` | A `$if` condition: whether the case holds a payload |
| `c.type`, `c.offset` | The payload's type and byte offset, on a case that has one |
| `sel v.[c]`, `v.[c]` | The case test and the payload it guards |
| `T.[c]{payload}` | A value of that case |

The full rules are in the reference for [intrinsics](../language/comptime-intrinsics.md).
