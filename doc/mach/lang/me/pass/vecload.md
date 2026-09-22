# mach.lang.me.pass.vecload

## fun run

```mach
pub fun run(m: *ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

rewrites a vector literal whose lanes are consecutive scalar loads from one
pointer at stride one into one vector load of that pointer: `f32x4{p[i],
p[i + 1], p[i + 2], p[i + 3]}` becomes the load the reinterpret
`@((?p[i]):~*f32x4)` spells (#3753). a machine target lowers a vector
literal to a stack slot written lane by lane and read back whole, so the
shape recognized here is that slot: every use of the slot is one of N lane
stores through a gep and exactly one whole load, every stored value is a
load of the lane type through a gep of one base at an index `c0 + k` for
lane k, and nothing between the first lane load and the whole load writes
memory or calls. the whole load is retargeted at lane 0's address; the
slot, its stores and the lane loads (each read by its store alone, so a
load is never discarded here that anything else could see) go, and the
addresses and index adds left unread go with a later dce. a vector's size
is its lanes packed with no padding at any width, so the bytes agree at
every lane count on every target

