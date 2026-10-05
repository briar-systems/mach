# mach.lang.me.legalize.scalarize.sites

the operations the target scalarizes, one site per operation, read by the
checks that warn at or refuse them

## rec Site

```mach
pub rec Site;
```

loc is the operation's own location when it has one, else the function's
declaration. lane_bits is 0 when the lane shape is outside the vocabulary,
and from_bits is the operand lane width, which a conversion changes

## fun list

```mach
pub fun list(m: *me_ir.Module, tgt: *resolved.Target, out: *Vector[Site]) err[A.Error];
```

every operator the target scalarizes, one site per operation, in function
and block order: the sites `simd = "require"` refuses and the default warns at

## fun detect

```mach
pub fun detect(m: *me_ir.Module, tgt: *resolved.Target, first: *Site) u32;
```

operators the target scalarizes (declared scalar rows, lane counts past the
register, and undeclared shapes alike), with the first site named

## fun detect_undeclared

```mach
pub fun detect_undeclared(m: *me_ir.Module, tgt: *resolved.Target, first: *Site) u32;
```

operators whose lane shape the target's catalog names neither packed nor scalar

