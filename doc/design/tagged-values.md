# Mach v5 tagged values and explicit failure control

Accepted by the owner on 2026-09-09 for #3217, superseding the 2026-09-08
revision. This is the normative implementation contract for #3218 and #3219.
The earlier revision's flow-sensitive proof analysis, `try` expression and
compiler-known canonical types are withdrawn. The representation, reflection
and secrecy contracts are carried forward unchanged.

The language surface is five things: `tag` declarations with an explicit
discriminator, `Type.case{...}` construction, `value.case` payload places under
a lexical guard, `sel` as the case test, and `def` at function scope. None of
them is flow-sensitive.

## Types and declaration

`tag` uses the existing aggregate declaration and generic-parameter conventions
and declares its discriminator type explicitly. A case has a name and either one
explicitly typed payload or no payload. Multiple values use an ordinary record
payload. Empty tags, duplicate case names, and a discriminator type that cannot
represent every case ordinal are rejected. The discriminator type is one of
`u8`, `u16`, `u32` or `u64`.

```mach
tag Reply: u8 {
    empty;
    value: i64;
}

tag ParseError: u8 {
    invalid;
    overflow;
}
```

The canonical failure types are ordinary std tags. The compiler knows none of
their names. `res[T, E]` has `err: E` first and `ok: T` second. `err[E]` has
`err: E` first and payloadless `ok` second and is distinct from `opt[E]`.
`opt[T]` has payloadless `none` first and `some: T` second. Each has one fixed
generic arity. There is no defaulted type argument, general type inference,
dummy success type, unit value, constructor function or automatic error
conversion. These declarations replace std `Return` and `Option` and add `err`.

```mach
pub tag res[T, E]: u8 { err: E; ok: T; }
pub tag opt[T]: u8    { none; some: T; }
pub tag err[E]: u8    { err: E; ok; }
```

`def` is accepted at function scope with its existing form. A local alias is
visible from its declaration to the end of the enclosing block.

```mach
fun flush() err[WriteError] {
    def E: err[WriteError];
    ret E.ok{};
}
```

## Construction

A tag value is constructed by naming the type, the case and the payload in
literal braces. The payload is positional because a case has exactly one. A
payloadless case takes empty braces. There is no other construction form.

```mach
def R: res[i64, ParseError];

val empty: Reply = Reply.empty{};
val value: Reply = Reply.value{42};
val good: R = R.ok{42};
val bad: R = R.err{ParseError.invalid{}};
val present: opt[i64] = opt[i64].some{42};
val absent: opt[i64] = opt[i64].none{};
```

A payload on a payloadless case, a missing payload, more than one payload, and a
case selector used without braces are errors. `Reply.value` alone is not a
value. Payload types and generic arguments remain explicit under the existing
rules. Whole-value assignment replaces the selected case and payload together.

## Case tests

`sel place.case` is a boolean expression. It is true when `place` currently
holds `case`. `sel` reads only the discriminator, never a payload, and has no
side effects. Its operand is a place: a binding, a field, an index or a
dereference, followed by exactly one case name. A call or other temporary is
not a place. It is an ordinary `bool`, so it composes with `!`, `&&` and `||`,
can initialize a `bool` binding, and can be returned.

```mach
if (sel r.ok)  { ... }
or (sel r.err) { ... }

val done: bool = sel r.ok;
if (!sel next.some) { brk; }
if (sel a.ok && sel b.ok) { ... }
```

`sel` is a keyword. Existing identifiers named `sel` must be renamed before the
keyword is reserved. Comparing a tag with `==`, whole-tag equality, payload
equality, ordering, a `.kind` field, a `match` construct and a `try` construct
do not exist.

## Payload places and guards

`value.case` is a payload place. Reading it, writing it and taking its address
are legal only inside a guard for that place and case. A guard is a lexical
region, not a flow fact.

A chain arm whose condition is exactly `sel P.c` guards `P.c` inside its block.

A chain whose every arm exits guards the remainder of the enclosing block for
the cases the chain did not test. An arm exits when every reachable path
through it leaves by `ret`, or by `brk` or `cnt` targeting a loop that encloses
the chain. Arms of such a chain are conditions of the form `sel P.c` or
`!sel P.c` on one place `P`. A two-case tag tested on one case guards the other
for the rest of the block. A tag with more cases is guarded for the one case the
chain leaves untested, and for nothing when more than one case remains.

No other condition opens a guard. A compound condition such as
`sel r.ok && r.ok > 3` is rejected because `r.ok` is read outside a guard.
Nest instead. Allowing the left operand of `&&` to guard its right operand is
an additive later decision.

