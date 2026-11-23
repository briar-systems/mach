[mach](../README.md) / [session](README.md) / **retention**

# mach.session.retention

which modules a session keeps products for between builds. every party that keeps products
alive across builds (a build path, an editor root, an engine plan) holds a retainer, and a
module's products stay while any retainer holds it. a build records the modules it loaded under
its retainer in rounds; a module no retainer holds any more is released, and its products are
retired the next time the session is idle

Source: [`src/session/retention.mach`](../../../src/session/retention.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.session.module`](module.md), [`mach.session.query`](query.md).

| Type | | |
| :--- | :--- | :--- |
| [`RetainerId`](#retainerid) | `def` | a retainer's slot in the retention table |
| [`Retention`](#retention) | `rec` | the retainers of a session and how many of them hold each module and file |

| On `RetainerId` | |
| :--- | :--- |
| [`RETAINER_SESSION`](#retainerid) | the retainer a session records under until a caller names another |

| On `Retention` | |
| :--- | :--- |
| [`init`](#init) | a retention table holding nothing, with the session's retainer open as RETAINER_SESSION |
| [`dnit`](#dnit) | release every live retainer and the table's storage, retiring nothing |
| [`acquire`](#acquire) | a new retainer holding nothing, in a free slot when one exists |
| [`record`](#record) | a module the open round loaded; held from now on, so nothing retires it under the build |
| [`reject`](#reject) | the rejected flag of the open round, for a load that ended before it reached any module |
| [`end_round`](#end_round) | close the open round |
| [`release`](#release) | a retainer that holds nothing any more; its modules no other retainer holds are released |
| [`retire`](#retire) | retire every product of the released modules no retainer holds again, and of the files no held module is backed by |

---

## RetainerId

```mach
pub def RetainerId: u32
```

a retainer's slot in the retention table

| Name | Value | |
| :--- | :--- | :--- |
| `RETAINER_SESSION` | `0` | the retainer a session records under until a caller names another |

## Retention

```mach
pub rec Retention {
    a:         *std.allocator.Allocator;
    retainers: Vector[Retainer];
    active:    RetainerId;
    modules:   Map[u32, u32];
    files:     Map[u32, u32];
    pending:   Vector[Held];
}
```

the retainers of a session and how many of them hold each module and file

| Field | Type | |
| :--- | :--- | :--- |
| `active` | `RetainerId` | the retainer a build records under |
| `modules` | `Map[u32, u32]` | how many retainers hold each module |
| `files` | `Map[u32, u32]` | how many held modules each file backs |
| `pending` | `Vector[Held]` | modules released since the last retirement, possibly held again since |

### init

```mach
pub fun init(a: *std.allocator.Allocator) res[Retention, mach.base.fail.Fail]
```

a retention table holding nothing, with the session's retainer open as RETAINER_SESSION

| Parameter | |
| :--- | :--- |
| `a` | the allocator the table and its retainers draw from |
| **returns** | the table, released with `dnit`, or a failure when the first retainer cannot be added |

### dnit

```mach
pub fun dnit(t: *Retention)
```

release every live retainer and the table's storage, retiring nothing

### acquire

```mach
pub fun acquire(t: *Retention) res[RetainerId, mach.base.fail.Fail]
```

a new retainer holding nothing, in a free slot when one exists

### record

```mach
pub fun record(t: *Retention, id: RetainerId, stable: mach.session.module.StableModuleId, file: mach.base.source.FileId, rejected: bool) err[mach.base.fail.Fail]
```

a module the open round loaded; held from now on, so nothing retires it under the build

| Parameter | |
| :--- | :--- |
| `rejected` | the load that reached it was rejected |

### reject

```mach
pub fun reject(t: *Retention, id: RetainerId)
```

the rejected flag of the open round, for a load that ended before it reached any module

### end_round

```mach
pub fun end_round(t: *Retention, id: RetainerId, finished: bool) err[mach.base.fail.Fail]
```

close the open round. an accepted round keeps exactly what it loaded. a rejected or unfinished one
keeps the last accepted round's modules and its own, and nothing older. invariant: the held set after
any run of rejected rounds is the one the last of them alone leaves, so typing an import one character
at a time cannot pile modules up

| Parameter | |
| :--- | :--- |
| `finished` | every build of the round ran to its end |

### release

```mach
pub fun release(t: *Retention, id: RetainerId) err[mach.base.fail.Fail]
```

a retainer that holds nothing any more; its modules no other retainer holds are released

### retire

```mach
pub fun retire(t: *Retention, db: *mach.session.query.QueryDb) err[mach.base.fail.Fail]
```

retire every product of the released modules no retainer holds again, and of the files no held module
is backed by. a failure leaves the released modules pending for the next call

