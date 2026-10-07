# mach.lang.me.opt.constfold

constant folding. an operation whose operands are constants becomes the
constant it computes: integer and float arithmetic and comparisons, negation
and complement, width changes, a float narrowing, a widening or high
multiply, a select on a constant condition or between equal choices, a
lane-bound mask, and a phi whose incomings agree. a fold that would trap or
overflow on the machine (a division by zero, MIN / -1) is declined and left
to run. it repeats until nothing folds, then turns a branch on a constant
into a jump and drops the phi incomings from the edge it removed

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

