[mach](../README.md) / [cli](README.md) / **ticker**

# mach.cli.ticker

a thread that calls back on a period, where the host has a thread to give. both members export
one ticker, and on a host with no threads none starts, as where the host refuses the thread

Source: [`src/cli/ticker.mach`](../../../src/cli/ticker.mach)

| Forwards | |
| :--- | :--- |
| [`Ticker`](#ticker), [`start`](#start), [`stop`](#stop) | |

---

## Forwards

### Ticker

Forwards the first of these whose condition holds, and they differ.

Where `capability.HAS_THREADS`: [`mach.cli.ticker.threaded.Ticker`](ticker/threaded.md#ticker).

```mach
pub rec Ticker {
    lock:   *std.sync.mutex.Mutex;
    period: std.chrono.duration.Duration;
    tick:   fun(ptr);
    ctx:    ptr;
    stop:   bool;
    wake:   std.sync.condition.Condition;
    thread: std.sync.thread.Thread;
}
```

Otherwise: [`mach.cli.ticker.absent.Ticker`](ticker/absent.md#ticker).

```mach
pub rec Ticker {
    stop: bool;
}
```

a running ticker, which must not move while it runs

### start

Forwards the first of these whose condition holds:

- [`mach.cli.ticker.threaded.start`](ticker/threaded.md#start), where `capability.HAS_THREADS`
- [`mach.cli.ticker.absent.start`](ticker/absent.md#start), otherwise

```mach
pub fun start(t: *Ticker, lock: *std.sync.mutex.Mutex, period: std.chrono.duration.Duration, tick: fun(ptr), ctx: ptr) bool
```

start `tick(ctx)` every `period` under `lock`, which the caller does not hold

### stop

Forwards the first of these whose condition holds:

- [`mach.cli.ticker.threaded.stop`](ticker/threaded.md#stop), where `capability.HAS_THREADS`
- [`mach.cli.ticker.absent.stop`](ticker/absent.md#stop), otherwise

```mach
pub fun stop(t: *Ticker)
```

stop a started ticker, returning once its thread has ended. the caller does not hold the lock

