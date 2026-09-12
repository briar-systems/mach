# `tag`: tagged value

A `tag` is a discriminated aggregate value that represents exactly one active
case at any moment. Each case has a name and either one explicitly typed payload
or no payload at all.

This page is the language reference for the
[v5 tagged-value contract](../design/tagged-values.md): declarations with an
explicit discriminator, `Type.case{payload}` construction, `sel` case tests,
lexical payload guards, the debug-profile discriminator trap, checked layout
and the reflection intrinsics.

## Grammar

```mach
tag NAME: u8 {
    case1;
    case2: type;
    ...
}

tag NAME[T, E]: u8 { ... }      # generic over type parameters
```

A case is declared with an identifier, an optional colon followed by a payload
type, and a terminating semicolon. Empty tags and duplicate case names are
rejected.

Tags use the standard aggregate declaration and generic parameter rules. Tag
declarations reject names that match SIMD vector spellings such as `f32x4`,
because vector spellings resolve as vector types in type positions.

## Examples

```mach accept
pub tag Reply: u8 {
    empty;
    value: i64;
}

pub tag ParseError: u8 {
    invalid;
    overflow;
}

pub tag Tree[T]: u8 {
    leaf: T;
    empty;
}
```

When multiple values must accompany a case, use an ordinary record payload:

```mach accept
use std.types.size.usize;
use std.types.string.str;

pub tag Entry: u8 {
    none;
    pair: rec { key: str; count: usize; };
}
```

An empty tag, a duplicate case name and a discriminator too narrow for the
case count are rejected:

```mach reject "duplicate tag case name"
tag Twice: u8 {
    one;
    one;
}
```

## Construction and initialization

A tag value is constructed by naming the type, the case and the payload:

```mach accept
tag Reply: u8 { empty; value: i64; }

val empty_reply: Reply = Reply.empty{};
val num_reply:   Reply = Reply.value{42};
```

The payload is positional because a case carries exactly one, and a payloadless
case takes empty braces. There is no other construction form:

- Omitting a payload on a payload-bearing case (`Reply.value{}`) is a compile error
- Supplying a payload to a payloadless case (`Reply.empty{1}`) is a compile error
- Supplying more than one payload (`Reply.value{1, 2}`) is a compile error
- Naming the payload (`Reply.value{value: 1}`) is a compile error
- The record-literal form (`Reply{value: 1}` or `Reply{empty}`) is a compile error
- A case selector alone (`Reply.value`) is not a value

```mach reject "payload"
tag Reply: u8 { empty; value: i64; }

val missing: Reply = Reply.value{};     # the case declares a payload
```

Whole-value assignment replaces the selected case and payload together.

### Default initialization

Zero initialization selects the first declared case and zero-initializes its
payload if one exists.

```mach run "empty"
use std.runtime;
use print: std.print;

tag Reply: u8 { empty; value: i64; }

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var reply: Reply;           # selects Reply.empty
    if (sel reply.empty) { print.println("empty"); }
    ret 0;
}
```

## The std failure tags

The canonical failure types are ordinary std tags, declared in
`std.types.canonical` with fixed generic arities and no compiler knowledge of
their names. They use the same mechanisms as every user tag: the same
construction form, `sel`, guards, layout and reflection. A module imports the
ones it spells:

```mach
use std.types.canonical.res;
use std.types.canonical.opt;
use std.types.canonical.err;
```

std declares them as:

```mach
pub tag res[T, E]: u8 { err: E; ok: T; }
pub tag opt[T]: u8    { none; some: T; }
pub tag err[E]: u8    { err: E; ok; }
```

- `res[T, E]` is either an error of type `E` or a value of type `T`; `err: E` is
  the first declared case and `ok: T` the second.
- `opt[T]` is either absence or a value of type `T`; payloadless `none` is
  first and `some: T` second.
- `err[E]` is either an error of type `E` or payloadless success; `err: E` is
  first and payloadless `ok` second. It is distinct from `opt[E]`, not an alias.

There is no defaulted type argument, general type inference, dummy success
type, unit value, constructor function or automatic error conversion.

```mach accept
use std.types.canonical.res;
use std.types.canonical.opt;
use std.types.canonical.err;

tag ParseError: u8 { invalid; overflow; }

val good: res[i64, ParseError] = res[i64, ParseError].ok{42};
val bad:  res[i64, ParseError] = res[i64, ParseError].err{ParseError.invalid{}};

val present: opt[i64] = opt[i64].some{42};
val absent:  opt[i64] = opt[i64].none{};

val finished: err[ParseError] = err[ParseError].ok{};
val failed:   err[ParseError] = err[ParseError].err{ParseError.overflow{}};
```

