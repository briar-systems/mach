# L5 tag native ABI transport inventory (#3218)

Classification inventory for roadmap item L5 on feat/3218, recorded 2026-09-11
before any transport change, then annotated with what the probes found. Every
cell was established by classifying the tag through the real type derivation
in `src/lang/be/codegen/mir/abi.mach` (`classify_signature`) and the real
convention vtable selected from the target registry, in the unit test
`mach.lang.be.codegen.mir.abi:tag_transport_classifies_as_the_record_of_its_layout_on_every_convention`.
The runtime rows were then executed by freestanding fixtures on linux-x86_64,
linux-arm64 (qemu-aarch64) and linux-riscv64 (qemu-riscv64) in both profiles,
and the C rows by a clang-built C callee and caller of matching layout.

Contract: `doc/design/tagged-values.md`, "Initialization and representation"
(native ABI transfer preserves exact logical object extents separately from
carrier width and alignment) and "Secrecy, ownership and std".

## The classification functions

| Convention | Function | Fed by |
| --- | --- | --- |
| SysV64 | `mach.lang.target.abi.sysv.classify_arg` / `classify_return` (`src/lang/target/abi/sysv.mach`) | `aggregate_eightbyte_mask` (`src/lang/be/codegen/mir/abi.mach`): a leaf walk that puts the discriminator at eightbyte 0 as INTEGER and every case payload at the common payload offset, merging INTEGER over SSE within an eightbyte |
| Win64 | `mach.lang.target.abi.win64.classify_arg` / `classify_return` (`src/lang/target/abi/win64.mach`) | size alone: 1, 2, 4 or 8 bytes ride a register, everything else is the address of a caller copy |
| AAPCS64 | `mach.lang.target.abi.aapcs64.classify_arg` / `classify_return` (`src/lang/target/abi/aapcs64.mach`) | `hfa_info`: `hfa_walk` has no tag arm, so a tag is never an HFA; sizes up to 16 ride x0/x1 |
| LP64D, LP64 | `mach.lang.target.abi.riscv.classify_arg_v` / `classify_return_v` with `v_lp64d()` / `v_lp64()` (`src/lang/target/abi/riscv.mach`) | `agg_layout`: the strict leaf walk refuses a tag (a union is never flattened), so no float-register split; sizes up to 2×XLEN ride a0/a1 |
| ILP32D, ILP32 | the same functions with `v_ilp32d()` / `v_ilp32()` | the same rule with XLEN of 4 bytes |

A tag is classified as the record of its layout: the discriminator is an
integer leaf at offset zero, and the payload area is a union of every case.
That decides the two convention-specific questions the roadmap row asks:

- **AAPCS64 HFA.** An HFA needs every leaf to be a float of one width. The
  discriminator is an integer leaf, so no tag is an HFA whatever its payload;
  `hfa_info` reports zero members for `TF16`, `TF8` and `TFM` and they ride the
  general registers. clang agrees for the C spelling (`struct { unsigned char
  d; union { double p; } u; }` arrives in x0/x1).
- **SysV mixed eightbytes.** `TF16` (`tag { a; b: f64; }`) is INTEGER, SSE: the
  discriminator eightbyte rides rdi and the payload eightbyte rides xmm0, and
  the return uses rax and xmm0. `TF8` (`f32` payload at offset 4) shares the
  discriminator's eightbyte, so it is INTEGER. `TFM` (`f64` and `i64` cases)
  merges INTEGER over SSE in the payload eightbyte, so it rides two general
  registers. clang and gcc produce the same three answers for the C spelling.
- **RISC-V float flattening.** The psABI passes a struct of one float and one
  integer in a float and an integer register, but never flattens a union. A
  tag's payload area is a union even with a single payload case, so `TF16`
  rides a0/a1 under lp64d. A C caller must spell the payload as a union to
  match; a bare `struct { unsigned char d; double p; }` is a different ABI
  type and is not a matching layout.

## Shapes

| Shape | Declaration | Size | Align |
| --- | --- | --- | --- |
| P1 | `tag P1: u8 { a; b; }` | 1 | 1 |
| T3 | `tag T3: u8 { a; b: [2]u8; }` | 3 | 1 |
| S8 | `tag S8: u8 { a; b: u32; }` | 8 | 4 |
| T9 | `tag T9: u8 { a; b: [8]u8; }` | 9 | 1 |
| T16 | `tag T16: u8 { a; b: i64; }` | 16 | 8 |
| O24 | `tag O24: u8 { a; b: [2]i64; c: u8; }` | 24 | 8 |
| NEST | `tag Nest: u8 { a; b: S8; }` | 12 | 4 |
| PK9 | `#[packed] tag PK9: u8 { a; b: u64; }` | 9 | 1 |
| AL16 | `#[align(16)] tag AL16: u8 { a; b: u8; }` | 16 | 16 |
| TF16 | `tag TF16: u8 { a; b: f64; }` | 16 | 8 |
| TF8 | `tag TF8: u8 { a; b: f32; }` | 8 | 4 |
| TFM | `tag TFM: u8 { a; b: f64; c: i64; }` | 16 | 8 |

## Classification per convention

Each cell is `argument / return`. A register cell names the carriers and the
carrier width; the logical bytes each carrier moves are the object's remaining
bytes at that offset (`abi.piece_memory_width`), so a 3-byte tag in an 8-byte
register moves 3 bytes and a 9-byte tag in two 8-byte registers moves 8 then 1.
`memory` is the by-value stack class, `byref` is the address of a caller copy
in a register, `sret` is a hidden result pointer.

