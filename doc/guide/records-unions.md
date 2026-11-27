# Records, unions and tags

A `rec` gives each field its own storage, a `uni` overlays its fields on the same bytes, and a `tag` holds exactly one of its cases and knows which. All three are plain values, copied by assignment, argument and return.

## Records

```mach
rec Point {
    x: i64;
    y: i64;
}

fun reset(p: *Point) {
    p.x = 0;
    p.y = 0;
}

fun demo() i64 {
    var a: Point = Point{x: 3, y: 4};
    val b: Point = Point{x: 3};
    a.y = a.y + b.y;
    reset(?a);
    ret a.x;
}
```

- A literal names the type and its fields. A positional `Point{3, 4}` is refused.
- A field the literal leaves out is zero, so `b.y` is 0 and `Point{}` is all zero.
- `p.x` reads through a pointer to a record.
- `==` and `!=` are refused on a whole record, union or tag. Compare the fields, or call `std.derive.eq[Point](?a, ?b)`.

A record, union or tag takes type parameters in brackets, and every use names the arguments:

```mach
rec Pair[T, U] { left: T; right: U; }
val pair: Pair[i64, u8] = Pair[i64, u8]{left: 5, right: 6};
```

Padding and alignment are set with `#[packed]` and `#[align(N)]`, listed under [decorators](decorators.md).

## Unions

```mach
uni Number {
    i: u64;
    f: f64;
}

fun bits(f: f64) u64 {
    val n: Number = Number{f: f};
    ret n.i;
}
```

A union is as large as its largest field, and a literal names the one field it writes. The compiler does not track which field was written last, so reading another one reinterprets the bytes. Keep unions for foreign layouts and wire formats, and use a tag everywhere else.

## Tags

```mach
rec Size {
    w: f64;
    h: f64;
}

tag Shape: u8 {
    dot;
    circle: f64;
    rect:   Size;
}

val a: Shape = Shape.dot{};
val b: Shape = Shape.circle{2.0};
val c: Shape = Shape.rect{Size{w: 3.0, h: 4.0}};
```

- The type after the name is the discriminator: `u8`, `u16`, `u32` or `u64`.
- A case holds one payload or none. Several values travel as a record.
- A value is built as `Type.case{payload}`, with empty braces for a case that has no payload. There is no other form.
- A zeroed tag holds its first case.

## Case tests and guards

`sel place.case` is a `u1` that is true when the place holds that case. Its operand is a place, never a call, and a pointer to a tag is read through. The payload is `place.case`, and it can be read, written or addressed only inside a guard for that case.

```mach fragment
fun area(s: Shape) f64 {
    if (sel s.dot)    { ret 0.0; }
    or (sel s.circle) { ret 3.14159 * s.circle * s.circle; }
    ret s.rect.w * s.rect.h;
}

fun is_big(s: Shape) u1 {
    ret sel s.circle && s.circle > 100.0;
}
```

A guard opens in three places:

1. The block of an arm whose whole condition is `sel p.c`, as in the second arm of `area`.
2. The right operand of `&&` whose left operand is `sel p.c`, as in `is_big`. The guard covers that operand only, not the block of an `if` around it.
3. The rest of the block after an `if` chain whose every arm leaves by `ret`, `brk` or `cnt` and whose tests leave one case over, as `s.rect` in `area`. A final bare `or` is guarded the same way.

Anywhere else the payload is refused:

```text
error[tag.payload_unguarded] a tag payload requires a `sel` guard for its case
```

A guard is a region of source, matched by the spelling of the place.

- `||`, `!` and every other operator open no guard, though an arm that tests `!sel p.c` counts toward rule 3.
- Assigning the whole place inside its guard is refused.
- Assigning a name the place is spelled with, such as the `i` of `items[i]`, ends the guard from there on. Copy the payload into a `val` first.

## Failure handling

Failure is a value. std declares three ordinary tags for it, each with the failure case first, so a zeroed one is a failure or `none`.

| Type | Cases | Holds |
|---|---|---|
| `res[T, E]` | `err: E`, `ok: T` | a value or an error |
| `opt[T]` | `none`, `some: T` | a value or nothing |
| `err[E]` | `err: E`, `ok` | success or an error |

```mach
use std.types.result.res;

tag ParseError: u8 {
    empty;
    bad_digit: u8;
}

fun digit(c: u8) res[i64, ParseError] {
    if (c < '0' || c > '9') { ret res[i64, ParseError].err{ParseError.bad_digit{c}}; }
    ret res[i64, ParseError].ok{(c - '0')::i64};
}

fun doubled(c: u8) res[i64, ParseError] {
    val r: res[i64, ParseError] = digit(c);
    if (sel r.err) { ret res[i64, ParseError].err{r.err}; }
    ret res[i64, ParseError].ok{r.ok * 2};
}
```

There is no `try` and no `?` operator. `doubled` tests the failure, returns it, and reads `r.ok` afterwards under rule 3. `opt` and `err` live in `std.types.option` and `std.types.error` and are handled the same way.

The full rules are in the reference for [`rec`](../language/rec.md), [`uni`](../language/uni.md) and [`tag`](../language/tag.md).
