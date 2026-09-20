# mach.lang.me.pass.vecwiden

## fun run

```mach
pub fun run(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

rewrites a vector literal whose lanes are the same-kind extensions of one
source vector's consecutive lanes into the lane-halving extension of that
source: `i32x4{v[0]::i32, .., v[3]::i32}` over an `i16x8` becomes one
`vec_widen_s` of `v`'s low half, and lanes 4..7 its high half (#3738). a
machine target lowers a vector literal to a stack slot written lane by lane
and read back whole, so the shape recognized here is that slot: every use
of the slot is one of N lane stores through a gep and exactly one whole
load, and every stored value is the extension of the source's lane at the
literal's position. the load becomes the widen op, the slot and its stores
go, and the extractions and extensions it leaves unread go with a later
dce. the rewrite happens only where the target's catalog packs the cell, so
a target without a packed row keeps the lane path unchanged