```mach
fun increment(input: str) res[i64, ParseError] {
    def R: res[i64, ParseError];
    val r: R = parse(input);
    if (sel r.err) { ret R.err{r.err}; }
    ret R.ok{r.ok + 1};
}

fun describe(e: LoadError) str {
    if (sel e.missing) { ret "no config"; }
    or (sel e.io)      { ret io_describe(e.io); }
    or (sel e.syntax)  { ret format_span(e.syntax); }
    ret "unreachable";
}
```

Inside a guard the payload place is ordinary storage. Reading it copies under
the existing value rules, writing it keeps the selected case, and `?value.case`
yields a typed pointer to naturally aligned storage. A nested payload place such
as `next.ok.some` requires a guard on `next.ok` before `sel next.ok.some` or
`next.ok.some` may appear.

```mach
if (sel r.ok) {
    val p: *u8 = ?r.ok.workers;
    if (@p > max) { @p = max; }
}
```

Within a guarded region, whole-value assignment to the guarded place is a
compile error. Rebind to a new name instead. Mutation through a pointer or by a
call is the programmer's obligation under the ordinary raw-memory rules, exactly
as for any pointer today. A payload read whose case is no longer selected is
undefined behavior of the same class as a stale pointer read. A raw pointer to
a payload does not pin a case or extend a lifetime. No borrow checker, proof
analysis or runtime validator is introduced. A debug-profile discriminator trap
on payload access is an open owner decision and not part of this contract.

Assignment evaluates and captures its RHS before evaluating its destination,
so a replacement initializer may read the old selected payload before
overwriting it. Historical #469/#494 chose LHS-first. The accepted v5 rule is
RHS-first and preserves the audited 4.30 behavior.

Guards obey ordinary scoping. A guard opened in a loop body is re-established on
every iteration by the chain that opens it. `fin` blocks and cleanup order are
unchanged; an exit inside a guarded block follows the existing `fin` rules.

## Initialization and representation

Ordinary zero initialization remains uniform. Case code zero selects the first
declared case and zero-initializes its payload. This applies to locals, globals,
omitted fields, arrays and comptime values. `opt` defaults to `none` and both
`res` and `err` default to their `err` case with a zero-initialized error
payload. Raw allocated capacity is not automatically a constructed value.

The discriminator is stored at offset zero in target byte order with the
declared type. One-case tags still have a discriminator. Codes follow
declaration order, with no custom codes, niche encoding, tag elision or case
reordering. Compiler resource and declaration limits remain checked
implementation limits.

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

Native ABI transfer preserves exact logical object extents separately from
carrier width and alignment. Whole-module targets represent logical cases and
payload types without invented physical registers. Layout reflection describes
the selected target's actual storage.

## Reflection and conversion

Add three intrinsics, following existing comptime conventions:

- `$is_tag(T)` identifies tag shapes, including the std `res`, `opt` and `err`
  declarations, which need no special treatment.
- `$cases(T)` enumerates owner-qualified descriptors in declaration order through
  the existing `$each` construct.
- `$discriminant_of(T)` produces the declared discriminator type.

Case descriptors expose `name`, `has_payload`, `type`, `offset` and `code`.
`name` is the existing comptime NUL-terminated string form. `has_payload` is a
comptime predicate. `type` retains declared qualifiers. `offset` and `code` use
the existing `u64` descriptor-integer convention. Accessing `type` or `offset`
on a payloadless case is an error.

```mach
$each case in $cases(T) {
    if (sel value.[case]) {
        $if (case.has_payload) {
            consume[case.type](value.[case]);
        }
    }
}
```

`sel value.[case]` is the test and `value.[case]` is the guarded payload place.
`T.[case]{payload}` and `T.[case]{}` construct through the same single-case
literal rule after specialization. A descriptor from another nominal type or
generic instantiation is rejected.

Reuse `$size_of`, `$align_of` and `$offset_of(T, payload_case)`. Layout answers
come from one checked target layout and become available under the same
complete-type rules as size/alignment, superseding the old lowering-only offset
exception. Unresolved or recursive layout is diagnosed rather than replaced with
a guess.

`$cases(^T)` is refused and `$is_tag(^T)` is false, matching current shape-query
conventions. Size, alignment and discriminator-type queries may inspect
outer-secret types because they expose storage metadata, never the active case.

Different nominal tags remain different types. Representation-changing `::` and
`:~` casts are rejected when either by-value representation contains a tag,
including through arrays, records and union alternatives. Same-type identity
casts remain identity. Transparent aliases preserve the same type. Ordinary
qualifier conversions retain their existing rules and must preserve the
underlying tag identity. They cannot introduce another route for
declassification.

