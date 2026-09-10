# `tag`: tagged value

A `tag` is a discriminated aggregate value that represents exactly one active
case at any moment. Each case has a name and either one explicitly typed payload
or no payload at all.

## Implementation status

The features described on this page represent the accepted Mach v5 contract
specified in [the tagged value design](../design/tagged-values.md). Declarations
with an explicit discriminator, `Type.case{payload}` construction, `sel` case
tests, checked layout, `$is_tag` and `$discriminant_of`
are implemented. Lexical payload guards, `$cases` and the debug discriminator
trap remain in progress; until guards land, every payload access is rejected.

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

```mach
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

```mach
pub tag Entry: u8 {
    none;
    pair: rec { key: str; count: usize; };
}
```

## Construction and initialization

A tag value is constructed by naming the type, the case and the payload:

```mach
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

Whole-value assignment replaces the selected case and payload together.

### Default initialization

Zero initialization selects the first declared case and zero-initializes its
payload if one exists.

```mach
var reply: Reply;           # selects Reply.empty
```

## Canonical tag types

Mach provides three canonical tag types with fixed generic arities. They require
no imports and use the same underlying tag mechanisms as user-defined tags:

- `res[T, E]` represents either an error of type `E` or a successful value of type `T`. It has fixed arity 2. Its first declared case is `err: E` and its second case is `ok: T`.
- `opt[T]` represents either absence or a present value of type `T`. It has fixed arity 1. Its first declared case is payloadless `none` and its second case is `some: T`.
- `err[E]` represents either an error of type `E` or payloadless success. It has fixed arity 1. Its first declared case is `err: E` and its second case is payloadless `ok`.

The `err[E]` form is distinct from `opt[E]` and is not an alias. Mach has no
dummy success type, unit type, or general type argument inference.

```mach
val good: res[i64, ParseError] = res[i64, ParseError].ok{42};
val bad:  res[i64, ParseError] = res[i64, ParseError].err{ParseError.invalid{}};

val present: opt[i64] = opt[i64].some{42};
val absent:  opt[i64] = opt[i64].none{};

val finished: err[ParseError] = err[ParseError].ok{};
val failed:   err[ParseError] = err[ParseError].err{ParseError.overflow{}};
```

Default initialization of `opt[T]` selects `none`. Default initialization of
both `res[T, E]` and `err[E]` selects `err` with a zero-initialized error payload.

The case names `ok`, `err`, `some`, and `none` are contextual members within
their respective tags. They are not global keywords.

## Case tests

`sel place.case` is a boolean expression that is true when `place` currently
holds `case`. It reads only the discriminator, never a payload, and has no side
effects.

```mach
if (sel reply.value) {
    # reply holds value here
}
or {
    # reply holds empty here
}
```

The operand is a place: a binding, a field, an index or a dereference, followed
by exactly one case name of that place's tag type. A call or other temporary is
not a place. The result is an ordinary `bool`, so it composes with `!`, `&&` and
`||`, can initialize a `bool` binding, and can be returned.

```mach
val done: bool = sel reply.value;
if (!sel next.some) { brk; }
if (sel a.ok && sel b.ok) { }
```

`sel` is a keyword. Comparing a tag with `==`, whole-tag equality, payload
equality, ordering, a `.kind` field and a `match` construct do not exist. An
outer-secret `^Tag` protects the selected case as well as the payload, so `sel`
refuses one.

## Payload places and guards

`value.case` is a payload place. Reading it, writing it and taking its address
are legal only inside a guard for that place and case. A guard is a lexical
region, not a flow fact: a chain arm whose condition is exactly `sel P.c` guards
`P.c` inside its block, and a chain whose every arm exits guards the remainder
of the enclosing block for the case the chain left untested.

```mach
fun read_value(reply: Reply) i64 {
    if (sel reply.value) {
        ret reply.value;    # guarded by the arm condition
    }
    ret 0;
}
```

Inside a condition, the right operand of `&&` is guarded by a `sel P.c` that is
its left operand, because `&&` short-circuits. `||`, `!` and every other
operator open no guard.

Inside a guard the payload place is ordinary storage: reading it copies under
the existing value rules, writing it keeps the selected case, and `?value.case`
yields a typed pointer to naturally aligned storage. Whole-value assignment to
the guarded place is a compile error; rebind to a new name instead.

A payload read whose case is no longer selected is undefined behavior of the
same class as a stale pointer read. A raw pointer to a payload does not pin a
case or extend a lifetime. There is no borrow checker, proof analysis or runtime
validator. In the debug profile only, each guarded payload access compares the
discriminator and traps on mismatch.

## Failure handling

Ordinary user-declared tags acquire no automatic `try` conventions. Explicit
failure handling via `try` is reserved for the canonical types `res[T, E]`,
`opt[T]`, and `err[E]`. See [try.md](try.md) for details.

## Layout and representation

A tag is laid out with its discriminator at byte offset zero in target byte
order. The discriminator type is the smallest unsigned integer among `u8`,
`u16`, `u32`, and `u64` capable of representing every declared case ordinal.
Declaration order determines case codes, starting at zero. Single-case tags
still include a discriminator.

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
and `try` branches must obey the constant-time rules. Copies preserve potentially
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

## See also

- [try.md](try.md) - explicit failure handling with canonical tags
- [rec.md](rec.md) - records and struct layout
- [uni.md](uni.md) - raw unions
- [types.md](types.md) - primitive and compound type reference
- [comptime-intrinsics.md](comptime-intrinsics.md) - reflection intrinsics
