# mach.lang.be.codegen.extfold

drops the widen of a value the machine already holds extended: a widening
load zero-fills its general register, a zext or sext fills to its width, and
an immediate move fills to its width, so a zext or sext that reads such a
value at or above the width it was extended from and writes at or below the
width it was extended to is a move, and one that writes past a whole-register
fill reads the whole register instead (#3660)

## fun run

```mach
pub fun run(tgt: *target.Target, m: *mir.MirModule) err[fail.Fail];
```

