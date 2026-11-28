[mach](../../README.md) / [cli](../README.md) / [ticker](README.md) / **absent**

# mach.cli.ticker.absent

the ticker of a host with no threads: none starts, so its owner draws nothing between its own
writes, as where the host refuses the thread

Source: [`src/cli/ticker/absent.mach`](../../../../src/cli/ticker/absent.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Ticker`](#ticker) | `rec` | a ticker that never runs |

| On `Ticker` | |
| :--- | :--- |
| [`start`](#start) | refuse the ticker: the host has no thread to run it on |
| [`stop`](#stop) | |

---

## Ticker

```mach
pub rec Ticker {
    stop: bool;
}
```

a ticker that never runs

### start

```mach
pub fun start(t: *Ticker, lock: *std.sync.mutex.Mutex, period: std.chrono.duration.Duration, tick: fun(ptr), ctx: ptr) bool
```

refuse the ticker: the host has no thread to run it on

### stop

```mach
pub fun stop(t: *Ticker)
```

