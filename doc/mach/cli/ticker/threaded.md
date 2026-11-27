[mach](../../README.md) / [cli](../README.md) / [ticker](README.md) / **threaded**

# mach.cli.ticker.threaded

a ticker on a thread of its own: every period it takes the owner's lock and calls `tick`, until
it is stopped. what `tick` reads is the owner's, guarded by that lock

Source: [`src/cli/ticker/threaded.mach`](../../../../src/cli/ticker/threaded.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Ticker`](#ticker) | `rec` | a running ticker, which must not move while it runs |

| On `Ticker` | |
| :--- | :--- |
| [`start`](#start) | start `tick(ctx)` every `period` under `lock`, which the caller does not hold |
| [`stop`](#stop) | stop a started ticker, returning once its thread has ended |

---

## Ticker

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

a running ticker, which must not move while it runs

| Field | Type | |
| :--- | :--- | :--- |
| `lock` | `*std.sync.mutex.Mutex` | the owner's lock, held around each tick |
| `period` | `std.chrono.duration.Duration` | the time between ticks |
| `tick` | `fun(ptr)` | what runs each period, the lock held |
| `ctx` | `ptr` | what `tick` is passed |
| `stop` | `bool` | the ticker was told to stop, guarded by `lock` |
| `wake` | `std.sync.condition.Condition` | signalled to end the wait between ticks early |
| `thread` | `std.sync.thread.Thread` | the thread it runs on |

### start

```mach
pub fun start(t: *Ticker, lock: *std.sync.mutex.Mutex, period: std.chrono.duration.Duration, tick: fun(ptr), ctx: ptr) bool
```

start `tick(ctx)` every `period` under `lock`, which the caller does not hold

| Parameter | |
| :--- | :--- |
| **returns** | whether it runs, false when the host refused the thread |

### stop

```mach
pub fun stop(t: *Ticker)
```

stop a started ticker, returning once its thread has ended. the caller does not hold the lock

