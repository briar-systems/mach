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
- a secret operand of the always-variable-latency `/` or `%`

```mach
fun leak(a: ^u32, t: *u8) u8 {
    if (a) { ret 1; }       # error: secret value used as a branch condition
    ret t[a];               # error: secret value used as a memory index
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

The check is **deep** and **fails closed**: a secret nested anywhere inside an
aggregate (including a generic instance's lazily-materialized fields) counts as
secret at these boundaries, and a placement the checker cannot prove severs no
weld — a `:~` reinterpret across differing shapes, or a secrecy difference
reachable through a function type — is rejected rather than allowed. This
over-rejects in some safe cases (briar-systems/mach#2167); rejecting is the
correct default for a confidentiality property.

## `#[oblivious]` — the codegen contract

The flow typing constrains the *source*; `#[oblivious]` carries the obligation
through *codegen*. Inside a function carrying it, the backend must not introduce
a secret-dependent branch, select a variable-latency instruction on a secret
operand, or dead-store-eliminate a zeroizing write to secret storage. Inline
`asm` is rejected inside such a function.

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

What does not hold — the known open holes:

- **`$fields` reflection projection inside a generic erases a secret field's
  secrecy**, which discloses the secret. Proven, security-blocking
  (briar-systems/mach#2168).
- a **secret pointer dereference is gated only by the translation validator**,
  which runs inside `#[oblivious]` functions alone, so anywhere else it compiles
  with no diagnostic — where the equivalent secret *index* is a source-level
  error everywhere (briar-systems/mach#2191)
- deep secrecy placement over-rejects differing-shape aggregates that lack field
  offsets — sound but too strict (briar-systems/mach#2167)
- the validator over-taints a wide secret on a narrow-ALU target, rejecting a
  public-count shift as a secret memory address — a false positive; it fails safe
  (briar-systems/mach#2195)
- a literal shift count coerces to `^T` and inherits secrecy, tripping the
  constant-time shift gate — a false positive; it fails safe
  (briar-systems/mach#2196)

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
