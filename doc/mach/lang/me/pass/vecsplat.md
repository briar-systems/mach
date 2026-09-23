# mach.lang.me.pass.vecsplat

## fun run

```mach
pub fun run(m: *ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

rewrites a vector shift whose count is a literal with the same value in
every lane into the shift by that one scalar: `v >> u32x4{k, k, k, k}`
becomes `v >> k`, the uniform count every baseline set shifts by in one
instruction (#3740). the source has no vector-by-scalar shift, so this is
the only producer of the uniform form. a lane is the same when every lane
stores one ssa value or equal constants. a machine target lowers a vector
literal to a stack slot written lane by lane and read back whole, so the
shape recognized there is that slot: every use of the slot is one of N lane
stores through a gep and exactly one whole load after the last store in the
same block, the shape vecwiden and vecload read. a target that builds a
vector from its lanes spells the literal as one vec.build. a slot whose load
nothing else reads goes with its stores

