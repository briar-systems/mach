# Intrinsics

Intrinsics are compiler-shipped comptime functions. They have the same
syntactic shape as user-defined function calls (`$name(args)`), but their
names are reserved and their implementations are built into the compiler.

The set is closed; adding a new intrinsic requires a compiler change.

## Value intrinsics

Return comptime constant unsigned integers. The storage type is whatever
the binding declares — Mach has no compiler-known `usize`.

```mach
$size_of(T)             # byte size of type T
$align_of(T)            # byte alignment of type T
$offset_of(T, field)    # byte offset of T's field
```

```mach
pub val POINT_SIZE: i64 = $size_of(Point);
pub val POINT_X:    i64 = $offset_of(Point, x);
```

## Type intrinsic

`$type_of(expr)` produces a comptime type value — the resolved type of its
argument `expr`. Type values have no runtime representation; they are only
meaningful as operands in comptime type comparisons.

```mach
$type_of(expr)          # comptime type value of expr
```

Type values can be compared with `==` / `!=` inside `$if` conditions:

```mach
$if ($type_of(arg) == i64) { write_i64(w, arg); }
$or ($type_of(arg) == str) { write_str(w, arg); }
$or { $error("unsupported type"); }
```

A bare type name (e.g. `i64`, `str`, `Point`) is the other valid operand.
The comparison selects one branch at compile time per monomorphization
instance — useful for per-element type dispatch inside `$each` bodies.

The provably-dead arms are **pruned** before type-checking, so each arm uses
`arg` at its own concrete type with no per-arm cast: the `str` arm above is
never checked against a `u64` element. Only the selected arm is type-checked
and emitted.

## Field intrinsic and projection

`$fields(T)` produces a comptime sequence of field descriptors for record or
union type `T`. Each descriptor carries three readable properties:

| Property   | Type     | Value                                        |
|------------|----------|----------------------------------------------|
| `f.name`   | `*u8`    | field name as a NUL-terminated string        |
| `f.type`   | type val | comptime type value of the field's type      |
| `f.offset` | integer  | byte offset of the field in `T`'s layout     |

`$fields(T)` is consumed by `$each f in $fields(T)`. Inside the loop body,
`v.[f]` projects the concrete field off an instance `v` — it is an lvalue
(readable and writable, including through a pointer receiver).

```mach
$fields(T)              # comptime field sequence for record/union T
v.[f]                   # comptime field projection: access the field f on v
```

```mach
rec Pair { x: i64; y: i64; }

fun sum(p: Pair) i64 {
    var total: i64 = 0;
    $each f in $fields(Pair) {
        total = total + p.[f];      # p.x on iteration 1, p.y on iteration 2
    }
    ret total;
}
```

`$each f in $fields(Empty)` expands to nothing when `T` has no fields.

### Heterogeneous fields

Because each `$each` iteration re-types `v.[f]` to the concrete field type,
heterogeneous records work naturally:

```mach
rec Mixed { a: i64; b: u8; }

fun total(m: Mixed) i64 {
    var t: i64 = 0;
    $each f in $fields(Mixed) {
        t = t + m.[f]::i64;     # m.a (i64) on iter 1, m.b (u8) cast to i64 on iter 2
    }
    ret t;
}
```

### Descriptor reads

Field descriptor properties can be read inside the loop body:

```mach
fun offsum(m: Mixed) i64 {
    var s: i64 = 0;
    $each f in $fields(Mixed) {
        s = s + f.offset::i64;    # 0 + 8 = 8 for Mixed { a: i64; b: u8; }
    }
    ret s;
}

fun count_i64(m: Mixed) i64 {
    var n: i64 = 0;
    $each f in $fields(Mixed) {
        $if (f.type == i64) { n = n + 1; } $or { }
    }
    ret n;
}
```

Note: if a record has a field literally named `type`, that ordinary field
access (`v.type`) is unaffected — `v.[f]` projection uses the `$each` loop
variable, which is always a field descriptor, never a regular member.

### Nested `$each`

`$each` can be nested:

```mach
fun cross(p: Pair, q: Pair) i64 {
    var t: i64 = 0;
    $each f in $fields(Pair) {
        $each g in $fields(Pair) {
            t = t + p.[f] * q.[g];
        }
    }
    ret t;
}
```

