# `tag`: tagged value

A `tag` is a discriminated aggregate value that represents exactly one active
case at any moment. Each case has a name and either one explicitly typed payload
or no payload at all.

## Implementation status

The features described on this page represent the accepted Mach v5 contract
specified in [the tagged value design](../design/tagged-values.md). At base
commit `fc5c9e7e`, tag parsing, declaration and constructor type checking, checked
layout, `$is_tag`, and documentation comment validation are implemented.
Canonical types, proof-based payload tracking, and code generation remain in
progress.

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

A tag literal names the type and initializes exactly one case in braces:

```mach
val empty_reply: Reply = Reply.empty{};
val num_reply:   Reply = Reply.value{42};
```

Brace initialization requires selecting exactly one case:

- Selecting no cases (`Reply{}`) is a compile error
- Selecting multiple cases (`Reply{empty, value: 42}`) is a compile error
- Omitting a payload on a payload-bearing case (`Reply.value{}`) is a compile error
- Supplying a payload to a payloadless case (`Reply.empty{1}`) is a compile error

Unlike vector literals such as `f32x4{1.0, 2.0, 3.0, 4.0}`, which require one
positional initializer for every vector lane, a tag literal specifies only the
single active case.

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

## Case selectors and testing

A case selector is written `TypeName.case`:

```mach
Reply.empty
Reply.value
```

A selector denotes a case identity, not a value. It cannot be stored in a
variable or passed as a function argument.

Comparing a tag value with a selector using `==` or `!=` tests which case is
currently active:

```mach
if (reply == Reply.value) {
    # reply has case value active here
}
or {
    # reply has case empty active here
}
```

Whole-tag equality (`a == b`) is a compile error, as is whole-tag inequality.
Selector comparisons test only the active case code and do not compare or order
payload values.

## Proof-required payload access

Reading a payload requires a current proof that the corresponding case is active.
The compiler tracks active proofs along control flow paths:

- Constructing a tag establishes proof of that case
- Testing `value == TagName.case` establishes proof inside the true branch
- Testing `value != TagName.case` excludes that case in the true branch and proves that case in the false branch
- Excluding a case proves another specific case only when it is the sole remaining possibility
- Branch joins retain only proofs that hold on every incoming path

```mach
fun read_value(reply: Reply) i64 {
    if (reply == Reply.value) {
        ret reply.value;    # valid because the branch proved reply is Reply.value
    }
    ret 0;
}
```

Attempting to read `reply.value` without an active proof is a compile error.

Assigning to a payload (`reply.value = 100`) requires an active proof of that
case and does not alter the selected case. Whole-value assignment changes the
active case and invalidates prior proofs.

## Mutation and alias invalidation

A write through a possibly overlapping mutable alias, or a call that can modify
the tested tag, invalidates active proofs for that tag.

```mach
fun inspect(reply: *Reply) i64 {
    if (@reply == Reply.value) {
        replace(reply);
        ret reply.value;    # rejected because the call invalidated the proof
    }
    ret 0;
}
```

An immutable value snapshot owns independent storage and preserves its proof:

```mach
fun snapshot(reply: *Reply) i64 {
    val saved: Reply = @reply;
    if (saved == Reply.value) {
        replace(reply);
        ret saved.value;    # accepted because saved is an independent value
    }
    ret 0;
}
```

Taking the address of a payload (`?value.case`) requires an active proof and a
naturally aligned payload place. The resulting raw pointer remains valid only
while the tag lives and retains that case. It does not pin the case or extend
object lifetime.

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
    if (value == T.[case]) {
        $if (case.has_payload) {
            consume[case.type](value.[case]);
        }
    }
}
```

`T.[case]` denotes the case selector, and `value.[case]` provides the
proof-checked payload projection.

## See also

- [try.md](try.md) - explicit failure handling with canonical tags
- [rec.md](rec.md) - records and struct layout
- [uni.md](uni.md) - raw unions
- [types.md](types.md) - primitive and compound type reference
- [comptime-intrinsics.md](comptime-intrinsics.md) - reflection intrinsics
