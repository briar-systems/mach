# lang/me/pass

Optimization passes over the IR. Each pass is a pure function from an IR module to an IR module. Pass order and composition are defined in `../pipeline.mach`.

## Files

- `mem2reg.mach` — promotes stack allocations into SSA values where their address is never observed.
- `dce.mach` — dead code elimination. Removes instructions whose results are unused and have no side effects.
- `inline.mach` — function inlining driven by size and call-site heuristics.

Passes are independently importable and testable; they are not required to go through the pipeline to run.
