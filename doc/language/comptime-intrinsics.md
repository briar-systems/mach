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

`T` is a **type**, written with the ordinary type grammar — not just a bare
name. A generic instance, a pointer, an array, a `^` secret, and a qualified
`module.Type` are all valid, including inside the generic that owns the
parameter:

```mach
fun probe[T]() u64 {
    ret $size_of(Box[T]) + $align_of(Pair[T, u64]) + $size_of(*T) + $size_of([4]T);
}
```

`$offset_of`'s **second** argument is the exception: a bare field name, resolved
against the record's layout, never a type or a value.

### Where a layout intrinsic is constant

`$size_of` and `$align_of` fold in every **type** position, including ones resolved
before layout would otherwise be known — the measured type's layout is established
on demand when the measurement asks for it, so where the type is *declared* relative
to where it is measured makes no difference:

| position | `$size_of` / `$align_of` |
|---|---|
| `val` / `var` initializer | yes |
| global `align` | yes |
| record / union type `align` | yes |
| array length `[N]T` | yes |
| `$if` / `$or` condition | **no** |

```mach
rec Pair { a: u64; b: u64; }

#[align($align_of(Pair))]        # a type's alignment
rec Over { x: u8; }

rec Holder { buf: [$size_of(Pair)]u8; }   # an array length, inside a field type
```

`$offset_of` is the exception among the three: it folds in a value position but not
in a type one, because a field offset is settled during lowering rather than by the
front end.

A `$if` / `$or` condition is not a type position and does not get the on-demand
layout, so a layout intrinsic there is still rejected, and reports why.

**Inside a generic, a predicate is answered per instantiation.** `$is_record(T)`
in a `fun f[T]()` body is not decided against the template's placeholder — the gate
is deferred and re-folded once for each instantiation with `T` substituted, so
`f[SomeRecord]` and `f[u64]` take different arms from one template. A template that
is never instantiated has no instance to answer for and reports nothing.

**Cycles are refused, not resolved.** A measurement whose answer is one of its own
inputs — `#[align($size_of(Self))]`, or two types each aligned to the other's size —
is reported as a layout cycle naming the type that closes it. A pointer field does
not create one: it stores an address of fixed width, so `rec Node { next: *Node; }`
measures normally.

## Type predicates

Ask about a type's **shape** rather than its storage. Each takes one type operand
and folds in a `$if` / `$or` gate:

```mach
$is_record(T)           # T is a record (or an instance of one)
$is_union(T)            # T is a union  (or an instance of one)
$is_pointer(T)          # T is a reference: the raw `ptr` or a typed `*U`
```

They are **comptime-only** — a gate condition selects an arm, and there is no
runtime boolean for one to become, so using a predicate as a value is an error.

**`^` is a constructor, and a predicate answers about the outermost one.** `^Pair`
is a secret, not a record, so all three answer false and a reflection walk refuses
it instead of descending into secret storage. That is what keeps a predicate and
`$fields` in agreement: `$fields(^Pair)` refuses, so a gate that called `^Pair` a
record would send a walk into an operand the intrinsic then rejects.

Outermost means outermost. `^*u8` is a secret pointer and answers false; `*^u8` is
a **public** pointer to secret storage and is still a pointer, since the address is
public. A generic instance answers as the declaration it instantiates, so `Box[i64]`
is a record — which is the type a reflection loop actually meets — and `Box[^u64]`
is a record too, because the instance is not itself secret; its field is, and the
field is where a walk meets the question.

Because all three answer false for `^T`, "nothing classifies it" is itself a usable
signal: a walk that gates on the three predicates and refuses the fallthrough
refuses secrets without needing to ask about secrecy directly.

```mach
rec Inner { x: u64; y: u64; }
rec Outer { i: Inner; n: u64; }

$each f in $fields(Outer) {
    $if ($is_record(f.type)) {
        $each g in $fields(f.type) { ... }   # descend
    } $or { ... }                            # a scalar field
}
```

## `$pointee_of(T)` — descend through a reference

`$is_pointer` tells a walk that a field is a reference. `$pointee_of` says what it
refers to, which is what makes the reference traversable rather than merely
detectable:

```mach
$pointee_of(*U)         # U
$pointee_of(**U)        # *U — one level, not all of them
```

It is a type **constructor**, in the same family as `*`, `[N]` and `^`, not a call
that returns a value. So it is written wherever a type is written, including nested
inside another intrinsic's operand and inside a generic argument list:

```mach
rec Node { value: i64; next: *Inner; }

$each f in $fields(Node) {
    $if ($is_pointer(f.type)) {
        $each g in $fields($pointee_of(f.type)) {    # gate, then descend
            total = total + (@(n.[f])).[g];
        }
    }
    $or { total = total + n.[f]; }
}
```

Because `str` is `def str: *char`, a `str` field is a reference field, and
`$pointee_of(str)` is `u8` — which is what a formatter that renders a `str` field
needs.

**Everything that is not a typed reference is refused, and the refusal names what
it was handed.** A plausible wrong type here flows into a `$fields` walk that then
reports about the wrong record, so refusing is the only safe answer:

| operand | result |
|---|---|
| `*U` | `U` |
| `ptr` | refused — the raw pointer is untyped and carries no pointee |
| `^*U` | refused — a `^` secret is not a reference |
| anything else | refused, naming the type |

`^` is **not** stripped, which puts `$pointee_of` with the predicates rather than
with `$size_of`: `$is_pointer(^*U)` answers false, so descending through `^*U` would
put the intrinsic and the gate that guards it back into disagreement, and would hand
a walk secret storage the gate refused it.

