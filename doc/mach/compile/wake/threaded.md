[mach](../../README.md) / [compile](../README.md) / [wake](README.md) / **threaded**

# mach.compile.wake.threaded

a signal threads wait on under a lock: std.sync.condition

Source: [`src/compile/wake/threaded.mach`](../../../../src/compile/wake/threaded.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Wake`](#wake) | `rec` |  |

| On `Wake` | |
| :--- | :--- |
| [`make`](#make) | |
| [`all`](#all) | wake every thread waiting on `w` |
| [`wait`](#wait) | wait under `lock`, which the caller holds, until another thread wakes `w` |

---

## Wake

```mach
pub rec Wake {
    woken: std.sync.condition.Condition;
}
```

### make

```mach
pub fun make() Wake
```

### all

```mach
pub fun all(w: *Wake)
```

wake every thread waiting on `w`

### wait

```mach
pub fun wait(w: *Wake, lock: *std.sync.mutex.Mutex) bool
```

wait under `lock`, which the caller holds, until another thread wakes `w`

| Parameter | |
| :--- | :--- |
| **returns** | true: the wait ended, and the caller looks again at what it waited for |

