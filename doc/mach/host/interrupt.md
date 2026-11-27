[mach](../README.md) / [host](README.md) / **interrupt**

# mach.host.interrupt

the process's interrupt, where the host can deliver one. both members export one surface: the
signals or console events caught while a command holds something it must release, and on a
host that delivers no such event an interrupt that is never taken

Source: [`src/host/interrupt.mach`](../../../src/host/interrupt.mach)

| Forwards | |
| :--- | :--- |
| [`queue`](#queue) | the wait queue a caught event wakes, which only a host with events has |
| [`hold`](#hold), [`release`](#release), [`taken`](#taken), [`name`](#name), [`status`](#status) | |

---

## Forwards

### queue

Forwards [`mach.host.interrupt.events.queue`](interrupt/events.md#queue), only where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`.

```mach
pub fun queue() *std.system.os.IoQueue
```

the wait queue a caught event wakes, which only a host with events has

### hold

Forwards the first of these whose condition holds:

- [`mach.host.interrupt.events.hold`](interrupt/events.md#hold), where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`
- [`mach.host.interrupt.absent.hold`](interrupt/absent.md#hold), otherwise

```mach
pub fun hold() err[std.io.error.Error]
```

catch the ending events from here until the matching `release`

### release

Forwards the first of these whose condition holds:

- [`mach.host.interrupt.events.release`](interrupt/events.md#release), where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`
- [`mach.host.interrupt.absent.release`](interrupt/absent.md#release), otherwise

```mach
pub fun release() err[std.io.error.Error]
```

end one hold; the last restores the default handling, and an event caught before it stays taken

### taken

Forwards the first of these whose condition holds:

- [`mach.host.interrupt.events.taken`](interrupt/events.md#taken), where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`
- [`mach.host.interrupt.absent.taken`](interrupt/absent.md#taken), otherwise

```mach
pub fun taken() bool
```

whether an ending event has been caught, now or earlier in the process

### name

Forwards the first of these whose condition holds:

- [`mach.host.interrupt.events.name`](interrupt/events.md#name), where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`
- [`mach.host.interrupt.absent.name`](interrupt/absent.md#name), otherwise

```mach
pub fun name() opt[str]
```

the name of the event taken, none while none was: the signal on linux and darwin, the console
or service event on windows

### status

Forwards the first of these whose condition holds:

- [`mach.host.interrupt.events.status`](interrupt/events.md#status), where `capability.HAS_IO_QUEUE && capability.HAS_PROCESS`
- [`mach.host.interrupt.absent.status`](interrupt/absent.md#status), otherwise

```mach
pub fun status() opt[i64]
```

the exit status of a process ended by the event taken, none while none was: 128 plus the
signal's number, as a shell reports a process the signal ended, and the console's own status
on windows

