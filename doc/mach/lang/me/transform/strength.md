# mach.lang.me.transform.strength

## fun run_in

```mach
pub fun run_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

induction-variable strength reduction (#3347). a value that is an affine
function `a * iv + b` of a loop's induction variable, with a constant `a` and
a loop-invariant `b`, is stepped by `a * step` each iteration instead of
recomputed. every operation involved is a wrapping add or multiply at the
iv's own width, so the stepped value equals the recomputed one modulo 2^n on
every iteration: no overflow case differs and nothing that can trap is read.
an address `base + index * size` becomes a pointer the latch steps, or keeps
a scaled index over a base hoisted to the preheader where the target folds
the scale into the access; an integer multiply becomes an integer iv

