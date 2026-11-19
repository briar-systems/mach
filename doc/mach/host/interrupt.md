[mach](../README.md) / [host](README.md) / **interrupt**

# mach.host.interrupt

the process's interrupt. while a command holds something it must release before it ends
(a scratch directory, a child it waits on), the signals or console events that would end
the process are caught instead of ending it. the first one caught is kept for the rest of
the process: every wait and every step that can stop asks `taken` and stops, the holder
releases what it made, and the command ends with `status`. holds nest, and the first hold
claims the process's one event source (std.process.events), the last returns it

Source: [`src/host/interrupt.mach`](../../../src/host/interrupt.mach)

| Functions | |
| :--- | :--- |
| [`hold`](#hold) | catch the ending events from here until the matching `release` |
| [`release`](#release) | end one hold; the last restores the default handling, and an event caught before it stays taken |
| [`taken`](#taken) | whether an ending event has been caught, now or earlier in the process |
| [`status`](#status) | the exit status of a process ended by the event taken: 128 plus the signal's number, as a shell reports a process the signal ended, and the console's own status on windows |

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

### taken

```mach
pub fun taken() bool
```

whether an ending event has been caught, now or earlier in the process

### status

```mach
pub fun status() i64
```

the exit status of a process ended by the event taken: 128 plus the signal's number, as a
shell reports a process the signal ended, and the console's own status on windows

