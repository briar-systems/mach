[mach](../../README.md) / [compile](../README.md) / [wake](README.md) / **absent**

# mach.compile.wake.absent

the signal of a host with one thread: no thread waits on it, and none could wake one that did

Source: [`src/compile/wake/absent.mach`](../../../../src/compile/wake/absent.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Wake`](#wake) | `rec` |  |

| On `Wake` | |
| :--- | :--- |
| [`make`](#make) | |
| [`all`](#all) | nothing: no thread waits |
| [`wait`](#wait) | refuse the wait: the one thread would wait on itself |

---

## Wake

```mach
pub rec Wake {
    waits: u32;
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

nothing: no thread waits

### wait

```mach
pub fun wait(w: *Wake, lock: *std.sync.mutex.Mutex) bool
```

refuse the wait: the one thread would wait on itself

| Parameter | |
| :--- | :--- |
| **returns** | false: nothing waited, and what the caller waited for cannot come |

