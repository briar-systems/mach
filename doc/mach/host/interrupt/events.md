[mach](../../README.md) / [host](../README.md) / [interrupt](README.md) / **events**

# mach.host.interrupt.events

the process's interrupt. while a command holds something it must release before it ends
(a scratch directory, a child it waits on), the signals or console events that would end
the process are caught instead of ending it. the first one caught is kept for the rest of
the process: every wait and every step that can stop asks `taken` and stops, the holder
releases what it made, and the command ends with `status`. holds nest, and the first hold
claims the process's one event source (std.process.events), the last returns it

Source: [`src/host/interrupt/events.mach`](../../../../src/host/interrupt/events.mach)

| Functions | |
| :--- | :--- |
| [`hold`](#hold) | catch the ending events from here until the matching `release` |
| [`release`](#release) | end one hold; the last restores the default handling, and an event caught before it stays taken |
| [`queue`](#queue) | the wait queue every caught event wakes while something holds the interrupt, so a wait blocked on it ends for an event as well as for what it watches there |
| [`taken`](#taken) | whether an ending event has been caught, now or earlier in the process |
| [`name`](#name) | the name of the event taken, none while none was: the signal on linux and darwin, the console or service event on windows |
| [`status`](#status) | the exit status of a process ended by the event taken, none while none was: 128 plus the signal's number, as a shell reports a process the signal ended, and the console's own status on windows |

---

## Functions

### hold

```mach
pub fun hold() err[std.io.error.Error]
```

catch the ending events from here until the matching `release`

| Parameter | |
| :--- | :--- |
| **returns** | the refusal to claim the event source, which leaves nothing held |

### release

```mach
pub fun release() err[std.io.error.Error]
```

end one hold; the last restores the default handling, and an event caught before it stays taken

| Parameter | |
| :--- | :--- |
| **returns** | the refusal to restore the handlers |

### queue

```mach
pub fun queue() *std.system.os.IoQueue
```

the wait queue every caught event wakes while something holds the interrupt, so
a wait blocked on it ends for an event as well as for what it watches there

| Parameter | |
| :--- | :--- |
| **returns** | the queue, borrowed until the last release, or nil when nothing holds it |

### taken

```mach
pub fun taken() bool
```

whether an ending event has been caught, now or earlier in the process

### name

```mach
pub fun name() opt[str]
```

the name of the event taken, none while none was: the signal on linux and darwin, the console
or service event on windows

### status

```mach
pub fun status() opt[i64]
```

the exit status of a process ended by the event taken, none while none was: 128 plus the
signal's number, as a shell reports a process the signal ended, and the console's own status
on windows

