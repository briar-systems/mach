# mach.lang.me.pass.shiftbound

the one home of the shift saturation (#3756, #3885, #3887). every scalar
shift leaves this pass marked count-bounded, its count below its operand's
width: a count the range analysis proves keeps the bare shift, and any other
count is saturated here in the ir, branch-free, around the mask
`m = mask_lt_u(count, W)`, all ones exactly when the count is in range. a
logical shift keeps its value only under `m` and shifts by `count & (W-1)`,
an arithmetic one shifts by `(count | ~m) & (W-1)`, which is W - 1 out of
range and fills with the sign. `m` is 0 or all ones, so it may and the value
before the shift as well as the result after it, and it goes into an
adjacent single-use and with a constant wherever there is one, so a mask of
a count the loop does not change is loop-invariant and licm hoists it with
the and. the mark is recomputed from scratch on each run, so it never
outlives its proof, and a count this pass saturated proves on a later run

## fun run_in

```mach
pub fun run_in(m: *ir.Module, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

