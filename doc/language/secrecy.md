# `^` — the secret qualifier

`^T` marks a type as carrying secret data for Mach's constant-time guarantee.
Sema tracks secrecy as an information-flow discipline: secret data may move and
be stored, but it may never reach a position a classical leakage model observes
(a branch, a memory address, a variable-latency instruction). This page covers
the flow-typing rules. The grammar lives in [grammar.md](grammar.md); the
`#[oblivious]` codegen contract that turns these types into a per-build proof is
separate.

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

Floating-point operations (gated by default) and integer multiply (gated per the
target's constant-time capability) join these once the codegen taint contract
lands.

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

## Trusted base

The only secret-to-public crossings are the explicit `:^` cast and inline `asm`
blocks (which a type system cannot check). Everything else is enforced. A proof
is always relative to a leakage model. Its fidelity to real silicon is
empirical.

## See also

- [types.md](types.md) — the compound type grammar `^` qualifies
- [operators.md](operators.md) — the `::` / `:~` casts that preserve secrecy
- [grammar.md](grammar.md) — the formal grammar of `^` and `:^`
