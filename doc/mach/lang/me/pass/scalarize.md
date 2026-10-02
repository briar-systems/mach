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
pub fun sites(m: *me_ir.Module, tgt: *lang_target.Target, out: *Vector[ScalarizeSite]) err[std_allocator.Error];
```

every operator the target scalarizes, one site per operation, in function
and block order: the sites `simd = "require"` refuses and the default warns at

## fun detect

```mach
pub fun detect(m: *me_ir.Module, tgt: *lang_target.Target, first: *ScalarizeSite) u32;
```

operators the target scalarizes (declared scalar rows, lane counts past the
register, and undeclared shapes alike), with the first site named

## fun detect_undeclared

```mach
pub fun detect_undeclared(m: *me_ir.Module, tgt: *lang_target.Target, first: *ScalarizeSite) u32;
```

operators whose lane shape the target's catalog names neither packed nor scalar

## fun expand_lane_ops_in

```mach
pub fun expand_lane_ops_in(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun expand_gap_ops_in

```mach
pub fun expand_gap_ops_in(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun run_in

```mach
pub fun run_in(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