| Shape | SysV64 | Win64 | AAPCS64 | LP64D / LP64 | ILP32D / ILP32 |
| --- | --- | --- | --- | --- | --- |
| P1 | rdi(8) / rax(8) | rcx(8) / rax(8) | x0(8) / x0(8) | a0(8) / a0(8) | a0(4) / a0(4) |
| T3 | rdi(8) / rax(8) | byref rcx / sret | x0(8) / x0(8) | a0(8) / a0(8) | a0(4) / a0(4) |
| S8 | rdi(8) / rax(8) | rcx(8) / rax(8) | x0(8) / x0(8) | a0(8) / a0(8) | a0,a1(4) / a0,a1(4) |
| T9 | rdi,rsi(8) / rax,rdx(8) | byref / sret | x0,x1(8) / x0,x1(8) | a0,a1(8) / a0,a1(8) | byref a0 / sret a0 |
| T16 | rdi,rsi / rax,rdx | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |
| O24 | memory(24) / sret rax | byref / sret | byref x0 / sret x8 | byref a0 / sret a0 | byref / sret |
| NEST | rdi,rsi / rax,rdx | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |
| PK9 | rdi,rsi / rax,rdx | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |
| AL16 | **rdi / rax** (was rdi,rsi / rax,rdx) | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |
| TF16 | rdi,xmm0 / rax,xmm0 | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |
| TF8 | rdi / rax | rcx / rax | x0 / x0 | a0 / a0 | a0,a1(4) / a0,a1(4) |
| TFM | rdi,rsi / rax,rdx | byref / sret | x0,x1 / x0,x1 | a0,a1 / a0,a1 | byref / sret |

The soft-float LP64 and ILP32 columns equal their D-extension siblings for
every shape, because no tag ever reaches a float register on RISC-V.

## Found and fixed during probing

- **SysV64 padding-only eightbyte.** `AL16` is 16 bytes with data in its first
  eightbyte only; the second eightbyte is padding and classifies NO_CLASS. The
  System V ABI assigns registers only to INTEGER and SSE eightbytes, and gcc
  and clang both pass the C spelling in rdi alone with the next argument in rsi,
  and return it in rax alone. mach's classifier counted the empty eightbyte as
  INTEGER, spent rsi on it and returned through rdx: a C callee would have read
  its next argument from the wrong register and a mach caller of a C callee
  would have copied rdx's leftovers into the object's tail padding. Fixed:
  `aggregate_eightbyte_mask` now reports padding-only eightbytes
  (`abi.EB_EMPTY_LO` / `abi.EB_EMPTY_HI`), `sysv.classify_arg` and
  `classify_return` build one piece per populated eightbyte, and the store
  side of `materialize_pieces` zeroes every logical byte no piece delivers so
  the tag's padding stays zero through the round trip. The same rule covers an
  over-aligned record.

- **Secret content was modelled as a secret home.** An aggregate's vreg in MIR
  is the address of its home, and the lowering seeded that vreg secret whenever
  the parameter or call result was outer-secret, so every field read of a
  by-value `^Rec` parameter and every copy of a `^Tag` inside an `#[oblivious]`
  function was refused as a secret-dependent memory address. Meanwhile a tag
  whose payload is `^u64` reached the argument registers straight from memory,
  so the register holding the secret payload carried public provenance. Fixed:
  `type.contains_secret` computes the union of a type's secret byte ranges
  (the type itself, an element, a field or a case payload), call arguments,
  returned values and parameters carry it, aggregate homes are never seeded
  secret, and the transport paths run with the object's secrecy so every
  carrier, caller copy and temporary between them is secret: the vreg each
  piece already loads through before its register now takes the object's
  secrecy instead of the public default. The
  discriminator load stays public because loads take their secrecy from the
  loaded type, which is what lets `sel` on a secret-payload tag branch in an
  oblivious function while a branch on the payload itself is still refused.

## Already correct (verified by probe)

- The carrier machinery keeps logical and carrier extents apart: a piece moves
  `piece_memory_width` bytes out of the object, an outgoing carrier narrower
  than its register is loaded through a zeroed carrier home, and an incoming
  carrier is stored into an owned home of the carrier extent that the logical
  copy then reads by the type's size. The runtime fixture
  `tag_transport_preserves_logical_extents_through_wider_carriers_on_every_native_backend`
  drives every shape above through a poisoned stack on the three Linux ISAs
  in both profiles: the callee never sees carrier padding as payload, a
  returned carrier stored into a packed neighbour or an array element leaves
  the adjacent bytes alone, and gap, inactive-suffix and tail bytes are zero
  after the round trip.
- Win64, AAPCS64, LP64D and the RV32 forms classify every shape as the table
  says, and the C controls agree: a clang-built C callee and C caller of
  matching layout exchange all twelve shapes with mach on linux-x86_64,
  linux-arm64, linux-riscv64 and windows-x86_64 (under wine), in both
  profiles, including the argument after an over-aligned tag. A C struct of
  9 bytes and mach's 9-byte tag agree on two 8-byte carriers with one
  logical byte in the second, which is the logical-versus-carrier distinction
  the roadmap row asks the controls to draw.
- Outer-secret tags copy by the fixed logical extent: `fun c(v: ^T) ^T { val u:
  ^T = v; ret u; }` lowers to a whole-object copy with no case test, and an
  oblivious body containing it is accepted, while `sel` on a `^T` is refused
  by sema.

## Not covered

- Darwin has no lane on this host; its SysV64 and AAPCS64 vtables are the
  Linux ones, so the classification table applies, but no fixture executed
  there.
- The RV32 forms have no execution engine, so their column is classification
  and unit-level only.
- The Windows control runs under wine, which is faithful for register and
  stack argument placement but not a Windows kernel.