Default initialization of `opt[T]` selects `none`. Default initialization of
both `res[T, E]` and `err[E]` selects `err` with a zero-initialized error
payload. The case names `ok`, `err`, `some` and `none` are members of their
tags, not keywords.

Their layouts and closed case sets are part of std's SemVer contract; they ship
in std 2.0.0 paired with Mach 5.0.0. They are std declarations like any other:
the compiler has no knowledge of the three names, they resolve only through an
import or a declaration in scope, and a module may declare its own `res`, `opt`
or `err` as a tag or as anything else. Declaring one twice in a module is the
ordinary duplicate definition. A module that spells `res` without importing it
is rejected the way any unresolved type name is:

```mach reject "unresolved type name `res`"
tag ParseError: u8 { invalid; }

fun parse(x: i64) res[i64, ParseError] { ret res[i64, ParseError].ok{x}; }
```

## Case tests

`sel place.case` is a boolean expression that is true when `place` currently
holds `case`. It reads only the discriminator, never a payload, and has no side
effects.

```mach accept
tag Reply: u8 { empty; value: i64; }

fun describe(reply: Reply) i64 {
    if (sel reply.value) {
        ret reply.value;        # reply holds value here
    }
    or {
        ret 0;                  # reply holds empty here
    }
}
```

The operand is a place: a binding, a field, an index or a dereference, followed
by exactly one case name of that place's tag type. A pointer to a tag is a place
too: `sel p.case` with `p: *Reply` auto-dereferences the pointer once and tests
its pointee, exactly as the payload place `p.value` already reads through it. A
call or other temporary is not a place. The result is an ordinary `bool`, so it
composes with `!`, `&&` and `||`, can initialize a `bool` binding, and can be
returned.

```mach accept
use std.types.bool.bool;
use std.types.bool.false;
use std.types.canonical.opt;

tag Reply: u8 { empty; value: i64; }

fun tests(reply: Reply, next: opt[i64], a: opt[i64], b: opt[i64]) bool {
    val done: bool = sel reply.value;
    for {
        if (!sel next.some) { brk; }
        brk;
    }
    if (sel a.some && sel b.some) { ret done; }
    ret false;
}
```

`sel` takes a place, so a call result is refused:

```mach reject "sel"
tag Reply: u8 { empty; value: i64; }

fun make() Reply { ret Reply.empty{}; }

fun test() i64 {
    if (sel make().empty) { ret 1; }
    ret 0;
}
```

`sel` is a keyword. Comparing a tag with `==`, whole-tag equality, payload
equality, ordering, a `.kind` field and a `match` construct do not exist. An
outer-secret `^Tag` protects the selected case as well as the payload, so `sel`
refuses one.

`sel` also evaluates at comptime over a constant tag. A `$if (sel C.case)`
gate on a module `val` constructed with `C.case{...}` tests the constant's
selected case, guards `C.case` in its arm the same way a runtime chain arm
does, and its payload reads fold to the constant payload. A comptime `sel` on
anything that is not a constant tag element (a parameter, a local, a module
`var`, or a constant of another module) is rejected with a located diagnostic
at the condition, and a payload read of a case the constant does not hold is a
compile-time error rather than the runtime undefined behavior, because comptime
state is never stale.

## Payload places and guards

`value.case` is a payload place. Reading it, writing it and taking its address
are legal only inside a guard for that place and case. A guard is a lexical
region, not a flow fact: a chain arm whose condition is exactly `sel P.c` guards
`P.c` inside its block, and a chain whose every arm exits guards the remainder
of the enclosing block for the case the chain left untested.

```mach accept
tag Reply: u8 { empty; value: i64; }

fun read_value(reply: Reply) i64 {
    if (sel reply.value) {
        ret reply.value;    # guarded by the arm condition
    }
    ret 0;
}
```

A payload read outside a guard is a compile error:

```mach reject "guard"
tag Reply: u8 { empty; value: i64; }

fun read_value(reply: Reply) i64 {
    ret reply.value;        # no guard opens here
}
```

Inside a condition, the right operand of `&&` is guarded by a `sel P.c` that is
its left operand, because `&&` short-circuits. `||`, `!` and every other
operator open no guard.

A guard is matched by spelling, so a guarded place must be a path of
identifiers, fields, dereferences and indexes by a name or a literal, `sel`
rejects anything else, and reassigning an index binding inside the guard is not
tracked and leaves the debug-profile discriminator check as the only net.

Inside a guard the payload place is ordinary storage: reading it copies under
the existing value rules, writing it keeps the selected case, and `?value.case`
yields a typed pointer to naturally aligned storage. Whole-value assignment to
the guarded place is a compile error; rebind to a new name instead.

