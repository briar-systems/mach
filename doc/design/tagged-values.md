# Mach v5 tagged values and explicit failure control

Accepted by the owner on 2026-09-08 for #3217. This is the normative implementation
contract for #3218 and #3219. The existing compiler does not implement these forms.

The owner approved the complete design and confirmed the one-argument `res[E]`
form after reviewing construction, direct case handling and `try` examples.
It represents an explicit payloadless success case and an error payload of type E.

## Types and construction

`tag` uses the existing aggregate declaration and generic-parameter conventions.
A case has a name and either one explicitly typed payload or no payload. Multiple
values use an ordinary record payload. Empty tags and duplicate case names are
rejected.

```mach
tag Reply {
    empty;
    value: i64;
}

tag ParseError {
    invalid;
    overflow;
}

val empty: Reply = Reply{empty};
val value: Reply = Reply{value: 42};
val good: res[i64, ParseError] = res[i64, ParseError]{ok: 42};
val bad: res[i64, ParseError] = res[i64, ParseError]{err: ParseError{invalid}};
val present: opt[i64] = opt[i64]{some: 42};
val absent: opt[i64] = opt[i64]{none};
```

Every tag literal selects exactly one case. A missing required payload, a payload
on a payloadless case, multiple selections and an empty literal are errors.
Payload types and generic arguments remain explicit under the existing rules.

`res[T, E]` is a canonical tag with `err: E` first and `ok: T` second.
`res[E]` has `err: E` first and payloadless `ok` second. This fixed one-argument
form does not introduce general type inference or a dummy success type.
`opt[T]` has payloadless `none` first and `some: T` second. These types work
without importing std and use the same tag implementation as user declarations.

```mach
tag WriteError {
    denied;
    native: i32;
}

fun flush() res[WriteError] {
    ret res[WriteError]{ok};
}
```

`ok`, `err`, `some` and `none` are contextual case members, not global keywords.
There is no `err` primitive and no `Void`, unit value, constructor function or
automatic error conversion.

## Tests, payload access and mutation

```mach
if (value == Reply.value) {
    val number: i64 = value.value;
}
or {
    # value is empty here
}
```

`Reply.value` is a case selector. It cannot be stored as a `Reply` value.
Comparing a tag with its own selector tests only its active case. `!=` negates
that test. This adds neither whole-tag equality nor payload equality or ordering.
There is no `.kind` field or `match` statement.

Payload access requires a current proof of the selected case. Construction and
ordinary branch flow establish proofs. Joining branches preserves only facts
true on every incoming path. Whole-value assignment changes the selected case
and invalidates earlier proofs. A payload assignment requires its case to be
selected and does not switch cases.

A write through a possibly overlapping mutable alias, or a call that can modify
the tested object, invalidates its proof. A separate immutable value snapshot
keeps its own proof. An immutable pointer binding does not make its pointee
immutable. Proofs do not depend on optimization level.

```mach
fun inspect(reply: *Reply) i64 {
    if (@reply == Reply.value) {
        replace(reply);
        ret reply.value;       # rejected because the call invalidated the proof
    }
    ret 0;
}

fun snapshot(reply: *Reply) i64 {
    val saved: Reply = @reply;
    if (saved == Reply.value) {
        replace(reply);
        ret saved.value;       # accepted because saved owns a separate value
    }
    ret 0;
}
```

Taking a payload address requires the same proof and a naturally aligned payload
place. The raw pointer remains valid only while the enclosing object lives and
that case remains selected. It does not pin a case or extend a lifetime.
Subsequent raw-pointer use carries this obligation under ordinary low-level
memory rules. No general borrow checker is introduced.

## Expression-level try

```mach
fun increment(input: str) res[i64, ParseError] {
    val number: i64 = try parse(input) or (error: ParseError) {
        ret res[i64, ParseError]{err: error};
    };
    ret res[i64, ParseError]{ok: number + 1};
}
```

`try` evaluates its operand once. Success extracts the payload. Failure binds
the explicitly typed error and executes the written failure block. That block
must exit and cannot initialize the destination or provide a fallback value.
Propagation and conversion remain ordinary visible code.

`try` accepts canonical `res` and `opt` values. Ordinary user-defined tags have
no implicit success/failure convention and use explicit case tests.

The operand is a prefix/postfix expression. A larger operand requires parentheses.
The `or` block is mandatory. Each independently fallible operand gets its own
`try`.