Ordinary pointer retyping remains an explicit raw-memory operation under the
existing secrecy/weld rules. A typed tag read requires live, aligned storage,
a valid case code and a valid selected payload. Raw union projection cannot
manufacture that validity. Foreign formats must be validated in ordinary code
and then constructed. There is no implicit validator or runtime decoder.

## Secrecy, ownership and std

A public tag has a public discriminator. Each payload keeps its declared
secrecy. Outer `^Tag` also protects the active case, so `sel` on an
outer-secret tag is rejected. Secret-dependent `sel` results remain subject to
the constant-time rules.

Storage and transport preserve the union of potentially secret byte ranges
across all cases. A carrier containing both public discriminator bits and secret
payload bits cannot be marked wholly public. Extracting the public discriminator
must preserve its public provenance through late lowering. Public case access
does not permit reading an inactive secret payload or erasing a secret-welded
pointer. Copies with a secret selected case use the public fixed type extent and
do not choose a case-dependent access pattern.

`value:>T` removes only outer secrecy from the same underlying tag value. It
preserves the selected case and all qualifiers declared inside payloads.

Tags add no moves, destructors, unwinding, allocation or resource rollback.
Domain errors, formatting, explicit conversions and cleanup belong in std or
user code. Address-bound owners initialize their final caller-owned storage and
are not returned by value inside a result.

Public tag layouts and closed case sets are part of std's SemVer contract.
Changing a closed public case set is a breaking change. The migration ships as
std 2.0.0 paired with Mach 5.0.0.

## Implementation and acceptance

Additional acceptance examples use the types above and ordinary helper
functions.

```mach
var reply: Reply;                        # empty, because it is declared first
var result: res[i64, ParseError];        # err with ParseError.invalid
val invalid: Reply = Reply.empty{1};     # rejected, no payload exists
val missing: Reply = Reply.value{};      # rejected, payload is required
val wrong: Reply = Reply.value;          # rejected, selector is not a value
val bad: Reply = Reply{value: 1};        # rejected, record literal form

val n: i64 = r.ok;                       # rejected outside a guard
if (sel r.ok && r.ok > 3) { }            # rejected, compound condition guards nothing

if (sel r.err) { log(r.err); }
val m: i64 = r.ok;                       # rejected, the arm does not exit

for (more) {
    val item: opt[i64] = lookup(key);
    if (!sel item.some) { cnt; }
    consume(item.some);                  # guarded, cnt targets the enclosing loop
}

if (sel r.ok) {
    r = R.err{ParseError.invalid{}};     # rejected, whole-value assignment in a guard
}

fin {
    if (sel w.err) { ret err[WriteError].err{w.err}; }   # rejected, return crosses a fin boundary
}
```

For a raw input byte containing a case code, ordinary code can validate and
construct a value. An unrecognized byte does not become an invalid `Reply`.

```mach
if (code == 0) { ret opt[Reply].some{Reply.empty{}}; }
if (code == 1) { ret opt[Reply].some{Reply.value{decoded_number}}; }
ret opt[Reply].none{};
```

An equal-size raw-record-to-`Reply` cast is rejected with either `::` or `:~`.
Wrapping the two sides in arrays, records or union alternatives does not make
that conversion valid. A pointer retype is a raw operation and does not perform
the validation shown above.

For `tag SecretReply: u8 { failed: i32; value: ^u64; }`, testing the public
`failed` case with `sel` and reading its `i32` payload is permitted. Reading the
secret value as public `u64`, erasing its storage through a public byte pointer,
or applying `sel` to an outer-secret `^SecretReply` is rejected. A saved
public-payload pointer expires when the enclosing tag changes to another case.

The existing source bootstrap starts with published Mach 4.26.5, builds the
pinned bridge and audited compiler sources, and checks self-hosting convergence.
It does not require the withdrawn Mach 4.30.0 release. Published std 1.0.1
remains immutable. Implement the tag model, construction, `sel`, guards and
function-scope `def` in a compiler that still builds the current compiler and
std sources. Rename every existing `sel` identifier in both source trees before
that compiler reserves the keyword, because the self-host fixpoint compiles the
compiler's own source with the new lexer. Record a pinned usable compiler at that
migration boundary and update the existing bootstrap channel in both
repositories accordingly.

Acceptance covers construction, defaults, guard opening and closing, exiting
chains, guarded assignment rejection, generics and reflection, casts through
containing types, raw validity obligations, packed/overaligned storage, exact
ABI transport, public-case/secret-payload carriers and module emitters. Positive
and negative cases must agree across optimization levels. All retained targets
must implement the contract before v5 completes.
