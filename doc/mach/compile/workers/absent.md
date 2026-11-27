[mach](../../README.md) / [compile](../README.md) / [workers](README.md) / **absent**

# mach.compile.workers.absent

the crew of a host with no threads: there is none to open. `open` refuses as it does where the
host has no thread left to give, and a crew that never opened takes no task and finishes none

Source: [`src/compile/workers/absent.mach`](../../../../src/compile/workers/absent.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Crew`](#crew) | `rec` | a crew that holds nothing |

| On `Crew` | |
| :--- | :--- |
| [`open`](#open) | refuse the crew: the host has no thread to run one on |
| [`submit`](#submit) | refuse the task: no crew is open to take it |
| [`finished`](#finished) | none: no task was taken, so none finishes |
| [`finished_now`](#finished_now) | none: no task was taken, so none finished |
| [`close`](#close), [`dnit`](#dnit) | |

---

## Crew

```mach
pub rec Crew {
    started: u32;
}
```

a crew that holds nothing

| Field | Type | |
| :--- | :--- | :--- |
| `started` | `u32` | the threads the pool runs, always none |

### open

```mach
pub fun open(c: *Crew, a: *std.allocator.Allocator, n: u32, units: usize, queued: usize) bool
```

refuse the crew: the host has no thread to run one on

### submit

```mach
pub fun submit(c: *Crew, index: u32, ctx: ptr, run: fun(ptr)) bool
```

refuse the task: no crew is open to take it

### finished

```mach
pub fun finished(c: *Crew) opt[u32]
```

none: no task was taken, so none finishes

### finished_now

```mach
pub fun finished_now(c: *Crew) opt[u32]
```

none: no task was taken, so none finished

### close

```mach
pub fun close(c: *Crew)
```

### dnit

```mach
pub fun dnit(c: *Crew)
```

