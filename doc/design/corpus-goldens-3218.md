# Why the layer B goldens changed on the v5 integration branch

This branch was rebuilt on 2026-09-11 as language-only (roadmap Track A): the
candidate era's inlining, object cache and other Track B work no longer sit on
it, so dev's goldens are the baseline and only language-caused moves remain.

## riscv64 frame alignment fix (L3)

The L3 probe surfaced that riscv64 frame slots below an odd saved-register
area lost their 16-byte alignment; the fix keeps them aligned. That moved 45
riscv64 layer B goldens and nothing on any other target. Evidence, computed
over every changed golden with frame-offset immediates normalized: 41 goldens
keep their line count and their instruction multiset exactly, so only the
offset each slot lands on changed. The other four (`vec/vec_i8x16`,
`vec/vec_u8x16`, `vec/vec_f32x5`, `vec/vec_lane_ops`) show the same frame
growth crossing the 12-bit immediate limit: a direct `ld t2, off(s0)` becomes
`lui`/`addiw`/`add` to materialize the address and the same load through it.
Same loads and stores, larger-offset addressing form. No instruction is added
or removed for any other reason.

## riscv64 goldens after the dev sync carrying #3270 (2026-09-11)

Merging dev at `da4fe7146` (cross-module inlining, #3110, which re-blessed every
riscv64 layer B golden on dev) into `feat/3218` conflicted on 45 riscv64 goldens
that the L3 frame-alignment fix had moved on this branch. Resolution: dev's
content was taken for every conflicting golden, the merged compiler was rebuilt,
and riscv64 layer B was re-run: 37 goldens differed from dev's, all in the same
shape as the L3 move. Evidence, computed over every changed golden with frame
offsets normalized in memory operands (`off(sp)`, `off(s0)`) and in the
`addi`/`addiw`/`lui`/`li` immediates that materialize a slot address: all 37
keep their line count and their instruction multiset exactly. No instruction is
added or removed; only the slot each frame reference lands on changed, which is
the L3 fix applied to dev's inlined bodies. The remaining 55 goldens matched
dev's byte for byte. Blessed on that evidence.
