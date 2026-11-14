# `^` — the secret qualifier

`^T` marks a type as carrying secret data for Mach's constant-time guarantee.
Sema tracks secrecy as an information-flow discipline: secret data may move and
be stored, but it may never reach a position a classical leakage model observes
(a branch, a memory address, a variable-latency instruction). This page covers
the flow-typing rules and the `#[constant_time]` codegen contract. The grammar lives
in [grammar.md](grammar.md); the decorator reference is in
[decorators.md](decorators.md).

## Secrecy lattice

There are two secrecy levels in a two-point lattice: public is the bottom,
secret the top. `^` lifts a type to secret and binds to the type immediately to
its right, so it nests with `*` and `[N]` in any order: `^u32`, `*^u8`, `^*u8`,
`[N]^u8`, `^MyRec`. Doubling collapses: `^^T` is `^T`.

A public value coerces *up* to secret wherever a secret is expected, with no
syntax:

```mach
fun up(p: u32) ^u32 {
    ret p; # public u32 flows into a secret slot
}
```

A **literal is public by construction** and stays public through that coercion:
its value sits in the instruction stream, so classifying it as secret would
protect nothing. `var s: ^u32 = 5;` types `5` as `u32` and relies on the
up-coerce. The join below is unaffected — a value *computed* with a secret is
secret however its other operand is spelled — so `v << 3` on a secret `v` still
yields a secret, while the constant `3` is not mistaken for a secret shift
count.

The reverse never happens implicitly. The only downgrade is the explicit `:>T`
strip below.

## Join

Any operation with a secret operand yields a secret result. Taint joins across
arithmetic, bitwise, shift, and comparison operators, and through a value read
out of a secret container:

```mach
#[constant_time("hard")]
fun mix(a: ^u32, b: u32) ^u32 {
    ret a + b; # ^u32 + u32 -> ^u32
}

rec Key { d: ^[32]u8; }

fun first(k: Key) ^u8 {
    ret k.d[0]; # element of a secret array is ^u8
}
```

A window keeps the secret of what it reads. `x[start, n]` over a secret array
`^[N]T` is the secret array `^[n]T`, as a window over a secret vector is a
secret vector, so `$is_secret` gives a window the same answer it gives the
whole value. The checker and constant evaluation type a window with the same
rule.

A secret array `^[N]T` coerces to the array of secrets `[N]^T` with no syntax,
and does so at every array level (`^[N][M]T` to `[N]^[M]T` or `[N][M]^T`).
Each element stays secret, so nothing is declassified. The coercion only moves
the secret inward: `[N]^T` lifts to `^[N]T` only by the usual public-to-secret
lift, which needs it to fit `[N]T` and so never applies, and no coercion
reaches the public `[N]T`:

```mach
val K: ^[_]u8 = [2]u8{1, 2};

fun main() i32 {
    val q:    ^[2]u8 = K[0, 2]; # the window's own type
    val s:    [2]^u8 = K[0, 2]; # fine: every element stays secret
    val last: u8     = q[1]:>u8;
    val head: u8     = s[0]:>u8;
    ret last::i32 + head::i32 - 3;
}
```

Taking the address of a `^T` value with `?` gives the public pointer `*^T` (the
address is public, the pointee secret), and dereferencing it with `@` recovers
the secret `^T`.

## Gates

A secret may not reach a position the leakage model observes. Each is a compile
error decided by operand type:

- a secret branch or loop condition (`if`, `for`)
- a secret left operand of a short-circuiting `&&` / `||` (it is the branch the
  operator keys on; a secret right operand only taints the result)
- a secret memory index (`table[i]` with `i` secret)
- a secret memory address — an access through a secret *pointer*, whether by
  `@p`, `p[i]`, or the auto-deref in `p.x`. The index and the address are the
  two halves of one effective address, so both are gated. Only a secret
  *pointer* is an address: a `^[N]T` or `^Rec` is a secret value living at a
  public address, and a `*^T` is a public address to secret storage
- a secret operand of the always-variable-latency `/` or `%`

```mach error secret value used as a branch condition
fun leak(a: ^u32, t: *u8, p: ^*u8) u8 {
    if (a) { ret 1; } # error: secret value used as a branch condition
    ret t[a]; # error: secret value used as a memory index
    ret @p; # error: secret value used as a memory address
}
```

A secret operand of a **floating-point** operation is refused as well: its
latency is variable on every target.

