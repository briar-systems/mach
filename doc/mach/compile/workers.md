[mach](../README.md) / [compile](README.md) / **workers**

# mach.compile.workers

the threads a codegen pass compiles on, where the host has threads to give. both members export
one crew, and on a host with no threads no crew opens, so each caller takes the serial path it
already has for a host that refused it a thread

Source: [`src/compile/workers.mach`](../../../src/compile/workers.mach)

| Forwards | |
| :--- | :--- |
| [`Crew`](#crew), [`open`](#open), [`submit`](#submit), [`finished`](#finished), [`finished_now`](#finished_now), [`close`](#close), [`dnit`](#dnit) | |

---

## Forwards

### Crew

Forwards the first of these whose condition holds, and they differ.

Where `capability.HAS_THREADS`: [`mach.compile.workers.pooled.Crew`](workers/pooled.md#crew).

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

Otherwise: [`mach.compile.workers.absent.Crew`](workers/absent.md#crew).

```mach
pub rec Crew {
    started: u32;
}
```

the pool, its storage and the channel of finished indices, all drawn from `a`

### open

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.open`](workers/pooled.md#open), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.open`](workers/absent.md#open), otherwise

```mach
pub fun open(c: *Crew, a: *std.allocator.Allocator, n: u32, units: usize, queued: usize) bool
```

a crew of `n` threads for at most `units` tasks, its queue holding `queued` waiting tasks, or false
when one cannot be had (storage, or a thread the host refused), the crew then holding nothing.
`c` must not move while the crew is open

### submit

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.submit`](workers/pooled.md#submit), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.submit`](workers/absent.md#submit), otherwise

```mach
pub fun submit(c: *Crew, index: u32, ctx: ptr, run: fun(ptr)) bool
```

`run(ctx)` handed to a thread as task `index`, below `units`, which no other open task holds

### finished

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.finished`](workers/pooled.md#finished), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.finished`](workers/absent.md#finished), otherwise

```mach
pub fun finished(c: *Crew) opt[u32]
```

the index of the next task to finish, waiting for one; none once the channel is closed

### finished_now

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.finished_now`](workers/pooled.md#finished_now), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.finished_now`](workers/absent.md#finished_now), otherwise

```mach
pub fun finished_now(c: *Crew) opt[u32]
```

the index of a task that finished and was not yet taken, none when there is none now

### close

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.close`](workers/pooled.md#close), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.close`](workers/absent.md#close), otherwise

```mach
pub fun close(c: *Crew)
```

no task starts after this: those still queued are dropped unrun, and it returns once every task a
thread took has finished and every thread has joined. the indices of finished tasks stay to be
taken with finished_now

### dnit

Forwards the first of these whose condition holds:

- [`mach.compile.workers.pooled.dnit`](workers/pooled.md#dnit), where `capability.HAS_THREADS`
- [`mach.compile.workers.absent.dnit`](workers/absent.md#dnit), otherwise

```mach
pub fun dnit(c: *Crew)
```

give back a closed crew once every finished index was taken

