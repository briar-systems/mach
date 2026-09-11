# L3 tag storage lowering inventory (#3218)

Probe inventory for roadmap item L3 on feat/3218, recorded 2026-09-11 before
any lowering change. Each row was established by a freestanding fixture built
with the feat/3218 compiler for linux-x86_64, linux-arm64 (qemu-aarch64) and
linux-riscv64 (qemu-riscv64) in both the debug and release profiles, exiting 0
only when every check in the fixture held. The fixtures are carried into
`src/lang/driver/tests.mach` as the L3 runtime tests.

Contract: `doc/design/tagged-values.md`, sections "Initialization and
representation" and "Payload places and guards".

## Mechanism already in place

A construction `T.case{payload}` lowers to a fresh stack slot of the tag's IR
type, a `memzero` of that slot, evaluation of the payload expression, a store
of the payload through `gep(slot, tag, [0, case])`, and a store of the case
code at offset zero. The construction is an aggregate value addressed by that
slot. Assignment lowers the RHS first (the construction or an aggregate
snapshot of the source place), then the destination address, then one
whole-object store. Guarded payload places lower to the same `gep` and, in the
debug profile, a discriminator compare that traps on mismatch. Static tag
values fold through the constant-aggregate path with the case code at offset
zero and the payload at the common payload offset.

Those four pieces together already give the L3 storage semantics: the zeroed
temp supplies gap, inactive-suffix and tail-padding zeroing, the temp supplies
RHS capture before the destination is overwritten, and the whole-object store
supplies copies for every place kind.

## Already correct (verified by probe)

| Shape | Probe result |
| --- | --- |
| Locals: zero-init to case 0 with zero payload, initialized, payload read, payload write, `?place.case` write-through, whole copy in and out, replacement by a payloadless case | all targets, both profiles |
| Globals: zero-init, static-initialized, replacement, payload write, `?g.case`, copy to and from locals | all targets, both profiles |
| Array elements: zero-init, element replacement, payload write, `?a[i].case`, element copy, whole-array copy, global array copy, runtime index | all targets, both profiles |
| Record fields: zero-init, field replacement, payload write, `?b.r.case`, record copy, record literal with a tag field, omitted tag field zero (static and runtime) | all targets, both profiles |
| Tags in tags: nested guard chain, nested payload write, inner replacement, `?o.inner` typed pointer, whole copy, static nested initializer, three-level generic nesting | all targets, both profiles |
| Through pointers: `sel (@p).c`, `(@p).c` read and write, `?(@p).c`, `@p = T.c{}`, `val v = @p`, pointer to array and record of tags | all targets, both profiles |
| Replacement zeroing: larger case by smaller case clears the inactive suffix, poisoned discriminator-payload gap is re-zeroed, payloadless replacement zeroes the full extent, 4096-byte payload replaced by a `u16` | byte-level, all targets |
| RHS-first: `a[i] = f(?i)` writes the element the call selected, `r = T.c{peek(?r) + 1}` reads the old payload, `@pick(..) = f(..)` runs the RHS before the destination call, `s = @ps` self-copy through an alias, `o = Outer.inner{@q}` with `q` into `o`'s own payload | all targets, both profiles |
| `#[packed]`: size `D + M`, align 1, payload at offset `D`, unaligned `u64`/`u32` payload loads and stores at array stride 17 and 65, nested record payload writes, whole copies through globals, `u16` and `u32` discriminators, packed record holding a natural tag and a packed tag | all targets, both profiles |
| `#[align(N)]`: object align raised and size rounded, locals aligned in a frame with three saved registers (the riscv64 fix below), globals, array elements and record fields aligned | all targets, both profiles |
| Discriminator widths `u8`, `u16`, `u32`, `u64`: offset zero, payload at `align_up(D, PAlign)`, gap zero | byte-level, all targets |
| Payload kinds: `f32x4` (align 16, offset 16), `[4096]u8`, `f64`, `[3]i64`, union (raw bytes preserved), nested tag | all targets, both profiles |
| By-value ABI: tags of 2, 9, 16, 24, 32 bytes and a nested tag passed, returned and passed nine at a time | all targets, both profiles |
| Static values: `val` tags, arrays of tags, records with tag fields, packed static tag; bytes identical to the runtime construction | all targets, both profiles |

## Found and fixed during probing

- riscv64 frame slots below an odd saved-register area lost their 16-byte
  alignment; an `#[align(16)]` tag local surfaced it. Fixed in
  `src/lang/target/isa/riscv/register.mach` (d1376627).
- The address-of rejection for a payload of a `#[packed]` tag named the place
  a record field. Fixed in `src/lang/fe/sema/infer.mach` (28ecf380).

## Missing before this lane

No storage shape in the roadmap row required a lowering change. What was
missing was the runtime evidence: the only tag codegen test asserted that
fixtures compile, and no test observed bytes, replacement zeroing, evaluation
order, packed access or alignment on a running program. This lane adds those
tests, one per roadmap item, on all three ISAs, plus a mutation control that
deletes the construction `memzero` and shows the zeroing test fail.

## Observations outside L3

- `sel p.c` and `p.c` with `p: *T` are rejected; the tag place rules require
  the explicit `(@p).c`, while record member access accepts the implicit
  dereference. Both `sel` and the payload place refuse consistently, so this
  is a lexical-place decision for the guard lane (#3219), not a lowering gap.
- `res` and `opt` are reserved as canonical names in this build, so a
  std-free fixture that needs a generic tag declares its own name.
