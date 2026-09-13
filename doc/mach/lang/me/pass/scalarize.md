# mach.lang.me.pass.scalarize

## rec ScalarizeSite

```mach
pub rec ScalarizeSite;
```

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

## fun expand_lane_ops

```mach
pub fun expand_lane_ops(m: *ir.Module, tgt: *target.Target) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `expand_lane_ops_in` over one

## fun expand_lane_ops_in

```mach
pub fun expand_lane_ops_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun expand_gap_ops

```mach
pub fun expand_gap_ops(m: *ir.Module, tgt: *target.Target) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `expand_gap_ops_in` over one

## fun expand_gap_ops_in

```mach
pub fun expand_gap_ops_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

## fun run

```mach
pub fun run(m: *ir.Module, tgt: *target.Target) res[bool, fail.Fail];
```

a run that owns its workspace; the pipeline runs `run_in` over one

## fun run_in

```mach
pub fun run_in(m: *ir.Module, tgt: *target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

