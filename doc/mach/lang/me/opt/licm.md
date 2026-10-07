# mach.lang.me.opt.licm

loop-invariant code motion. an instruction of a loop whose operands the
loop does not change moves to the loop's preheader when it has no effect,
cannot trap and is worth computing once. a load moves only from a constant
address in a global that no store or call in the loop may write, and an
address whose leading indices are invariant has that prefix hoisted on its
own. loops are visited innermost first, so a value hoisted from an inner
loop can leave its outer one too

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

