# mach.lang.me.pass.scalarize

## rec ScalarizeSite

```mach
pub rec ScalarizeSite;
```

loc is the operation's own location when it has one, else the function's declaration

## fun detect

```mach
pub fun detect(m: *ir.Module, tgt: *target.Target, first: *ScalarizeSite) u32;
```

operators the target scalarizes (declared scalar rows, lane counts past the
register, and undeclared shapes alike), with the first site named

## fun detect_undeclared

```mach
pub fun detect_undeclared(m: *ir.Module, tgt: *target.Target, first: *ScalarizeSite) u32;
```

operators whose lane shape the target's catalog names neither packed nor scalar

## fun expand_lane_ops_in

```mach
pub fun expand_lane_ops_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun expand_gap_ops_in

```mach
pub fun expand_gap_ops_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun run_in

```mach
pub fun run_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

