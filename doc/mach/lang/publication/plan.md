# mach.lang.publication.plan

## rec Destination

```mach
pub rec Destination;
```

## rec Home

```mach
pub rec Home;
```

a project root homed at its control home, with the cleaned absolute path
it was opened by: a plan given one roots every destination directory under
that path by descent from it, so each keeps its lock, claims and backup
under the home keyed by its relative path. directories outside the path
are rooted plainly, as they are without a home

## rec Plan

```mach
pub rec Plan;
```

## fun init

```mach
pub fun init(out: *Plan, alloc: *A.Allocator) err[txn.Error];
```

## fun init_homed

```mach
pub fun init_homed(out: *Plan, alloc: *A.Allocator, home: *Home) err[txn.Error];
```

`home` is borrowed for the plan's lifetime; nil plans without one

## fun absolute_clean

```mach
pub fun absolute_clean(alloc: *A.Allocator, value: str) res[str, outcome.Fail];
```

a path rooted at the working directory and cleaned: the home's path and a
planned directory are compared in one coordinate system, so a relative
project spelling (`mach doc .`) is contained the same as an absolute one

## fun add

```mach
pub fun add(plan: *Plan, name: str, create_parents: bool) res[*Destination, outcome.Fail];
```

## fun seal

```mach
pub fun seal(plan: *Plan) err[txn.Error];
```

## fun dnit

```mach
pub fun dnit(plan: *Plan) err[txn.Error];
```