```mach reject "cannot assign to this place inside a guard"
tag Reply: u8 { empty; value: i64; }

fun reset(reply: Reply) i64 {
    var r: Reply = reply;
    if (sel r.value) {
        r = Reply.empty{};          # whole-value assignment inside the guard
        ret 1;
    }
    ret 0;
}
```

The check covers the guarded place and every object it is reached through by
value (`b.r = ...` under `sel b.r.value` is refused too). A pointer ends that
chain: under `sel p.value` with `p: *Reply`, reassigning `p` or writing
`@p = ...` is not tracked, because the guard covers the storage the pointer
reached when the test ran, and a write through a pointer is the ordinary
raw-memory obligation below.

A payload read whose case is no longer selected is undefined behavior of the
same class as a stale pointer read. A raw pointer to a payload does not pin a
case or extend a lifetime. There is no borrow checker, proof analysis or runtime
validator. In the debug profile only, each guarded payload access compares the
discriminator and traps on mismatch.

## Layout and representation

A tag is laid out with its discriminator at byte offset zero in target byte
order. The discriminator type is the one the declaration names after the tag
name (`tag Name: u8 { ... }`), one of `u8`, `u16`, `u32` or `u64`; a type too
narrow to number every declared case is rejected. Declaration order determines
case codes, starting at zero. Single-case tags still include a discriminator.

Let `D` be the discriminator size. Let `M` be the maximum payload size, and
`PAlign` the maximum payload alignment (or 1 when no cases have payloads). The
common payload offset is `align_up(D, PAlign)`. Total object alignment is the
maximum of discriminator alignment, payload alignment, and any explicit
`#[align(N)]` decorator. Total size is the aligned extent of the common payload
area. Tags with no payloads contain only the discriminator and trailing padding.

Packing with `#[packed]` places the payload immediately after the discriminator
at offset `D` with base alignment 1. Explicit alignment raises object alignment
and rounds total size without repacking nested payloads. Packed value access
uses legal unaligned operations. A typed payload pointer must not promise
stronger alignment than its storage provides.

Construction captures the active payload before overwriting the destination and
zeroes tag-owned gaps, inactive payload suffix bytes, and tail padding. Active
payload representation rules remain unchanged, including raw union bytes.
Replacing a larger case clears the bytes that become inactive.

Representation-changing `::` and `:~` casts are rejected when either by-value
representation contains a tag, including through records, arrays, or unions.
Same-type identity casts remain valid, and transparent aliases preserve type
identity. Ordinary pointer retyping remains an explicit raw-memory operation.
Reading through a typed tag pointer requires live, aligned storage with a valid
case code and a valid selected payload. A pointer cast does not validate raw data.

Stripping secrecy with `:>` removes only outer secrecy from the tag value and preserves the active case and inner
payload qualifiers.

## Secrecy and ownership

A public tag has a public discriminator, while each payload retains its declared
secrecy. Outer `^Tag` also protects the active case. Secret-dependent case tests
must obey the constant-time rules. Copies preserve potentially
secret storage across all cases and use the fixed public type extent, rather
than choosing a copy size from a secret active case.

An immutable pointer binding does not make its pointee immutable. Raw payload
pointers carry the lifetime and active-case obligations above. Tags introduce
no borrow checker, moves, destructors, allocation, unwinding, or automatic
resource rollback. Address-bound owners initialize caller-owned final storage
and are not returned by value inside results.

## Reflection

Tags support comptime reflection through three intrinsics:

- `$is_tag(T)` returns true for public tag shapes, including canonical tags
- `$cases(T)` yields a comptime sequence of case descriptors
- `$discriminant_of(T)` returns the unsigned integer type used for the discriminator

Case descriptors expose `name`, `has_payload`, `type`, `offset`, and `code`.
Accessing `type` or `offset` on a descriptor where `has_payload` is false is an
error.

```mach
$each case in $cases(T) {
    if (sel value.[case]) {
        $if (case.has_payload) {
            consume[case.type](value.[case]);
        }
    }
}
```

`sel value.[case]` is the case test, `value.[case]` is the guarded payload
place, and `T.[case]{payload}` constructs through the same single-case rule
after specialization.

## Deprecation

`#[deprecated]` and `#[deprecated("msg")]` apply to a `tag` declaration and,
inside the body, to a single case, where it is the only decorator a case
accepts. A deprecated case warns at every external construction, `sel` test
and payload place that names it; see [decorators.md](decorators.md).

## See also

- [decorators.md](decorators.md) - `#[deprecated]`, `#[packed]`, `#[align(N)]`
- [rec.md](rec.md) - records and struct layout
- [uni.md](uni.md) - raw unions
- [types.md](types.md) - primitive and compound type reference
- [comptime-intrinsics.md](comptime-intrinsics.md) - reflection intrinsics
