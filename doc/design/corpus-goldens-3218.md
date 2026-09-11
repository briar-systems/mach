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