```mach
val total: i64 = (try left() or (error: ParseError) {
    ret res[i64, ParseError]{err: error};
}) + (try right() or (error: ParseError) {
    ret res[i64, ParseError]{err: error};
});

val number: i64 = try lookup(key) or {
    ret res[i64, ParseError]{err: ParseError{invalid}};
};

try flush() or (error: WriteError) {
    ret res[WriteError]{err: error};
};
```

Options have no error binding. Result failure bindings use the exact error type.
Payloadless result extraction produces no value and is allowed only as a direct
expression statement. It cannot be an initializer, argument or arithmetic operand.

Every reachable path through the failure block must leave it through an ordinary
valid `ret`, or `brk`/`cnt` targeting an enclosing loop outside the failure block.
A break from a loop created inside the failure block does not qualify. Calling a
function does not implicitly prove that control cannot return. Existing `fin`
restrictions and cleanup order apply unchanged to the actual exit path.

Call arguments evaluate left to right. Assignment evaluates and captures its RHS
before evaluating its destination. A failed extraction skips the remaining
expression and does not initialize its destination. A replacement initializer
may read the old selected payload before overwriting it.

The assignment rule preserves the current compiler and audited 4.30 behavior.
Historical #469/#494 chose LHS-first. The accepted v5 rule is RHS-first. This explicitly supersedes that earlier
assignment-order decision.

## Initialization and representation

Ordinary zero initialization remains uniform. Case code zero selects the first
declared case and zero-initializes its payload. This applies to locals, globals,
omitted fields, arrays and comptime values. Canonical option defaults to `none`
and result defaults to `err` with a zero-initialized error payload. Raw allocated
capacity is not automatically a constructed value.

The discriminator is stored at offset zero in target byte order. Its type is
the smallest of `u8`, `u16`, `u32` and `u64` that represents every case ordinal.
One-case tags still have a discriminator. Codes follow declaration order, with
no custom codes, niche encoding, tag elision or case reordering. Compiler resource
and declaration limits remain checked implementation limits.

For natural layout, let `D` be discriminator size, `M` the maximum payload size
and `PAlign` the maximum payload alignment, or one with no payloads. The common
payload offset is `align_up(D, PAlign)`. Object alignment is the maximum of the
discriminator alignment, payload alignment and any explicit alignment. Total
size is the aligned extent of that common payload area. Payloadless tags contain
only the discriminator and any required outer padding.

Existing `#[packed]` and `#[align(N)]` rules apply. Packing places the payload
immediately after the discriminator with base alignment one. Explicit alignment
raises object alignment and rounds total size. Packing does not repack a nested
payload. Packed value access uses legal unaligned operations. Taking a typed
payload pointer must not fabricate a stronger alignment guarantee.

Construction captures the payload before overwriting the destination. It zeroes
tag-owned gaps, inactive payload suffix and tail padding. The active payload
retains its ordinary value/representation rules, including any raw union bytes.
This does not promise recursive canonicalization of foreign payload padding.
Replacing a larger case clears its now-inactive suffix.

Native ABI transfer preserves exact logical object extents separately from carrier
width and alignment. Whole-module targets represent logical cases and payload
types without invented physical registers. Layout reflection describes the
selected target's actual storage.

## Reflection and conversion

Add three intrinsics, following existing comptime conventions:

- `$is_tag(T)` identifies public tag shapes, including `res` and `opt`.
- `$cases(T)` enumerates owner-qualified descriptors in declaration order through
  the existing `$each` construct.
- `$discriminant_of(T)` produces the actual unsigned discriminator type.

Case descriptors expose `name`, `has_payload`, `type`, `offset` and `code`.
`name` is the existing comptime NUL-terminated string form. `has_payload` is a
comptime predicate. `type` retains declared qualifiers. `offset` and `code` use
the existing `u64` descriptor-integer convention. Accessing `type` or `offset`
on a payloadless case is an error.

```mach
$each case in $cases(T) {
    if (value == T.[case]) {
        $if (case.has_payload) {
            consume[case.type](value.[case]);
        }
    }
}
```

`T.[case]` is the named selector and `value.[case]` is the proof-checked payload
projection. `T{[case]: payload}` and `T{[case]}` reconstruct through the same
single-case literal rule after specialization. A descriptor from another nominal
type or generic instantiation is rejected.

Reuse `$size_of`, `$align_of` and `$offset_of(T, payload_case)`. Layout answers
come from one checked target layout and become available under the same complete-
type rules as size/alignment, superseding the old lowering-only offset exception.
Unresolved or recursive layout is diagnosed rather than replaced with a guess.

`$cases(^T)` is refused and `$is_tag(^T)` is false, matching current shape-query
conventions. Size, alignment and discriminator-type queries may inspect outer-
secret types because they expose storage metadata, never the active case.

