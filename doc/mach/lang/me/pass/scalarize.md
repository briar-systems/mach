# mach.lang.me.pass.scalarize

## rec ScalarizeSite

```mach
pub rec ScalarizeSite;
```

loc is the operation's own location when it has one, else the function's
declaration. lane_bits is 0 when the lane shape is outside the vocabulary,
and from_bits is the operand lane width, which a conversion changes

## fun sites

```mach
pub fun sites(m: *me_ir.Module, tgt: *resolved.Target, out: *Vector[ScalarizeSite]) err[A.Error];
```

every operator the target scalarizes, one site per operation, in function
and block order: the sites `simd = "require"` refuses and the default warns at

## fun detect_undeclared

```mach
pub fun detect_undeclared(m: *me_ir.Module, tgt: *resolved.Target, first: *ScalarizeSite) u32;
```

operators whose lane shape the target's catalog names neither packed nor scalar

## val LANES

```mach
pub val LANES: pass.Pass = pass.Pass;
```

the lane-operation expansion the pipeline schedules

## val GAPS

```mach
pub val GAPS: pass.Pass = pass.Pass;
```

the gap expansion the pipeline schedules

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

