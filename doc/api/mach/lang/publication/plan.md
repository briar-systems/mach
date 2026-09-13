# mach.lang.publication.plan

## rec Destination

```mach
pub rec Destination;
```

## rec Plan

```mach
pub rec Plan;
```

## fun init

```mach
pub fun init(out: *Plan, alloc: *A.Allocator) err[txn.Error];
```

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

