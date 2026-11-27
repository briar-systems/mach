[mach](../README.md) / [compile](README.md) / **wake**

# mach.compile.wake

the wait one thread ends for another, where the host has a second thread. both members export one
signal, and on a host with one thread nothing can end a wait, so it is refused where it would start

Source: [`src/compile/wake.mach`](../../../src/compile/wake.mach)

| Forwards | |
| :--- | :--- |
| [`Wake`](#wake), [`make`](#make), [`all`](#all), [`wait`](#wait) | |

---

## Forwards

### Wake

Forwards the first of these whose condition holds, and they differ.

Where `capability.HAS_THREADS`: [`mach.compile.wake.threaded.Wake`](wake/threaded.md#wake).

```mach
pub rec Wake {
    woken: std.sync.condition.Condition;
}
```

Otherwise: [`mach.compile.wake.absent.Wake`](wake/absent.md#wake).

```mach
pub rec Wake {
    waits: u32;
}
```

### make

Forwards the first of these whose condition holds:

- [`mach.compile.wake.threaded.make`](wake/threaded.md#make), where `capability.HAS_THREADS`
- [`mach.compile.wake.absent.make`](wake/absent.md#make), otherwise

```mach
pub fun make() Wake
```

### all

Forwards the first of these whose condition holds:

- [`mach.compile.wake.threaded.all`](wake/threaded.md#all), where `capability.HAS_THREADS`
- [`mach.compile.wake.absent.all`](wake/absent.md#all), otherwise

```mach
pub fun all(w: *Wake)
```

wake every thread waiting on `w`

### wait

Forwards the first of these whose condition holds:

- [`mach.compile.wake.threaded.wait`](wake/threaded.md#wait), where `capability.HAS_THREADS`
- [`mach.compile.wake.absent.wait`](wake/absent.md#wait), otherwise

```mach
pub fun wait(w: *Wake, lock: *std.sync.mutex.Mutex) bool
```

wait under `lock`, which the caller holds, until another thread wakes `w`