Different nominal tags remain different types. Representation-changing `::` and
`:~` casts are rejected when either by-value representation contains a tag,
including through arrays, records and union alternatives. Same-type identity
casts remain identity. Transparent aliases preserve the same type. Ordinary
qualifier conversions retain their existing rules and must preserve the underlying
tag identity. They cannot introduce another route for declassification.

Ordinary pointer retyping remains an explicit raw-memory operation under the
existing secrecy/weld rules. A typed tag read requires live, aligned storage,
a valid case code and a valid selected payload. Raw union projection cannot
manufacture that validity. Foreign formats must be validated in ordinary code
and then constructed. There is no implicit validator or runtime decoder.

## Secrecy, ownership and std

A public tag has a public discriminator. Each payload keeps its declared secrecy.
Outer `^Tag` also protects the active case. Secret-dependent case tests and `try`
branches remain subject to the constant-time rules.

Storage and transport preserve the union of potentially secret byte ranges across
all cases. A carrier containing both public discriminator bits and secret payload
bits cannot be marked wholly public. Extracting the public discriminator must
preserve its public provenance through late lowering. Public case access does not
permit reading an inactive secret payload or erasing a secret-welded pointer.
Copies with a secret selected case use the public fixed type extent and do not
choose a case-dependent access pattern.

`value:>T` removes only outer secrecy from the same underlying tag value. It
preserves the selected case and all qualifiers declared inside payloads.

Tags add no moves, destructors, unwinding, allocation or resource rollback.
Domain errors, formatting, explicit conversions and cleanup belong in std or user
code. Address-bound owners initialize their final caller-owned storage and are
not returned by value inside a result.

Public tag layouts and closed case sets are part of std's SemVer contract.
Changing a closed public case set is a breaking change. The migration ships as
std 2.0.0 paired with Mach 5.0.0.

## Implementation and acceptance

Additional acceptance examples use the types above and ordinary helper functions.

```mach
var reply: Reply;                       # empty, because it is declared first
var result: res[i64, ParseError];       # err with ParseError.invalid
val invalid: Reply = Reply{empty: 1};   # rejected, no payload exists
val missing: Reply = Reply{value};      # rejected, payload is required
val wrong: Reply = Reply.value;         # rejected, selector is not a value

consume(try parse(input) or (error: ParseError) {
    ret res[i64, ParseError]{err: error};
}, later());                           # later runs only after successful extraction

for (more) {
    val item: i64 = try lookup(key) or {
        cnt;
    };
    consume(item);
}

val fallback: i64 = try parse(input) or (error: ParseError) {
    log(error);
};                                    # rejected, failure can fall through

fin {
    try flush() or (error: WriteError) {
        ret res[WriteError]{err: error};
    };                                # rejected, return crosses a fin boundary
}
```

For a raw input byte containing a case code, ordinary code can validate and
construct a value. An unrecognized byte does not become an invalid `Reply`.

```mach
if (code == 0) {
    ret opt[Reply]{some: Reply{empty}};
}
if (code == 1) {
    ret opt[Reply]{some: Reply{value: decoded_number}};
}
ret opt[Reply]{none};
```

An equal-size raw-record-to-`Reply` cast is rejected with either `::` or `:~`.
Wrapping the two sides in arrays, records or union alternatives does not make
that conversion valid. A pointer retype is a raw operation and does not perform
the validation shown above.

For `tag SecretReply { failed: i32; value: ^u64; }`, testing a public
`SecretReply.failed` case and reading its `i32` payload is permitted. Reading
the secret value as public `u64`, erasing its storage through a public byte
pointer, or branching on an outer-secret `^SecretReply` is rejected. A saved
public-payload pointer expires when the enclosing tag changes to another case.

The existing source bootstrap starts with published Mach 4.26.5, builds the
pinned bridge and audited compiler sources, and checks self-hosting convergence.
It does not require the withdrawn Mach 4.30.0 release. Published std 1.0.1 remains
immutable. Implement the shared tag model and canonical types, then `try`, before
adopting the new forms in the compiler and std. Record a pinned usable compiler
at that migration boundary and update the existing bootstrap channel accordingly.

Acceptance covers construction, defaults, branch refinement and invalidation,
explicit failure exits and cleanup, generics and reflection, casts through
containing types, raw validity obligations, packed/overaligned storage, exact
ABI transport, public-case/secret-payload carriers and module emitters. Positive
and negative cases must agree across optimization levels. All retained targets
must implement the contract before v5 completes.
