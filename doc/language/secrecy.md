# `^` — the secret qualifier

`^T` marks a type as carrying secret data for Mach's constant-time guarantee.
Sema tracks secrecy as an information-flow discipline: secret data may move and
be stored, but it may never reach a position a classical leakage model observes
(a branch, a memory address, a variable-latency instruction). This page covers
the flow-typing rules and the `#[oblivious]` codegen contract. The grammar lives
in [grammar.md](grammar.md); the decorator reference is in
[decorators.md](decorators.md).

> **Experimental preview.** The constant-time support is incomplete and
> unaudited: a proven secret-disclosure path is still open. Read
> [Assurance](#assurance) before relying on any of this. **Do not build
> production cryptography on it at this version.**

## Secrecy lattice

There are two secrecy levels in a two-point lattice: public is the bottom,
secret the top. `^` lifts a type to secret and binds to the type immediately to
its right, so it nests with `*` and `[N]` in any order: `^u32`, `*^u8`, `^*u8`,
`[N]^u8`, `^MyRec`. Doubling collapses: `^^T` is `^T`.

A public value coerces *up* to secret wherever a secret is expected, with no
syntax:

```mach
fun up(p: u32) ^u32 { ret p; }      # public u32 flows into a secret slot
```

A **literal is public by construction** and stays public through that coercion:
its value sits in the instruction stream, so classifying it as secret would
protect nothing. `var s: ^u32 = 5;` types `5` as `u32` and relies on the
up-coerce. The join below is unaffected — a value *computed* with a secret is
secret however its other operand is spelled — so `v << 3` on a secret `v` still
yields a secret, while the constant `3` is not mistaken for a secret shift
count.

The reverse never happens implicitly. The only downgrade is the explicit `:^`
strip below.

## Join

Any operation with a secret operand yields a secret result. Taint joins across
arithmetic, bitwise, shift, and comparison operators, and through a value read
out of a secret container:

```mach
fun mix(a: ^u32, b: u32) ^u32 { ret a + b; }    # ^u32 + u32 -> ^u32
rec Key { d: ^[32]u8; }
fun first(k: Key) ^u8 { ret k.d[0]; }            # element of a secret array is ^u8
```

`&T` of a `^T` value is the public pointer `*^T` (the address is public, the
pointee secret), and dereferencing `*^T` recovers the secret `^T`.

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

```mach
fun leak(a: ^u32, t: *u8, p: ^*u8) u8 {
    if (a) { ret 1; }       # error: secret value used as a branch condition
    ret t[a];               # error: secret value used as a memory index
    ret @p;                 # error: secret value used as a memory address
}
```

Three more gates decide against the target's constant-time capabilities rather
than the source alone, so they are reported at lowering:

- a secret operand of a **floating-point** operation (always variable-latency,
  gated on every target)
- a secret operand of an **integer multiply** on a target without a trusted
  data-independent-timing mode — conservatively every ISA today
- a secret **variable shift count** on a target without a barrel shifter

A secret value passed to a variadic pack is also rejected, including a secret
wrapped inside an aggregate.

The gates are checked against the types of the **instance**, not of the template.
A generic's body is re-checked per instantiation under its concrete type
arguments, and a pack-tailed function's body — the statements around its `$each`
as well as the unrolled body itself — is re-typed per monomorphized instance, so
a `T` that instantiates to a secret is gated exactly as the secret spelled out
in full would be.

## Downgrade with `:^`

`:^` is the only way to remove `^`. It produces a new public value and never
reinterprets storage in place:

```mach
fun publish(a: ^u32) u32 { ret a:^; }       # bare strip
fun publish2(a: ^u32) u32 { ret a:^u32; }   # explicit target names the public type
```

`:^` peels exactly the outer qualifier, so it can never launder a welded pointee
(`*^T` stays `*^T`). `::` and `:~` may neither add nor drop `^`.

## Welded-storage pointers

Secrecy is fixed at declaration and is non-launderable, which makes the
public/secret aliasing leak unconstructable with no alias analysis:

- a `^T` is stored only through a `*^T`, never a `*T` (the lattice forbids the
  downgrade)
- a secret-welded pointer cannot be erased to the untyped `ptr`
- a `uni`'s overlapping variants must agree on secrecy

```mach
fun erase(p: *^u8) ptr { ret p; }     # error: cannot erase a secret pointer to ptr
uni Bad { a: ^u32; b: u32; }          # error: variants disagree on secrecy
```

The union rule is a property of the union **type**, not of the syntax that
declared it, so it holds for an inline `uni { ... }` with no declaration to hang
a check on, and at every **instance** of a generic union. At the declaration a
variant typed by a generic parameter says nothing about secrecy, so `uni U[T] {
a: T; b: u32; }` agrees there and is decided where each instance is formed:

```mach
uni U[T] { a: T; b: u32; }
rec Box[T] { u: U[T]; }

var s: U[^u32];                       # error: this instantiation makes the variants disagree
var b: Box[^u32];                     # same error: the instance need not be spelled
var p: U[u32];                        # fine, and so is an all-secret instantiation
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

Two aggregates are compared **by byte extent**, not by field ordinal: matching
`^` placement in the type graph does not put two fields on the same bytes, so
each paired field must also agree in size and alignment. That is what keeps
every later field aligned between the two, and without it a narrower secret on
one side displaced a public field onto a secret one. Differing *shapes* are
fine — with the common prefix identical byte for byte, a longer aggregate's
extra fields provably begin past the shorter one's extent, so a public field
beyond the secret is accepted. This holds only for sequential layout; a union
overlays every variant at offset 0, so a differing-shape pair involving one is
admitted only when neither side has a public-stored byte at all.

The comparison reads the same layout the backend emits. Where it cannot
determine a layout it declines, which rejects.

## `#[oblivious]` — the codegen contract

The flow typing constrains the *source*; `#[oblivious]` carries the obligation
through *codegen*. Inside a function carrying it, the backend must not introduce
a secret-dependent branch or select a variable-latency instruction on a secret
operand.

Inline `asm` inside such a function is **validated**, not rejected. The block is
parsed into instructions and walked for the same three leaks the compiler checks
everywhere else — a secret reaching a branch condition, a memory address, or a
variable-latency operation the target cannot do in constant time. Taint enters
through the block's `{name}` bindings, whose secrecy is stamped from the local's
declared type. What the walk cannot model, it refuses:

| construct | why |
|---|---|
| a body that does not parse | nothing to analyze |
| a data directive (`.byte`, `.word`, `.long`, `.quad`) | its payload can encode any instruction |
| a mnemonic the target has not classified | its timing behaviour is unknown |
| **a conditional branch, on x86-64 only** | its condition rides FLAGS, which the inline-asm effect model does not represent (#2460) |

That last row is a per-target asymmetry worth stating plainly. aarch64's only
conditional forms are `cbz`/`cbnz` and riscv64's branches compare two registers, so
on both the condition is a register operand the walk can see, and both get the full
three checks. x86-64's `jcc` family conditions on flags the model does not carry, so
a `cmp` of a secret before it would be invisible — the branch is refused there until
flags are modeled.

The variable-latency check also bites unevenly: x86-64's and aarch64's asm grammars
carry no divide, multiply or float instruction at all, so it reaches only their
register-count shifts. riscv64's grammar admits the whole M-extension, so on that
target the check is substantive.

`#[oblivious]` remains a **per-function** contract. A call out to a non-oblivious
function is not validated — that is the boundary the decorator draws, not a hole in
it, and it applies to a callee containing `asm` exactly as it applies to any other.

The zeroizing-write guarantee is separate and broader; it is described below.

A secret-taint bit is threaded from sema's flow typing through IR and MIR to the
emitted instruction stream, preserved across every value replacement, inline
clone, instruction selection, and register-allocator copy. The one place taint
stops is the declassify barrier a `:^` cast lowers to. Secret-free code carries
no taint and compiles byte-identically.

A function instance that **computes** on a secret must carry `#[oblivious]`; one
that only moves, stores, or declassifies secrets is transparent and stays
annotation-free. The check runs per monomorphized instance, so a generic
instantiated at a secret type is held to the concrete type's rules.

```mach
#[oblivious]
fun ct_select(mask: ^u32, a: ^u32, b: ^u32) ^u32 { ret (a & mask) | (b & ~mask); }
```

A **translation validator** re-derives the taint over the lowered,
target-independent MIR as a monotone dataflow fixpoint and independently
re-checks the leakage conditions, so a secret reaching a branch condition, a
memory address, or a forbidden variable-latency op is a compile error naming the
function and the offending operation. It is a backstop behind the compile-time
gates, not a replacement for them.

## The zeroizing-write guarantee

Wiping a secret is only useful if the wipe survives to run. That guarantee exists,
but it is **not** provided by `#[oblivious]`, and it is scoped more broadly than the
decorator is.

A store into secret storage is tainted at lowering, from two composing sources: the
stored **value**'s secrecy, and — for a public value written into secret storage, the
shape a wipe takes — the **destination**'s secrecy, read from the lvalue's semantic
type. Either one marks the store. That taint is the thing an optimization must
consult, and it is keyed on the storage, not on any decorator, so:

```mach
# no decorator: the wipe is protected anyway
fun clear(p: *^u8, n: usize) {
    var i: usize = 0;
    for (i < n) { p[i] = 0; i = i + 1; }
}
```

**What it covers.** Memory, reached through a pointer that escapes the function — the
shape `crypto.ct.zeroize` has. Such a store cannot be promoted out of memory, and the
taint is present for any future pass to read.

**What it does not cover.** A value the compiler keeps in a **register**. Writing to a
promoted local is not a memory write, so wiping one is not preserved:

```mach
var x: ^u8 = k;
x = 0;          # NOT guaranteed: `x` may never have been in memory
```

Adding `#[oblivious]` does not change this — the wipe is outside the memory-address
leakage model rather than an exception within it. To wipe reliably, write through a
pointer whose target is memory the compiler cannot promote away, which is what the
standard library's `zeroize` does. Settled: the wipe guarantee is memory-scoped; secret register lifetimes are outside
it (#2456).

**Today the guarantee is not yet load-bearing**, because no dead-store elimination
exists to remove anything: an entirely dead fill of a *public* local also survives at
release. The taint is what makes the requirement enforceable *before* such a pass
lands, and `mach.lang.driver:secret_store_taint_survives_lower` pins it.

The contract is only offered where mach emits the instructions that execute.
A target whose back half hands a module to a downstream compiler instead — the
experimental SPIR-V backend — **rejects `#[oblivious]`**: neither that
translation nor the device's timing behaviour is covered by the leakage model,
so the obligation could be neither validated nor upheld. Compile constant-time
code for a machine target and pass such a target only public data.

## Trusted base

The only secret-to-public crossings are the explicit `:^` cast and inline `asm`
blocks (which a type system cannot check). Everything else is enforced. A proof
is always relative to a leakage model. Its fidelity to real silicon is
empirical.

## Assurance

**The constant-time guarantee is incomplete. This support is an experimental
preview and has not been audited. Do not build production cryptography on it at
this version.**

What holds today: the type system checks that the source respects the leakage
model, `#[oblivious]` carries the obligation through codegen, and the
translation validator independently re-checks the lowered MIR. The dudect-style
timing harness at `int/ct/` measures a branchless constant-time reference
against a deliberately-leaky control and flags the leak with Welch's t-test —
run it with `bash int/ct/ct.sh`.

**What the timing harness does and does not assure.** The leakage model has
three channels, and the harness does not cover them uniformly
(briar-systems/mach#2363):

| channel | assured by |
|---|---|
| control-flow trace | the source-level branch gate, plus the harness's latency mode |
| variable-latency operands | the sema/lowering gates, plus the harness's latency mode |
| memory-address trace | the source-level **secret-index and secret-address gates**, plus the harness's address mode |

The two harness modes need opposite sampling and neither substitutes for the
other. Latency mode times a large batch of calls per sample, which is what lifts
a running-time difference above clock resolution. That same batching *hides* an
address-trace leak: every call in a batch is handed the same input, so after the
first call both input classes are reading a warm cache line and the single cache
miss carrying the signal is averaged away. Measured on one function — a
secret-indexed read over a table larger than the last-level cache — latency mode
scores |t| ≈ 1–15 across runs (straddling its own threshold, so it neither
confirms nor denies) while address mode, at one call per sample, scores in the
hundreds.

Two consequences worth stating plainly:

- A clean latency-mode number for a table lookup is **not** evidence of
  address-trace safety. It is the wrong instrument for that channel.
- An address-trace leak is only *measurable* when the table exceeds the
  last-level cache. A cache-resident table leaks its index just as truly and no
  timing harness will see it. For small tables the property rests entirely on
  the secret-index gate and on reading the emitted code.

What does not hold — the known open holes:

- **`$fields` reflection projection inside a generic erases a secret field's
  secrecy**, which discloses the secret. Proven, security-blocking
  (briar-systems/mach#2168).
- the validator over-taints a wide secret on a narrow-ALU target, rejecting a
  public-count shift as a secret memory address — a false positive; it fails safe
  (briar-systems/mach#2195)
- a **member or index access through a secret pointer** is rejected by the
  secret-address gate, but a `p.x` / `p[i]` on a plain `^Rec` or `^[N]T` — which
  this page documents as legal — fails at lowering, which does not strip the `^`
  before resolving the field. A spurious error, not a disclosure
- an anonymous `uni` nested in a **generic** record does not get union layout:
  its variants occupy distinct storage instead of overlapping
  (briar-systems/mach#2239). A layout defect, not a secrecy one — the mixed-secrecy
  rule rejects those instantiations either way — but it is the reason a
  `Result[^u32, u32]` built before that rule landed leaked nothing at run time

The validator's scope is the lowered MIR; it trusts instruction selection, width
legalization, register allocation, and encoding to be timing-preserving.
Validation of the emitted machine code is future work. Where mach does not own
those stages at all — a whole-module emitter such as SPIR-V — the contract is
refused rather than assumed.

## See also

- [types.md](types.md) — the compound type grammar `^` qualifies
- [operators.md](operators.md) — the `::` / `:~` casts that preserve secrecy
- [decorators.md](decorators.md) — the `#[oblivious]` decorator reference
- [grammar.md](grammar.md) — the formal grammar of `^` and `:^`
