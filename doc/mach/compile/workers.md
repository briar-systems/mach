[mach](../README.md) / [compile](README.md) / **workers**

# mach.compile.workers

the threads a codegen pass compiles on: a pool over std.sync.worker_pool that runs each task it
is handed on a thread with the driver's stack reserve, then reports the task's index on a channel
of finished indices, which only the owner reads. a task's writes reach the owner through that
channel, so the owner reads what a task made only once it received the task's index

Source: [`src/compile/workers.mach`](../../../src/compile/workers.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Crew`](#crew) | `rec` | the pool, its storage and the channel of finished indices, all drawn from `a` |

| On `Crew` | |
| :--- | :--- |
| [`open`](#open) | a crew of `n` threads for at most `units` tasks, its queue holding `queued` waiting tasks, or false when one cannot be had (storage, or a thread the host refused), the crew then holding nothing |
| [`submit`](#submit) | `run(ctx)` handed to a thread as task `index`, below `units`, which no other open task holds |
| [`finished`](#finished) | the index of the next task to finish, waiting for one; none once the channel is closed |
| [`finished_now`](#finished_now) | the index of a task that finished and was not yet taken, none when there is none now |
| [`close`](#close) | no task starts after this: those still queued are dropped unrun, and it returns once every task a thread took has finished and every thread has joined |
| [`dnit`](#dnit) | give back a closed crew once every finished index was taken |

---

## Crew

```mach
pub rec Crew {
    a:         *std.allocator.Allocator;
    pool:      std.sync.worker_pool.Pool;
    queue:     *std.sync.worker_pool.Task;
    queue_cap: usize;
    threads:   *std.sync.thread.Thread;
    workers:   *std.sync.worker_pool.Worker;
    count:     usize;
    tasks:     *Task;
    units:     usize;
    finished:  std.sync.channel.Channel[u32];
    indices:   *u32;
    started:   u32;
}
```

the pool, its storage and the channel of finished indices, all drawn from `a`

| Field | Type | |
| :--- | :--- | :--- |
| `units` | `usize` | the most tasks it is handed, one per index below it |
| `started` | `u32` | the threads the pool runs, which a readout summed over them names |

### open

```mach
pub fun open(c: *Crew, a: *std.allocator.Allocator, n: u32, units: usize, queued: usize) bool
```

a crew of `n` threads for at most `units` tasks, its queue holding `queued` waiting tasks, or false
when one cannot be had (storage, or a thread the host refused), the crew then holding nothing.
`c` must not move while the crew is open

### submit

```mach
pub fun submit(c: *Crew, index: u32, ctx: ptr, run: fun(ptr)) bool
```

`run(ctx)` handed to a thread as task `index`, below `units`, which no other open task holds

### finished

```mach
pub fun finished(c: *Crew) opt[u32]
```

the index of the next task to finish, waiting for one; none once the channel is closed

### finished_now

```mach
pub fun finished_now(c: *Crew) opt[u32]
```

the index of a task that finished and was not yet taken, none when there is none now

### close

```mach
pub fun close(c: *Crew)
```

no task starts after this: those still queued are dropped unrun, and it returns once every task a
thread took has finished and every thread has joined. the indices of finished tasks stay to be
taken with finished_now

### dnit

```mach
pub fun dnit(c: *Crew)
```

give back a closed crew once every finished index was taken