Whether an integer **multiply** or a **variable shift** runs in time
independent of a secret operand is the target's question, and it is asked
inside a `#[constant_time]` function, where mirl reads the class masc gives
each instruction it selects under the timing modes the function's mode admits
(see [Constant-time multiply by instruction set](#constant-time-multiply-by-instruction-set)
and [Soft and hard](#soft-and-hard)). A secret operand of an instruction whose
latency depends on it there is refused, naming the function and the
instruction set's reason. `$mach.build.ct_mul(op, width)` reads mirl's answer
for a multiply at comptime (see `comptime-mach.md`). A secret 128-bit `/` or
`%` is refused like every secret division: the helper it would call is a loop
over the dividend's bits. Where a variable shift is admitted, the saturation
of a count at or above the operand width ([operators.md](operators.md#bitwise))
is a compare to an all-ones-or-zero mask and a mask on the count and the
value, with no branch, so it reveals nothing the shift itself would not.

A secret value passed to a variadic pack is also rejected, including a secret
wrapped inside an aggregate.

The gates hold in **every** function, whether or not it carries
`#[constant_time]`. Branching, indexing, addressing or dividing on a `^` value
needs a `:>T` downgrade first everywhere. Everything else is free everywhere: a
secret moves through storage, arithmetic and calls in any function, and only a
`#[constant_time]` function adds a guarantee about the code the compiler emits
for it.

The gates are checked against the types of the **instance**, not of the template.
A generic's body is re-checked per instantiation under its concrete type
arguments, and a pack-tailed function's body — the statements around its `$each`
as well as the unrolled body itself — is re-typed per monomorphized instance, so
a `T` that instantiates to a secret is gated exactly as the secret spelled out
in full would be.

## Asking about secrecy at comptime

Secrecy is asked about at comptime through two predicates, because a library has
two different questions and each needs its own answer:

- `$is_secret(T)` is the **per-field** question a reflection walk asks: is this
  type `^`-qualified at the outermost level?
- `$holds_secret(T)` is the **byte-class** question a memory primitive asks: is
  any byte of this type secret?

Both are type predicates like `$is_record` / `$is_union` / `$is_pointer`:
comptime-only, valid as a `$if` / `$or` gate condition, answered per
instantiation inside a generic. The full reference is in
[comptime-intrinsics.md](comptime-intrinsics.md).

### `$is_secret`: the question a walk asks

`$is_secret(T)` folds true when `T` is `^`-qualified **at the outermost level**,
and false otherwise.

It exists because secrecy was otherwise invisible to a library. Every other
predicate asks about the shape *under* the `^` and so answers false for every
secret, which makes `$is_record(^u64)` and `$is_record(u64)` the same answer — a
reflection walk could only ever meet a secret field as a fallthrough it had to
refuse. `$is_secret` is the positive question, and it is what lets `std.derive`
decide rather than refuse: a formatter redacts a secret field, a hash refuses one
(a data-dependent fold is a leak in the shape of a digest), and an equality picks
the constant-time comparison instead of the early-out whose timing *is* the
secret.

```mach fragment
rec Session { id: u64; key: ^[32]u8; }

$each f in $fields(Session) {
    $if ($is_secret(f.type)) { }            # redact: no read of `key` is emitted
    $or { render(s.[f]); }
}
```

**Outermost only**, the same line the rest of the family draws:

- `^*u8` is secret — the pointer *is* the secret, which is the shape the
  welded-storage rules exist for
- `*^u8` is **not** — a public address to secret storage. Ask
  `$is_secret($pointee_of(f.type))` for the pointee
- `[N]^u8` is not — a public array whose elements are secret
- a record with a secret field is not — the **field** is, and that is where a
  walk meets the question

### `$holds_secret`: the question a memory primitive asks

`$holds_secret(T)` folds true when any part of `T` is secret: `^u8`, `[32]^u8`,
`Session` above, and `Box[^u64]` all answer true, while `u64`, `[32]u8` and a
record of public fields answer false. It is the answer the compiler already
computes when it refuses to erase a typed pointer to the raw `ptr` (see
[Welded-storage pointers](#welded-storage-pointers)), one implementation rather
than two, so a gate on it and the cast check can never disagree. That walk
follows typed pointers, so `*^u8` holds a secret too: erasing a `**^u8` to `ptr`
is refused for the same reason.

That makes it the question a primitive like `std.memory.zero[T]` needs. A word
kernel over `ptr` is legal only for a type with no secret byte, a wholly secret
type goes to the constant-time kernel, and a type that mixes the two is handled
element by element:

```mach fragment
pub fun zero[T](p: *T, count: usize) {
    $if (!$holds_secret(T)) { raw_zero(p::ptr, $size_of(T) * count); }
    $or ($is_secret(T))     { ct.zeroize(p::*^u8, $size_of(T) * count); }
    $or                     { zero_typed[T](p, count); }
}
```

### Why a walk must not ask the deep question

A walk that gated on `$holds_secret` instead of `$is_secret` would be wrong in
the common case. A formatter walking a record whose one field is secret would see
`$holds_secret` answer true for the record and redact all of it, public `id`
included, and it still could not say which field was the secret. The per-field
question is the one a walk actually has, and `$is_secret(f.type)` is exactly it.
The deep question belongs to code that treats a type as bytes, where one secret
byte decides how every byte is moved.

Note that a walk which skips a secret field is pinned by the flow rules rather
than by convention — reading one into a public accumulator does not compile, so a
walk that gates wrongly is a compile error, not a silent disclosure.

## Downgrade with `:>T`

`:>T` is the only way to remove `^`. It produces a new public value and never
reinterprets storage in place, and it always names the public type it lands on:

```mach
fun publish(a: ^u32) u32 {
    ret a:>u32;
}

fun publish2(a: ^*u8) *u8 {
    ret a:>*u8;
}
```

`:^` is no operator; the parse stops at the colon:

```mach error expected ';' after 'ret'
fun publish(a: ^u32) u32 { ret a:^u32; }
```

`:>T` peels exactly the outer qualifier, so it can never launder a welded pointee
(`*^T` stays `*^T`). `::` and `:~` may add `^` at a value position but never drop it. Promotion
needs no operator of its own: a public value coerces up to secret implicitly,
and a cast to a secret type (`x::^u16`) is an ordinary cast. The same holds for
a scalar, for a scalar field of a by-value aggregate, and for a pointer value,
whose address becomes secret. What a pointer reaches stays exact at every depth,
so `*u8` does not cast to `*^u8`: the target is shared storage, and a secret
written through the new alias would be read back through the old public one.

`:>T` is the only declassification and there is no untyped form: `:^` is not
an operator, so `x:^` and `x:^T` are parse errors at the colon. Inside a
generic body the operand may be typed by a
parameter (`fun show[U](s: U) u32 { ret s:>u32; }`): the template cannot decide
whether `u32` is `U` stripped, so it checks only that the target is public and
each instance settles the equality under its concrete arguments, the way every
other secrecy gate is checked against the instance.

## Secrets have no compile-time value

A module constant whose type holds a secret (`val K: ^u64 = 7;`, a `^[N]u8`
array, a record or tag with a secret field) is a run-time value like any other
secret. It has no compile-time value: wherever the compiler asks for one, a
read of it is refused as `secret.comptime`. That covers every comptime
argument (`take(K)` for `fun take($c: u64)`), an array, window or field length
(`[K]u8`, `a[0, K]`), `$size_of`, `$align_of` and `$length_of` of a type that
spells one, a `$if` gate in a function or at module scope, a `$each` over a
secret constant array, a decorator argument (`#[align(K)]`), and a public field
or element read of a secret constant (`K[0]`). A literal is public, and a public
field of a constant that holds a secret elsewhere keeps its compile-time value
(`C.count` of `R{count: 4, key: 9}`).

`:>` of a constant is itself compile-time evaluable, so an author who means the
value writes the declassification where it is used, as anywhere else:

```mach
val K: ^u64 = 7;

fun take($c: u64) u64 {
    ret c;
}

fun main_value() u64 {
    ret take(K:>u64); # a compile-time 7, declassified here
}
```

Inside a `#[constant_time]` function that cast is listed by
`mach check --declassified` like any other. A constant's own initializer may
read secret constants when its declared type holds a secret
(`val K2: ^u64 = K + 1;`), and the value it binds is secret in turn. A comptime
parameter declared with a type that holds a secret (`$c: ^u64`) is a
contradiction, and its declaration is refused. Mach has no explicit tag
discriminator values and no field defaults, so neither can name a secret.
`#[embed]` cannot produce a secret array: the embedded bytes are public by
type (`[_]u8`), so a key compiled into an image is embedded public and copied
into secret storage, `val` to `var` of a `^` type, where it is used.

## Welded-storage pointers

Secrecy is fixed at declaration and is non-launderable, which makes the
public/secret aliasing leak unconstructable with no alias analysis:

- a `^T` is stored only through a `*^T`, never a `*T` (the lattice forbids the
  downgrade)
- a secret-welded pointer cannot be erased to the untyped `ptr`
- a `uni`'s overlapping variants must agree on secrecy
- a `tag`'s cases share a byte only where they agree on its class, by the same
  comparison, so a tag whose payloads disagree lays them out in separate areas
  (it may be larger than one area would be; see
  [tag.md](tag.md#layout-and-representation))

```mach error union variants must agree on secrecy
fun erase(p: *^u8) ptr {
    ret p; # error: type mismatch: expected ptr, found *^u8
}

uni Bad { a: ^u32; b: u32; } # error: variants disagree on secrecy
```

The union rule is a property of the union **type**, not of the syntax that
declared it, so it holds for an inline `uni { ... }` with no declaration to hang
a check on, and at every **instance** of a generic union. At the declaration a
variant typed by a generic parameter says nothing about secrecy, so `uni U[T] {
a: T; b: u32; }` agrees there and is decided where each instance is formed:

```mach error this instantiation makes overlapping fields part secret
uni U[T]   { a: T; b: u32; }
rec Box[T] { u: U[T]; }

var s: U[^u32]; # error: this instantiation makes the variants disagree
var b: Box[^u32]; # same error: the instance need not be spelled
var p: U[u32]; # fine, and so is an all-secret instantiation
```

A *partially* concrete template (`U[T, ^u32]` written inside another generic) is
a legal annotation with both legal and illegal instantiations, so it is never
rejected where it is written — only at the arguments that actually make a pair
mixed.

The check is **deep** and **fails closed**: a secret nested anywhere inside an
aggregate (including a generic instance's lazily-materialized fields) counts as
secret at these boundaries, and a placement the checker cannot prove severs no
weld — a secrecy difference reachable through a function type, or a shape whose
layout it cannot determine — is rejected rather than allowed.

Types are compared **by byte extent**, not by field ordinal. Each type lays out
as a run of bytes, each byte secret or public, with padding counted as public
and a pointer counted by what it reaches. A `::` or `:~` from `*S` to `*T`
retypes the storage `S` covers, and a `*S` may address a run of `S` values, so
the rule is directional:

1. every byte both `S` and `T` cover has the same class on both sides
2. a narrowing (`T` no larger than `S`) is accepted
3. a widening is accepted only when the extra bytes cannot change class: every
   byte of `S` has one class and every byte of `T` has that same class
4. beneath a pointer field (a pointer to a pointer, a union variant of pointer
   type) the target is shared storage, so the relation holds both ways: equal
   extent and the same class at every byte

A by-value cast between aggregates is the one place a byte may turn from public
to secret, and a pointer field keeps its target exact even there.

A generic whose argument grows through a pointer, such as `rec S[T] { v: ^u32;
next: *S[*T]; }`, has a fresh instance below every pointer, so the comparison
cannot walk it there. Below a pointer, two instances of one such generic are
compared by their arguments instead. The declarations place each parameter:
**laid** when its argument's bytes sit inside the storage beside other fields,
the whole **target** of a pointer, or **nowhere** when it only flows into
parameters that are themselves placed nowhere. A laid pair of arguments must have
one size, one alignment and the same class at every byte, a target pair must match
both ways, and an argument placed nowhere is not compared at all. A
disagreement names the field it lies in.

```mach
rec S[T] { v: ^u32; next: *S[*T]; }

# fine: `T` reaches no byte, so the two agree at every depth
fun view(p: *S[u32]) *S[u64] {
    ret p::*S[u64];
}
```

Two laid arguments that differ in size or alignment are refused as unprovable,
under `secret.variant_unprovable`, because padding can line the two layouts up at
one depth and not the next. So is a pointer from such an instance into storage of
another shape, since the two sides never meet at an instance they could compare.

```mach error cannot prove that the two types agree on secrecy
rec W[T] { pad: u32; x: T; }
rec S[T] { s: ^u64; v: T; next: *S[W[T]]; }
rec R    { a: u32; b: u32; }

# refused: below the first pointer `v` holds `W[u64]` on one side and `W[R]` on the other
fun view(p: *S[u64]) *S[R] {
    ret p::*S[R];
}
```

The rule is the same for scalars and aggregates, and the variants of a union
overlay its own storage from offset 0, so every pair of variants agrees on each
byte both cover. A widening past a secret is refused because the extra bytes
may be the next element of a run:

```mach error cannot add or drop the secret qualifier
rec Key { x: ^u32; }

rec Wide { x: ^u32; y: u32; }

# fine: one secret class throughout, as a run of `^u8` is
fun words(p: *^u8) *^u64 {
    ret p::*^u64;
}

# fine: a narrowing keeps the class of every byte it still covers
fun head(p: *Wide) *Key {
    ret p::*Key;
}

# refused: `y` of the first element is the next element's secret `x`
fun widen(p: *Key) *Wide {
    ret p::*Wide;
}
```

A type that stores no value (an empty `rec`, a `[0]T`, or an aggregate of only
those) has zero extent and so no class: anything narrows to it, and it widens
to nothing that stores a value. The rule speaks only for the bytes a cast's
types cover. Indexing past them, as `(?q.a)[1]` does or as indexing a narrowed
`*^u8` past its first byte does, is an out-of-bounds access, and no cast rule
closes it (see [Assurance](#assurance)).

The comparison reads the same layout the backend emits. Where it cannot
determine a layout it declines, which rejects.

## Comparing and ordering addresses

`==`, `!=`, `<`, `<=`, `>` and `>=` all accept two pointer-like operands
whatever their pointees' secrecy, including `*^T` against `*^T` and `*^T`
against a public `*U`. The result is a public `bool` — `u8`, per
[operators.md](operators.md) — so it branches, feeds `&&`, and returns like any
other public value:

```mach
use std.types.bool.bool;
use std.types.size.usize;

fun overlap(first: *^u8, first_len: usize, second: *^u8, second_len: usize) bool {
    ret first <= ?second[second_len - 1] && second <= ?first[first_len - 1];
}
```

**This launders nothing**, and the reason is the line the whole model draws: a
`*^T` is a *public address to secret storage*. The address was never secret, so
reading it is not a downgrade. What the welding rules protect is the *pointee* —
that the bytes at that address can only ever be reached as `^T` — and ordering
reaches no bytes at all. It consumes two addresses and produces one `bool`; no
integer and no pointer comes out of it, so there is no value to turn back into a
`*T` and dereference. Ordering is exactly as revealing as the `==` the language
has always permitted, which answers the same question one address at a time.

The conversions stay closed. None of these become legal by admitting ordering,
and each has a test pinning it:

| refused | why |
| --- | --- |
| `p::usize`, `p:~usize` | `::` and `:~` may not drop `^` |
| `p::*^u8` from `p: *u8` | adding `^` to a pointer's target lets a secret be written through the alias and read back public |
| `p:>*u8` | a `:>T` target must name the operand's *stripped* type, and `*^u8` stripped is `*^u8` |
| `@((?p):~*usize)` | the same, reached through a pointer to the slot |
| `val e: ptr = p;` | a secret-welded pointer does not erase to `ptr` |

Three deliberate decisions:

- **A differing pointee type orders.** `*^u8 < *u32` compiles. Addresses are
  addresses; both operands are pointer-width and the comparison is over the
  address space, not over either pointee. This is the pair `==` already accepts,
  and narrowing it for ordering alone would refuse the common case of relating a
  byte cursor to a typed buffer.
- **`nil` orders.** `p > nil` compiles and compares against the null address as
  zero. It falls out of `nil` being a pointer-like operand, the same way
  `p == nil` does. It is rarely what you mean — `p != nil` is — but it is not a
  leak, and special-casing it would be surface with nothing behind it.
- **The comparison is not itself constant-time-gated, and does not need to be.**
  Both operands are public addresses, so ordering is `CT_OP_NONE`: it lowers to
  one unsigned integer compare of pointer width, data-independent on every
  supported target. There is no secret operand for the leakage model to track.

Inside a `#[constant_time]` function this needs no exception. The result of ordering
two public addresses is public, so it may steer a branch there like any other
public condition. The gates are unchanged where the *pointer itself* is the
secret: a `^*T` is a secret value, the order of two of them is secret, and that
`bool` still cannot be a branch condition.

```mach error secret value used as a branch condition
use std.types.bool.bool;

#[constant_time("hard")]
fun before(a: *^u8, b: *^u8) bool {
    ret a < b; # public addresses, public bool
}

fun leak(a: ^*u8, b: ^*u8) u8 {
    if (a < b) { ret 1; } # error: secret value used as a branch condition
    ret 0;
}
```

## `#[constant_time]` — the codegen contract

The flow typing constrains the *source*. `#[constant_time(mode)]` carries the
obligation through *codegen*, and its mode, `"soft"` or `"hard"`, is required:
a bare `#[constant_time]` is refused. Inside a function carrying it, in either
mode:

- the code-level rules apply to the emitted code: no secret branch or loop
  condition, short-circuit operand, index, address or division;
- a callee that receives a secret must itself be declared constant time, as
  [Calls](#calls) states;
- masc's walk proves all of this on the encoded code, so a secret the
  compiler's own passes route to a branch, an address or a variable-latency
  instruction is refused, never emitted.

The two modes differ only in how the processor's data-independent-timing mode is
held, which [Soft and hard](#soft-and-hard) describes.

Inline `asm` inside such a function is **validated**, not rejected. The block is
encoded with the rest of the function and masc's walk reads its bytes as it reads
every other instruction, in one pass over the function's code, so nothing about a
block is taken on trust from its text: a data directive is walked as the
instructions its bytes decode to. The walk holds the same state through a block
as anywhere else.

- **Registers.** Every register is seeded at the function's entry: an argument
  register as the declaration says, a register the convention holds public at
  every entry (the stack pointer, the timing mode's register, a reserved one no
  instruction adds to, as the thread pointer) public, and every other one, the
  frame pointer and the flags a caller's arithmetic accrued included, as possibly
  holding the caller's secret. A block that leaves a secret in a register held
  public at entry is refused there. The walk holds a register's secrecy whole,
  so a register or argument slot that carries a secret member of a record
  passed by value together with a public one is secret whole, and a branch on
  that public member is refused, naming the parameter and the bytes that share
  the slot. Take such a record by reference, or order its members so secret
  and public ones fall in different 8-byte pieces. A record holding a secret
  that the convention itself passes by reference, as aarch64 and riscv64 pass
  one larger than 16 bytes and windows one larger than 8, is refused the same
  way when a public member decides a branch.
- **Flags.** Each instruction reads and writes each flag bit as masc's tables
  state it, so a branch on flags a secret compare filled is refused, and an
  instruction that writes only some of them, as `inc` leaves carry, leaves the
  rest as they were.
- **Timing.** An instruction whose timing depends on an input under the target's
  timing mode is refused when that input may hold a secret, by the target's own
  list: Intel's data operand independent timing instructions on x86-64, the
  instructions Arm marks data-independent-time on aarch64, Zkt's on riscv64.
  Every grammar parses divides, multiplies and float instructions, and the list
  decides each.
- **Memory.** The walk places each access as an extent off a register's value or
  a symbol, follows the frame exactly where the producer states a slot's address
  never leaves the code, and keeps what each extent may hold. A secret spilled and
  reloaded comes back secret. A store of a secret is admitted only into memory
  something states may hold one: a frame slot the walk places, a global whose type
  holds a secret, or memory reached through a pointer the block loaded from a
  `{name}` binding whose pointee is secret in every byte. A binding whose pointee
  holds no secret states its memory public, and every other destination is memory
  nothing states the secrecy of. A secret stored to either is refused
  ([asm.md](asm.md) has the binding rules and an example).
- **The timing mode.** A block may not turn the timing mode off: aarch64's
  `msr dit` from a register and `msr pstate.dit, #0` are refused whatever encodes
  them. Only the read, set and restore the frame code places are admitted.

What the walk cannot read, it refuses: bytes that decode to no instruction, an
instruction whose effects masc's tables leave undecided, and control that leaves
the block where the walk cannot follow it.

`{name}` bindings carry the local's declared type into the walk. A pointer to a
secret (`*^u32`) is a public address and a secret load: the register it is
staged into may address memory, and what a load through it produces is secret,
while a secret pointer (`^*u32`) is a secret address.

On a typed stream (wasm) the producer states no store, so the store rule above
does not apply there, and a stream's walk takes a secret stored at an address it
does not place as possibly anywhere.

### Calls

`#[constant_time]` is a **per-function** contract, and a call is where it meets
another function. A call trusts the callee's declaration and never reads its
body: the callee's mode and which bytes of each of its parameters and of its
result are secret, all read from its declaration, are all the caller's check
knows of it. The
callee's own build holds its body to that declaration, wherever the callee is
compiled, so a definition that breaks it fails its own module and the program
never links with it. This works the same within a module and across modules, a
generic instance included.

A constant-time function may pass a secret only to a callee that

- it names directly, since a function type states no mode: a secret passed
  through a function value is refused, whatever the value points at;
- is declared `#[constant_time("soft")]` or `#[constant_time("hard")]`, a generic
  included, whose instances take its mode. An unmarked function is refused
  whether it is generic or not, in the same module or another: mark it;
- is not `#[weak]`, since a definition elsewhere may replace its body at link
  time with code nothing here checked. An `ext` function never qualifies, as
  `#[constant_time]` on an `ext` declaration is refused;
- is in a mode the caller may rely on: a soft caller may call either mode,
  and a hard caller a hard callee, or a soft one where the build guarantees
  every timing mode a soft function's code may be held under (see
  [Soft and hard](#soft-and-hard)), which is every x86-64 build, an aarch64
  build with `dit` and a riscv64 build with `zkt`.

"Passing a secret" reaches through pointers: an argument whose type holds a
secret anywhere it reaches, as `*^u8` or a record with a `^` field, as
`$holds_secret` answers, counts, and the refusal is `secret.call_untrusted`,
naming the type. masc's walk checks the same at the machine level from the
callee's declaration and the calling convention: a secret in a register or
stack slot the callee's declaration says is public is refused, and so is any
secret a call to a callee it does not trust could reach, in a register, an
argument on the stack or memory the callee could reach through a pointer,
naming the callee. So a call to a callee that is not constant time, even one
passing only public values, is refused while a secret sits in memory the callee
could reach.

A trusted callee may read any memory a pointer it is given reaches. What makes
that safe is the rule its own walk holds it to: every read outside its own
frame and its module's globals takes what it reads as the type it reads through
says, secret through a pointer to a secret and public through a pointer to
public storage, which welded storage keeps free of secrets, and as secret where
the compiler states nothing, as inside an `asm` block. So a callee never takes
a secret its caller holds as public, and the caller takes everything the callee
may have written as possibly secret after the call.

A trusted callee's result is what its declaration says it is. The bytes of the
result its type declares public are public in the caller, so a constant-time
caller may branch on a public status a trusted callee returns even when it
passed that callee a secret, and the bytes it declares secret are secret. The
callee's own build holds it to that at every return: no secret is left in a
part of the result its type declares public. A value declassified with `:>T`
is public, so a constant-time function may return `x:>u64` as a public `u64`
([Declassification](#declassification)). A result returned through memory, as a
large record is, is held to the same contract: the callee's build holds that it
leaves no secret in a byte its type declares public, and the caller takes those
bytes as holding what they held before the call.

A callee that a shared library exports stays the definition its callers in the
library were checked against: an exported constant-time function that is not
`#[weak]` takes protected visibility on ELF, so no other image preempts it. A
PE DLL reaches its own exports directly, and on Mach-O only a weak definition
is coalesced across images, so neither needs it.

A call that passes no secret and reaches none needs nothing of its callee, so a
constant-time function may call any function with public values while no
secret sits where that callee could reach.

The walk reads the callee's code as the image links it, so a callee another image
could replace when it loads would void the check. In a shared library no
definition is seen from another image unless it is `#[export]`, and an exported
constant-time function that is not `#[weak]` is protected on ELF, as PE and
Mach-O libraries bind their calls to their own exports at link. A call inside the
image therefore runs the code the walk read. A `#[weak]` default is never
trusted, since another definition of its name may replace it.

The zeroizing-write guarantee is separate and broader; it is described below.

### Declassification

`:>T` is the declassification everywhere, a constant-time function included. A
value that went through `:>T` is public for every purpose there: it may decide
a branch or a loop, form an index, an address or a length, be stored into
public memory, be returned in a public result and be passed where a callee
declares a public parameter. The promise of `#[constant_time]` is that nothing
secret that was not explicitly declassified decides timing or reaches public
storage or a public result.

What stays refused is everything else: a secret that did not go through `:>T`
reaching a branch, an index, a variable-latency instruction, public memory, a
public result or a public argument. A value declassified in part is public only
in that part: a declassified field leaves the record's other fields as secret
as they were, and the low half of a wide value declassified leaves the high
half secret. A declassify of a pointer makes the address public and nothing it
points to (`^*^u8` to `*^u8`). A record or array secret whole (`^R`) declassified
whole is public whole. The value a declassify copies from keeps its secrecy, so
a later use of the source is checked as the secret it is.

The checker cannot judge what the author declared public. A parser that
declassifies each byte of a secret key file to decide whether it is a padding
character or part of the alphabet branches on a fact of every secret byte, and
the build accepts it, since the author said that fact is public. Each `:>T` in
a constant-time function is a claim the author makes and a reviewer checks.

`mach check --declassified` lists those claims for the reviewer. Once the
project checks, it writes every `:>T` of the project's own modules to stdout,
one a line, as `file:line:column: function: type`: the function the cast
stands in, `test <name>` for a test and `<module>` outside both, and the type of
the value declassified as sema typed it. A cast in the body of a generic is
listed once, at its place in the source, whatever instances the module makes:
its type as the body spells it, `T` for a type parameter, and `<each
expansion's>` for an operand no frame of the build types: one only a `$each`
expansion types, and one in a `$if` or `$or` arm the build does not select,
which is listed though the build compiles none of it. It lists what the source states, so a
declassification the compiler makes itself, as the test of a division's
divisor for zero, is not listed. The flag belongs to `check` alone. The listing
stays plain lines on stdout under `--diagnostics json`, which turns only stderr
into records ([readout.md](readout.md#streams)).

A secret-taint bit is threaded from sema's flow typing through IR and MIR to the
emitted instruction stream, preserved across every value replacement, inline
clone, instruction selection, and register-allocator copy. The one place taint
stops is the declassification a `:>T` cast lowers to: a copy the walk of the
encoded code takes as leaving its destination public, and nothing else. Secret-free code carries
no taint and compiles byte-identically.

No function is required to carry `#[constant_time]`. A function that computes on
a secret without it compiles, held to the gates above and to nothing more: the
compiler makes no promise about the timing of the code it emits for it. The
decorator is how a function asks for that promise.

```mach
#[constant_time("hard")]
fun ct_select(mask: ^u32, a: ^u32, b: ^u32) ^u32 {
    ret (a & mask) | (b & ~mask);
}
```

## The zeroizing-write guarantee

[`$zeroize(p, count)`](comptime-intrinsics.md) is the operation that holds this
guarantee itself, for any `*T`: public, wholly secret or mixed. It lowers to one of
mirl's kept fills, so no pass removes, narrows or merges its stores, including at
release when the target is dead afterwards. A kept fill never lowers to an external,
preemptible `memset`, and its length and timing rest on the size of the target alone,
so a secret `count` is refused.

Wiping a secret is only useful if the wipe survives to run. That guarantee exists,
but it is **not** provided by `#[constant_time]`, and it is scoped more broadly than the
decorator is.

A store states the secrecy of the storage it writes from the place's type, never
from its value: secret where the type holds a secret, public where it holds none.
Its value's secrecy is the value's, which the back end follows from register to
memory, and a store of a value that holds a secret not declassified into storage
stated public is refused in a constant-time function. A store of a public value
into secret storage, the shape a wipe takes, writes a public value. A copy is marked when
the bytes it moves are secret, and the zeroing fill that clears a secret aggregate's
padding writes zeros and is not marked. A constant-time function's copy or fill of a
secret needs a length known at compile time, and one whose length is known only at run
time is refused at the copy or fill on every target, a `$zeroize` excepted. What keeps a wipe in place is that no pass
removes a store, and that a `$zeroize` is a kept fill, which no pass may remove even
once one does, keyed on the write, not on any decorator, so:

```mach
use std.types.size.usize;

# no decorator: the wipe is protected anyway
fun clear(p: *^u8, n: usize) {
    var i: usize = 0;
    for (i < n) {
        p[i] = 0;
        i    = i + 1;
    }
}
```

**What it covers.** Memory, reached through a pointer that escapes the function — the
shape `crypto.ct.zeroize` has. Such a store cannot be promoted out of memory.

**What it does not cover.** A value the compiler keeps in a **register**. Writing to a
promoted local is not a memory write, so wiping one is not preserved:

```mach fragment
var x: ^u8 = k;
x = 0;          # NOT guaranteed: `x` may never have been in memory
```

Adding `#[constant_time]` does not change this — the wipe is outside the memory-address
leakage model rather than an exception within it. To wipe reliably, write through a
pointer whose target is memory the compiler cannot promote away, which is what the
standard library's `zeroize` does. Settled: the wipe guarantee is memory-scoped; secret register lifetimes are outside
it.

**Today the guarantee holds trivially.** mach has no dead-store elimination, so
nothing removes a store: an entirely dead fill of a *public* local also survives at
release. The guarantee becomes load-bearing only if such a pass is added: a kept
fill holds that pass to it, and a plain store into secret storage carries the
secret statement of the place it writes, which such a pass would have to honour.

What a target offers is read at the `#[constant_time]` mark from masc's instruction
set for the target's architecture, which says whether its rows classify their
timing, whether it cannot guarantee timing at all, or whether it classifies none:

- **Its rows classify their timing** (x86_64, aarch64, riscv64, riscv32): masc's
  walk of the encoded code validates the function.
- **It cannot guarantee timing** (wasm, whose runtime decides the instructions that
  execute): the function compiles, with the warning `secret.target_unenforceable`
  at the mark that the target cannot practically enforce constant-time execution.
- **It classifies none** (SPIR-V, whose module a downstream compiler translates):
  the function is **refused** under `secret.target_unverifiable`, since neither
  that translation nor the device's timing behaviour is covered by the leakage
  model. Compile constant-time code for a machine target and pass such a target
  only public data.

## Constant-time multiply by instruction set

`*` on a secret operand means what it means on a public one: the wrapping,
same-width product. What changes per target is whether the machine can execute
it without a timing leak. mirl decides that from the class masc's instruction
set gives each row its selection emits for the multiply, never from a guess,
and mach states no table of its own. A **cell** is one multiply at one operand
width, and the class of the rows that realize it says the **condition** under
which it has data-independent timing:

- **always**: nothing to check;
- **PSTATE.DIT on**: the cell is data-independent only while the processor's
  DIT mode is set, which a `soft` function sets where the processor has it and
  a `hard` function requires in the target's baseline (see [Soft and
  hard](#soft-and-hard));
- **extensions selected**: the cell is data-independent only on a machine that
  has the named extensions. A `hard` function admits the row only when the
  target selects every one of them (`ext` in the manifest, or the `isa`). A
  `soft` function admits it without them and leaves the timing to the
  processor, as it does an aarch64 row without DIT: riscv64's `mul` rows build
  in a soft function on a target without `zkt`.

The multiply column is the spelling `$mach.build.ct_mul` takes: `low` is the
same-width product, `high_u` and `high_s` the upper half of the unsigned and
signed full product, `high_su` the upper half of a signed-by-unsigned product,
and `wide_u` and `wide_s` the full double-width product of two operands of the
named width as one instruction. The width is the operand's: a product whose
result is no wider than the machine's ALU is a `low` cell at the result width
(`(a::^u64) * (b::^u64)` over 32-bit `a` and `b` is the 64-bit `low` cell on a
64-bit target), and only a product wider than the ALU is fused into the
widening cell (see [operators.md](operators.md)). The narrower `wide` and
`high` rows classify the inline-asm forms (the one-operand `mul r32`) the same
way.
The declared rows, one line per multiply:

| Instruction set | Multiply | Widths | Condition |
|---|---|---|---|
| `x86_64` | `low` | 8, 16, 32, 64 | always |
| `x86_64` | `high_u` | 8, 16, 32, 64 | always |
| `x86_64` | `high_s` | 8, 16, 32, 64 | always |
| `x86_64` | `wide_u` | 8, 16, 32, 64 | always |
| `x86_64` | `wide_s` | 8, 16, 32, 64 | always |
| `aarch64` | `low` | 8, 16, 32, 64 | PSTATE.DIT on |
| `aarch64` | `high_u` | 64 | PSTATE.DIT on |
| `aarch64` | `high_s` | 64 | PSTATE.DIT on |
| `aarch64` | `wide_u` | 32 | PSTATE.DIT on |
| `aarch64` | `wide_s` | 32 | PSTATE.DIT on |
| `riscv64` | `low` | 8, 16, 32, 64 | `m` and `zkt` selected |
| `riscv64` | `high_u` | 64 | `m` and `zkt` selected |
| `riscv64` | `high_s` | 64 | `m` and `zkt` selected |
| `riscv64` | `high_su` | 64 | `m` and `zkt` selected |
| `riscv32` | none | | |
| `spirv` | none | | |

The table describes masc's rows, and masc's registration of each instruction
set is the source. Inside a constant-time function a secret multiply whose
rows are not classified constant time under the function's mode is refused, as
is every lane multiply (`i32x4 * i32x4` on a secret vector), which no
instruction set classifies.

**Realized cells.** A cell no instruction executes directly is admitted exactly
when every cell of its realization is, under the union of their conditions. On
x86-64 the one-operand `mul` and `imul` write the full product to a register
pair, so the high halves and the widening products are the same instruction;
on aarch64 and riscv64 the full product is the low multiply beside the high
one (`mul` + `umulh`/`smulh`, `mul` + `mulhu`/`mulh`), so the 64-bit `wide_u`
and `wide_s` are admitted there through the 64-bit `low` and high rows. A
**128-bit** multiply is never declared by a target, because no target executes
one: `(a::^u128) * (b::^u128)` with 64-bit `a` and `b` is the 64-bit widening
product, one instruction, and its high half `(... >> 64)::^u64` is that same
instruction; a secret `^u128 * ^u128` low product is the schoolbook over the
lanes, three 64-bit low products and the widening one. `$mach.build.ct_mul(wide_u,
64)` answers for the 64-bit low and high halves together, so on aarch64 it needs
DIT like the 64-bit high half, and a 128-bit cell answers as mirl's selection
covers a multiply of that width.

**Why each row holds.** Every row cites its source beside the declaration in the
instruction set's registration, and the kind of each claim is marked:

- **x86-64, always, Intel and AMD.** On Intel it is a vendor guarantee:
  the "Data Operand Independent Timing" guidance states that processors which
  do not enumerate DOITM may be assumed to behave as if it were enabled for the
  listed instructions, and `mul` (F6, F7), `imul` (69, 6B, 0F AF, F6, F7) and
  `mulx` (F6) are on that list, so every operand size of the one- and
  two-operand forms. BearSSL's `ctmul` table, measured per core, records a
  constant-time multiply since the first Pentium. On AMD the evidence is by
  table rather than by vendor document: AMD publishes no equivalent list and no
  mode control (absence, verified 2026-09-18), the same BearSSL table records a
  constant-time multiply at every width for every AMD core from K7 through Zen,
  no AMD x86-64 core has ever been documented with a data-dependent multiplier,
  and AMD's SB-1039 advises constant-time algorithms, which presupposes
  constant-time instructions. The compiler emits `imul` for the low half and
  the one-operand `mul`/`imul` for the high halves and widening products.
- **aarch64, PSTATE.DIT on.** The Arm ARM (DDI 0487, "About PSTATE.DIT") and
  the DIT register page of DDI 0601 list the data-processing (3 source)
  instructions `madd` (`mul`), `msub`, `smaddl` (`smull`), `smsubl`, `smulh`,
  `umaddl` (`umull`), `umsubl` and `umulh` as data-independent while DIT is
  set. The 8- and 16-bit low halves are the 32-bit `madd`, the 64-bit high
  halves are `umulh` and `smulh`, and the 32-bit widening rows are `umaddl`
  and `smaddl`. A `soft` function sets the mode around itself where the
  processor has it, and a `hard` one builds only where the target's baseline
  holds DIT, and its refusal names the feature.
- **riscv64, `m` and `zkt` selected.** The RISC-V Cryptography Extensions
  Volume I (scalar), chapter "Data Independent Execution Latency Subset: Zkt",
  table RVM, lists `mul`, `mulh`, `mulhsu`, `mulhu` and (rv64) `mulw`, and
  excludes `div` and `rem`. Zkt changes no instruction: it is a promise about
  the hart's timing, so like any selected extension it is assumed of every
  machine the binary runs on. riscv32 declares no rows yet.
- **SPIR-V** has no timing model and declares nothing.

**DOITM.** masc's x86-64 rows state the timing Intel lists under its Data
Operand Independent Timing Mode (`IA32_UARCH_MISC_CTL[0]`), and mirl checks
every x86-64 constant-time function under that mode. It is a model-specific
register the kernel owns and user space cannot set, so mach neither switches it
nor offers a manifest key for it. Intel states that a processor which does not
enumerate DOITM behaves as if it were on for the listed instructions. On one
that enumerates it and runs with it off, the code still runs, and the listed
instructions' timing is not promised. A program that wants the mode asks the
operator for the kernel control, the same class as SMT and other side-channel
mitigations.

**Fail closed, never fall back.** In a constant-time function a secret
multiply mirl's check does not clear is a compile error. The compiler never
substitutes a bit-serial loop, a shift-add expansion or a call for it, so a
secret square and a secret multiply by a constant each reach the machine as the
multiply the selection emits, and masc's walk of the encoded code holds it to
its row. How a constant-time function holds the mode those rows need is the
next section.

## Soft and hard

An aarch64 multiply has data-independent timing only while the processor's
DIT mode is on (Arm ARM DDI 0487, "About PSTATE.DIT"; the DIT register page of
DDI 0601 lists the instructions: `madd`, `msub`, `smaddl`, `smsubl`, `smulh`,
`umaddl`, `umsubl`, `umulh`). The mode is per-thread processor state that user
code turns on with `msr dit, 1`, and a processor without FEAT_DIT has no such
bit. On riscv the same promise is the Zkt extension, which changes no
instruction and is assumed of every machine a binary built with it runs on. A
constant-time function's mode says what happens when the build cannot be sure
the promise holds.

**`soft`** holds the mode where it can and runs anyway where it cannot. mirl
emits a save of the DIT bit, a set and a restore around the function. The set
is `msr dit, #1` followed by `dsb nsh` and `isb sy`, as `std.system.dit.enable`
writes it, so no instruction after it runs speculatively under the timing from
before it (Apple's "Writing ARM64 code for Apple platforms" asks for a
speculation barrier after turning the mode on). The restore needs none. Where the
target does not guarantee DIT, the set is guarded by std's byte that says
whether the processor has the mode, and where it does, the set is
unconditional. Without DIT the function still runs, with the code-level rules
held and the instruction timing left to the processor. A call from one
constant-time function to another does not toggle the mode again.

**`hard`** is decided when the program is compiled, and never at run time. The
build's target must guarantee data-independent timing for every operation the
function uses, or the build is refused with a diagnostic naming the missing
feature:

- aarch64 needs DIT in the target's baseline, as darwin-arm64 has it;
- riscv needs Zkt in the selection;
- x86-64's scalar cells hold unconditionally, so they pass.

**Calls between the modes.** Both modes do the same at entry and at every
exit (`mirl.legal.timing`): a function entered with the mode on switches
nothing and leaves it on, and one entered with it off switches it on and
restores it before it returns. They differ only in what the check admits. A
soft function's code is held under every mode the processor may offer, the
switched and the promised ones included, and a hard function's only under the
modes the build guarantees. So a soft caller may call either mode, and a hard
caller may rely on a soft callee exactly where the build guarantees every mode
a soft function's code may be held under, as the hard callee's own check would
have required: every x86-64 build, whose one mode the system sets for every
process, an aarch64 build that selects `dit` and a riscv64 build that selects
`zkt`. Elsewhere a hard function passing a secret to a soft one is refused
(`secret.call_untrusted`), and masc's walk takes the soft callee as one it does
not trust.

A `hard` function needs nothing linked into the image, and a `soft` one needs
only the availability byte below and, in a shared library, the probe that sets
it. There is no object mark the linker checks, and a program never refuses to
start for want of the mode. For code that holds the mode by hand,
std keeps a manual DIT API: `std.system.dit.enable`, `disable` and `read` on
aarch64, and `std.crypto.ct.begin`, which turns the mode on for the calling
thread where the processor has it. `std.system.timing.mode()` answers what the
hardware offers.

The availability byte, `__mach_timing_available`, is one byte in each image,
hidden, so no image reads another's. Every object whose `soft` functions read
it defines it, weak and zero, and `std.system.dit` holds the strong definition
that stands for those where the image reaches it, so an image links whether or
not it imports std. std's program start sets it once before `main` and before
any thread starts, from what the system reports:

- **aarch64-linux** reads `HWCAP_DIT` (`1 << 24`) in `AT_HWCAP`, which the
  kernel exposes since v4.17 (commit 7206dc93a58f, "arm64: Expose Arm v8.4
  features"; `Documentation/arch/arm64/elf_hwcaps.rst`: "Functionality implied
  by ID_AA64PFR0_EL1.DIT == 0b0001"). PSTATE is saved to `SPSR_EL1` on every
  exception entry and restored on return, so the bit outlives a context switch.
- **aarch64-darwin** reads the sysctl `hw.optional.arm.FEAT_DIT`. Apple's
  "Writing ARM64 code for Apple platforms", section "Enable DIT for
  constant-time cryptographic operations", documents the mode as per-thread
  state user code turns on with `msr dit, #1` and reads back as bit 24 of
  `mrs dit`.
- **freestanding aarch64** reads `ID_AA64PFR0_EL1.DIT` when it runs at EL1, and
  leaves the byte 0 where it may not ask.
- **aarch64-windows** leaves the byte 0, so a `soft` function runs without the
  mode there, and a `hard` one builds only where the target's baseline holds
  DIT.

A shared library runs no program start, so on Linux each object that reads the
byte also holds `__mach_timing_probe`, a merged hidden function, and an entry
for it in the image's table of load-time functions at priority 0
(`.init_array.00000`) in the probe's own group, so the image keeps one probe and
one entry. The loader runs the table before `dlopen` returns and before any
library that depends on this one runs its own, and mink lays the entries of a
table by priority, a lower one first and every entry that states none after,
so the probe runs before any other load-time function of the image, one that
calls a `soft` function included. An executable keeps std's start: it has no
table its loader runs before `_start`, so the entry would never run there, and
a library initialiser can reach an executable's `soft` function before `main`
only through a pointer the executable gave it, in which case that call runs
without the mode, as `soft` allows.

The probe reads `ID_AA64PFR0_EL1` with `mrs` and sets the byte to 1 where its DIT
field, bits 51 to 48 read signed, is at least 1. At EL0 that read traps, and
Linux emulates it from v4.11, which advertises the emulation as `HWCAP_CPUID`,
showing the DIT field from v4.17 and reading it as 0 before
(`Documentation/arch/arm64/cpu-feature-registers.rst`: "the kernel emulates
the operation if the source belongs to the supported system register space",
and an unexposed register "causes SIGILL"). A kernel older than v4.11 has no
emulation and sends SIGILL, so a library with a `soft` function fails to load
there. That is accepted: a library has no other source of the answer, since it
runs no start that is handed the auxiliary vector and mach links no C library to
ask, and v4.9, the last kernel without the emulation, left long-term support
in January 2023. Windows gives user code no read of the ID registers, so no
probe is delivered there and the byte stays 0.

An image that runs no table of load-time functions and no std start, such as
a freestanding image with its own entry, keeps the byte 0, so its `soft`
functions run without the mode, as `soft` promises and `hard` exists to
refuse. Such an image sets the byte by calling the probe once before any
`soft` function runs, declared as
`#[symbol("__mach_timing_probe")] ext fun probe();`, at EL1 where the read
is the processor's own. The probe is present in every image that holds a `soft`
function reading the byte. `std.system.dit.available` and
`std.crypto.ct.begin` read the same byte. A `hard` function reads no byte and
behaves in a library as in a program.

## Padding of secret aggregates

A secret aggregate's padding bytes are zero. A type holding a secret, a `^T` or
an aggregate with a secret member or element, has every byte its layout leaves
between or after its members zeroed wherever a value of it is built: a literal,
a member-by-member construction and a default initialization. A copy moves the
value whole, padding included, so the padding stays zero. A stale secret
therefore never rides in the padding of a value that is later copied or written
out whole. An aggregate that holds no secret is laid out and built as before,
its padding unspecified, except a literal in a constant-time function, whose
padding is zeroed too: the function's check refuses a read of stack no store
reached, and a load of the whole value reads its padding.

A tag or union literal writes the same way. Where its type holds a secret, or
the literal is in a constant-time function, every byte the case or member it
names leaves unwritten is zeroed before the discriminator and the payload are
stored, so `ret opt[u64].none{}` in a constant-time function returns a value
whose payload bytes are zero rather than stale. Anywhere else a tag literal
writes its discriminator and its payload alone, and a union literal its
member alone.

## What is promised about memory

The compiler promises two things of secret data: a constant-time function
processes it in constant time, and no value typed secret reaches a public one
without a `:>`. It does not promise memory hygiene. A function's stack slots,
spills and registers are not wiped when it returns, so a secret's bytes can
remain in dead stack memory after the function that held them. Bytes nothing
wrote, as a public record's padding or the unused bytes of a union or tag of a
public type, are unspecified and may hold whatever was there before, a dead
frame's secret included. A program that sends or stores a record by its raw
bytes sends those bytes too. A long-lived buffer holding a secret is wiped by
its author with std's helpers, `std.crypto.ct.zeroize` or
`std.memory.secret.borrow_wipe`.

## Trusted base

The guarantee rests on four parts:

- mach's secrecy check, the flow typing and gates above, which hold for the
  source,
- mirl's lowering, which carries each secret through selection, allocation,
  spilling and framing to the emitted instructions and adds the timing switch
  of a `soft` function,
- masc's check of the final program, the walk of each constant-time
  function's encoded code against the timing class of every row it executes,
- the hardware timing mode, DIT on aarch64 and DOITM on x86-64, where the rows
  need it.

The only secret-to-public crossings are the explicit `:>T` cast and inline
`asm`, which masc's walk validates but a type system cannot check. A proof is
always relative to a leakage model. Its fidelity to real silicon is empirical.

What is checked and what is trusted: the checker verifies the functions marked
`#[constant_time]`, their `asm` included. Code that is not marked is trusted to
follow the type system and its declared signatures, and `asm` outside a
constant-time function is not checked. Three consequences of that statement are
known and accepted under it, each awaiting a ruling:

- an untrusted callee that reads a register its declaration does not pass, and
  so a secret the caller left there;
- an untrusted callee whose `asm` reads its caller's frame;
- untrusted code whose `asm` moves a secret it reaches through a global into
  public storage it was given, directly or through a trusted callee.

## Assurance

**The guarantee.** For code marked `#[constant_time]` and compiled for a
covered target, the emitted machine code's branches and memory addresses never
depend on secret values, and no instruction whose timing depends on its
operands receives a secret. masc's walk proves this on the encoded code of each
such function, after selection, register allocation, spilling and frame
layout, and a build that cannot prove it is refused.

**Covered targets.** Constant time is a working feature on linux x86-64 and
linux aarch64. Every other target is experimental, which covers linux riscv64
and riscv32, darwin x86-64 and aarch64, windows, uefi and every freestanding
target: the same checks run there, but the targets have not been cross-checked
to the standard the covered ones have. wasm targets compile a constant-time
function with the warning `secret.target_unenforceable`, since the runtime
chooses the instructions that execute, and SPIR-V refuses one under
`secret.target_unverifiable`.

**Soft and hard.** A `soft` function switches the hardware timing mode on
around itself where the processor has it and runs without it where it does
not. A `hard` function builds only where the target guarantees the timing of
every instruction it uses, and the refusal, `secret.target_unguaranteed`,
names the missing feature. Arm promises an instruction's timing only while DIT
is on, and the linux aarch64
baseline holds no DIT, so there a `hard` function builds only when the target
selects `dit` in its `isa` or `ext`, and a `soft` one switches DIT on when the
processor has it. On linux riscv64 the same holds of Zkt.
[Soft and hard](#soft-and-hard) has the details.

**Out of scope.** Cache and power side channels are not covered. Neither is an
access past the object a pointer addresses, including past a narrowed pointer
cast: secrecy holds for in-bounds accesses only.

**The x86-64 caveat.** masc's x86-64 timing rows come from Intel's "Data
Operand Independent Timing Instructions" list (intel.com developer article
768547, updated 2026-02-23, 840 entries). A listed instruction's timing is
independent of its data operands while DOITM is on, and
[DOITM](#constant-time-multiply-by-instruction-set) says when that holds. AMD
publishes no equivalent list, so on AMD processors the guarantee rests on
Intel's list with no statement from AMD behind it.

**Independent checking.** The guarantee calls for two independent analyzers to
cross-check the emitted binaries, BINSEC's checkct and valgrind's memcheck with
the secret marked undefined (ctgrind), whose harness is mirl's `test/ct`. For the v7
toolchain only ctgrind has been run, on linux x86-64, over a binary built from
std's SHA-256 and `std.crypto.ct` functions with two planted leaks: it reported
nothing on the constant-time code and flagged the secret branch and the secret
index. checkct has not been run, and no analyzer has been run on linux aarch64.

## See also

- [types.md](types.md) - the compound type grammar ^ qualifies
- [tag.md](tag.md) - tagged values and outer-secret ^Tag rules
- [comptime-intrinsics.md](comptime-intrinsics.md) - $is_secret, $holds_secret and the rest of the type-predicate family
- [operators.md](operators.md) - the :: / :~ casts that preserve secrecy and :>T
- [decorators.md](decorators.md) - the #[constant_time] decorator reference
- [grammar.md](grammar.md) - the formal grammar of ^ and :>T
