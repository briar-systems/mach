# mach.lang.retention

## def RetainerId

```mach
pub def RetainerId: u32
```

## val RETAINER_SESSION

```mach
pub val RETAINER_SESSION: RetainerId = 0
```

the retainer a session records under until a caller names another

## rec Held

```mach
pub rec Held;
```

## rec Retention

```mach
pub rec Retention;
```

active:  the retainer a build records under
rounds:  each load is its own round; false while a caller keeps one round open across builds
modules: how many retainers hold each module
files:   how many held modules each file backs
pending: modules released since the last retirement, possibly held again since

## fun init

```mach
pub fun init(a: *A.Allocator) res[Retention, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(t: *Retention);
```

## fun open

```mach
pub fun open(t: *Retention) res[RetainerId, fail.Fail];
```

a new retainer holding nothing, in a free slot when one exists

## fun record

```mach
pub fun record(t: *Retention, id: RetainerId, stable: module.StableModuleId, file: source.FileId, rejected: bool) err[fail.Fail];
```

a module the open round loaded; held from now on, so nothing retires it under the build

rejected: the load that reached it was rejected

## fun reject

```mach
pub fun reject(t: *Retention, id: RetainerId);
```

the rejected flag of the open round, for a load that ended before it reached any module

## fun end_round

```mach
pub fun end_round(t: *Retention, id: RetainerId, finished: bool) err[fail.Fail];
```

close the open round. an accepted round keeps exactly what it loaded. a rejected or unfinished one
keeps the last accepted round's modules and its own, and nothing older. invariant: the held set after
any run of rejected rounds is the one the last of them alone leaves, so typing an import one character
at a time cannot pile modules up

finished: every build of the round ran to its end

## fun release

```mach
pub fun release(t: *Retention, id: RetainerId) err[fail.Fail];
```

a retainer that holds nothing any more; its modules no other retainer holds are released

## fun retire

```mach
pub fun retire(t: *Retention, db: *query.QueryDb) err[fail.Fail];
```

retire every product of the released modules no retainer holds again, and of the files no held module
is backed by. a failure leaves the released modules pending for the next call

