# mach.lang.me.pass.widenmul

## fun run

```mach
pub fun run(m: *ir.Module, tgt: *target.Target) res[bool, fail.Fail];
```

rewrites a multiply of two same-kind extensions of one type into the
widening multiply of the unextended operands: lane-wise on vectors wherever
the target's catalog packs that cell, and on scalars wherever the product is
wider than the target's ALU, where legalize realizes it through the target's
declared widening form (#3511). a truncation of that product to its operand
width then becomes the plain multiply, and a truncation of its shift by the
operand width becomes the high-half multiply, so the half a program selects
is one instruction and no double-width product is materialized. the
extensions stay in place for any other use, and a later dce drops the ones
nothing reads