## `$each` — compile-time unroll

`$each` is a statement form that splices its body once per element of a
comptime sequence. There are three sequence forms:

```mach
$each f in $fields(T) { ... }    # one iteration per field of T
$each a in va { ... }            # one iteration per element of pack va
$each x in ARR { ... }           # one iteration per element of a constant array val
```

`$each` is valid only in statement scope (inside a function body). It is not
a loop — the body is duplicated at compile time, not iterated at runtime.
Enclosing runtime variables (e.g. an index or accumulator) are shared across
all unrolled copies.

See [variadics.md](variadics.md) for the pack form (`$each a in va`).

### `$each` over a comptime-constant array

`$each x in ARR` unrolls the body once per element of `ARR`, binding `x` to that
element's compile-time constant per iteration. Unlike the pack and `$fields`
forms, every element shares one type (the array's element type), so the loop
variable is an ordinary constant value: it reads as a value, casts, dispatches a
per-element `$if`, and — for a record element — projects fields with `x.field`.

```mach
val PRIMES: [4]i64 = [4]i64{2, 3, 5, 7};

fun sum() i64 {
    var total: i64 = 0;
    $each x in PRIMES {
        total = total + x;      # x is 2, then 3, then 5, then 7
    }
    ret total;                  # 17
}
```

A per-element `$if` selects its arm from the element's constant, so heterogeneous
handling falls out of the unroll:

```mach
rec Rule { tag: i64; fn: fun(i64) i64; }

val RULES: [3]Rule = [3]Rule{
    Rule{tag: 1, fn: inc},
    Rule{tag: 2, fn: dbl},
    Rule{tag: 3, fn: neg},
};

fun run(n: i64) {
    $each r in RULES {
        $if (r.tag == 2) { use_double(r.fn(n)); }   # r.fn folds to the element's function
        $or              { use_other(r.tag, r.fn(n)); }
    }
}
```

**Eligibility.** `ARR` must name an immutable `val` (never a `var`) declared in
the current module, whose type is a fixed-size array `[N]E` fully initialized by
an array literal of exactly `N` elements. `E` must be a scalar or record type;
nested-array element types are not supported. An empty array (`[0]E`) unrolls to
nothing. Each violation is reported with a teaching diagnostic.

`x.field` on a record element projects the element's constant: a scalar field
folds to a constant, a function-pointer field yields a function reference, and a
record field materializes the nested literal. Projection is one level deep
(`x.field`); `x` itself is a constant and has no address (`?x` is rejected).

## Diagnostic intrinsics

`$error("msg")` fails compilation with `msg` when it is **reached** — on a live
path: an unconditional position, or a `$if` / `$or` arm the compiler selects. A
`$error` in a discarded (dead) arm never fires, so it is the natural total-
coverage fallback for a `$type_of` dispatch — the unhandled-type `$or {}` arm
fails the build at compile time instead of falling through to a runtime error.

`$error` is valid in both declaration and statement scope and takes one
string-literal message.

```mach
$error("msg")           # fails compilation when reached

$if (!supported) {
    $error("this target is not supported");
}

$if ($type_of(arg) == i64) { write_i64(w, arg); }
$or ($type_of(arg) == str) { write_str(w, arg); }
$or { $error("no writer for this argument type"); }    # compile error on an unhandled type
```

> **`$assert` not yet implemented.** `$assert` parses as a comptime directive
> but the compiler does not yet evaluate it. It is intended as sugar over `$if`
> plus `$error` — `$assert(cond, "msg")` ≡ `$if (!cond) { $error("msg"); }`,
> e.g. `$assert($size_of(i64) == 8, "expected 64-bit i64");`.

## Not provided as intrinsics

Code intrinsics — runtime-instruction emitters like `trap`, `fence`,
`pause` — are not in the compiler-shipped set. They belong in stdlib as
functions with per-arch `asm` bodies. See [policy.md](policy.md).

## See also

- [comptime.md](comptime.md) — channel overview
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
- [variadics.md](variadics.md) — `$each a in va`, `va: ...`, `va.len`, `va...`