**Following references does not terminate structurally.** The by-value walk `$fields`
supports does: a record cannot contain itself by value, so descending on `$is_record`
reaches a finite set of types. A reference graph has no such property —
`rec Grow[T] { p: *Grow[*T]; n: i64; }` is legal and has unboundedly many distinct
instances, and a walk that follows `p` generates `Grow[i64]`, `Grow[*i64]`,
`Grow[**i64]` without end. The compiler's generic-instantiation guard turns that into
a diagnostic naming the derivation chain rather than a hang, but that is a backstop,
not a termination story. A library that walks references owes its callers one of its
own, which is why `std.derive` refuses reference fields by default and offers
following as a separately named member.

## `$type_name(T)` — a type's spelling

```mach
$type_name(T)           # the type's spelling, as a NUL-terminated string
```

Unlike the predicates this **is** a value (`*u8`), usable anywhere one is. The
spelling is the same one diagnostics print, so a name a program reads and a name an
error reports cannot drift. Composites spell compositely (`$type_name(*Pair)` is
`"*Pair"`), and `^` spells too: `$type_name(^Pair)` is `"^Pair"`. Stripping it would
be a drift on the one qualifier where a drift matters most, since a diagnostic about
that type prints `^Pair`.

## Where `^` is stripped

One rule covers the whole surface: **`^` is stripped only where the question is
about storage.**

| asks about | strips `^` |
|---|---|
| `$size_of` / `$align_of` / `$offset_of` | yes — a secret occupies its base type's storage |
| `$is_record` / `$is_union` / `$is_pointer` | no — `^T` is a secret, not a `T` |
| `$pointee_of` | no — `^*U` is a secret, and is refused rather than followed |
| `$type_name` | no — the spelling is `^T` |
| `$fields` | no — a secret record is refused, not walked |
| type comparison (`f.type == u64`) | no — `^u64` is not `u64` |

## The type operand

`$size_of` / `$align_of` / `$offset_of` / `$fields` and the queries above all take a
**type** in argument 0, written with the ordinary type grammar — plus one extra
form: a field descriptor's `f.type` inside a `$each` body. `$pointee_of` is part of
that grammar rather than one of its consumers, so it composes with every one of
them (`$size_of($pointee_of(f.type))`, `$fields($pointee_of(f.type))`).

```mach
$each f in $fields(T) {
    val n: u64 = $size_of(f.type);      # the field's own size
    $each g in $fields(f.type) { ... }  # its own fields
}
```

The same form is valid in a **generic argument list**, which is what makes a walk
recursive rather than merely descending (#2691):

```mach
fun eq[T](a: *T, b: *T) bool {
    $each f in $fields(T) {
        $if ($is_record(f.type)) {
            if (!eq[f.type](?a.[f], ?b.[f])) { ret false; }   # re-enter at the field's type
        }
        $or { if (a.[f] != b.[f]) { ret false; } }
    }
    ret true;
}
```

Without it, `$fields(f.type)` gives one level of descent per `$each` someone wrote,
so a walk reaches only as deep as its author hand-unrolled. With it the walk is
written once and reaches any depth.

**Termination is structural and needs no depth limit.** Each descent instantiates at
a field's own type, a record's fields are finite, and a record cannot contain itself
by value — a self-reference must go through a pointer, which is a different type and
which `$is_record` does not select. Note that a walk which followed references would
not have this property: `rec Grow[T] { p: *Grow[*T]; }` is legal and has unboundedly
many distinct instances reachable through its pointer.

Inside the loop a field's type has no spelling, only the descriptor. A path that is
genuinely a qualified type name (`mod.Type`) still reads as one, and a wrong one
still reports against the type grammar — in the generic argument list exactly as in
an intrinsic operand. A name that is not a `$each` loop variable is reported where
it is written.

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

The operand is read at the **instance's** concrete type, in a plain generic as
much as in a pack-tailed one, and whether it is a parameter, a local, a field, or
a type derived from a generic parameter (`*T`). `$type_of(x) == T` against a
generic parameter compares the instance's argument on both sides, so it holds.

The provably-dead arms are **pruned** before type-checking, so each arm uses
`arg` at its own concrete type with no per-arm cast: the `str` arm above is
never checked against a `u64` element. Only the selected arm is type-checked
and emitted.

## Field intrinsic and projection

`$fields(T)` produces a comptime sequence of field descriptors for record type
`T`, written with the full type grammar exactly as the layout intrinsics take it
(`$fields(Box[T])`, `$fields(Pair[A, B])`, `$fields(mod.Rec)`). A union is
refused: its variants overlap in storage, so a member walk over them would report
distinct fields at distinct offsets that do not exist. Each descriptor carries
three readable properties:

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

## Not provided as intrinsics

Code intrinsics — runtime-instruction emitters like `trap`, `fence`,
`pause` — are not in the compiler-shipped set. They belong in stdlib as
functions with per-arch `asm` bodies. See [policy.md](policy.md).

**`$assert` is not an intrinsic either**, and is not planned as one: `$if` and
`$error` already compose to it exactly, so a dedicated directive would add spelling
without adding capability. Write the composition directly.

```mach
# instead of $assert(cond, "msg")
$if (!cond) { $error("msg"); }

$if (!($mach.build.arch == $mach.arch.x86_64)) { $error("expected x86_64"); }
```

The composition inherits `$if`'s condition rules, which is the point: the same
conditions fold there as in any other gate, and the ones that do not — a layout
intrinsic, a type query over an unbound generic parameter — refuse with their own
cause rather than through a second surface that could describe them differently.

## See also

- [comptime.md](comptime.md) — channel overview
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
- [variadics.md](variadics.md) — `$each a in va`, `va: ...`, `va.len`, `va...`
