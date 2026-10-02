# mach.lang.be.codegen.ctvalidate

the early constant-time walk over mir, run before legalization on every
oblivious function. the leakage model has three channels a secret may not
reach: the control-flow trace (a conditional branch, a branch-implemented
select, an indirect target), the memory-address trace (a load or store base
or index), and operand-dependent latency (divide, remainder and every float
operation always; a multiply unless the machine model declares a
constant-time row for its exact cell, and a register-count shift unless it
declares `ct_trust_var_shift`). taint is a two-point lattice per value
re-derived as a monotone fixpoint; `MIR_DECLASSIFY` is the
only downgrade; an opcode, operand kind or register outside the catalog is
refused, never treated as public. `ctwalk` repeats the same check over the
emitted instruction stream after every late expansion

## fun validate

```mach
pub fun validate(tgt: *resolved.Target, m: *codegen_mir.MirModule) err[fail.Fail];
```

