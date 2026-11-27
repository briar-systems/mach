[mach](../../README.md) / [host](../README.md) / [subprocess](README.md) / **absent**

# mach.host.subprocess.absent

the supervisor of a host that starts no process. an owner here never holds a child: its one
spawn is refused, it is never running, and everything that needs a running child refuses with
the same cause, so the diagnostic a command ends with names what the host cannot do

Source: [`src/host/subprocess/absent.mach`](../../../../src/host/subprocess/absent.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `rec` | the one refusal of this member, named with the pathname the caller gave |
| [`OwnedSubprocess`](#ownedsubprocess) | `rec` | the owner of a child that is never started |

| On `Error` | |
| :--- | :--- |
| [`refusal`](#refusal) | the machine's refusal `e` to `verb` the child, an environment failure under `k` naming the child and, when `cwd` is given, the directory it would have run in |
| [`refusal_of`](#refusal_of) | the machine's refusal `e` of `op`, which names the child as its caller knows it |
| [`resolve`](#resolve) | refuse the search: no program found on PATH could be started |

| On `OwnedSubprocess` | |
| :--- | :--- |
| [`init`](#init) | |
| [`bound`](#bound) | record the bound a child would have run under |
| [`expired`](#expired) | false: no child ran to outlive its bound |
| [`dnit`](#dnit) | |
| [`spawn`](#spawn) | refuse the spawn: the host starts no process |
| [`spawn_native`](#spawn_native) | refuse the spawn: the host starts no process |
| [`running`](#running) | false: no owner here ever holds a child |
| [`wait`](#wait), [`wait_any`](#wait_any), [`finish_capture`](#finish_capture), [`abandon_capture`](#abandon_capture) | |
| [`capture_bytes`](#capture_bytes) | nil and no bytes: no child wrote a capture |
| [`capture_dnit`](#capture_dnit), [`timeout`](#timeout), [`cancel`](#cancel) | |

---

## Error

```mach
pub rec Error {
    identity: str;
}
```

the one refusal of this member, named with the pathname the caller gave

| Field | Type | |
| :--- | :--- | :--- |
| `identity` | `str` | the pathname of the child that was not started, borrowed from the caller |

### refusal

```mach
pub fun refusal(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, verb: str, e: Error, cwd: str) mach.base.fail.Fail
```

the machine's refusal `e` to `verb` the child, an environment failure under `k` naming the child
and, when `cwd` is given, the directory it would have run in

### refusal_of

```mach
pub fun refusal_of(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, op: mach.base.fail.Op, e: Error) mach.base.fail.Fail
```

the machine's refusal `e` of `op`, which names the child as its caller knows it

### resolve

```mach
pub fun resolve(a: *std.allocator.Allocator, name: str) res[str, Error]
```

refuse the search: no program found on PATH could be started

## OwnedSubprocess

```mach
pub rec OwnedSubprocess {
    terminal: SubprocessTerminal;
    pathname: str;
    timeout:  std.chrono.duration.Duration;
}
```

the owner of a child that is never started

| Field | Type | |
| :--- | :--- | :--- |
| `terminal` | `SubprocessTerminal` | STATE_UNSTARTED, then STATE_SPAWN_FAILURE once its spawn was refused |
| `pathname` | `str` | the pathname its spawn was given, naming its refusals, borrowed from the caller |
| `timeout` | `std.chrono.duration.Duration` | the bound `bound` was given |

### init

```mach
pub fun init(p: *OwnedSubprocess)
```

### bound

```mach
pub fun bound(p: *OwnedSubprocess, timeout: std.chrono.duration.Duration) err[Error]
```

record the bound a child would have run under

### expired

```mach
pub fun expired(p: *OwnedSubprocess) bool
```

false: no child ran to outlive its bound

### dnit

```mach
pub fun dnit(a: *std.allocator.Allocator, p: *OwnedSubprocess)
```

### spawn

```mach
pub fun spawn(a: *std.allocator.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: std.system.os.Environment,
    options: SpawnOptions) err[Error]
```

refuse the spawn: the host starts no process

### spawn_native

```mach
pub fun spawn_native(a: *std.allocator.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: std.system.os.NativeEnvironment,
    options: SpawnOptions) err[Error]
```

refuse the spawn: the host starts no process

### running

```mach
pub fun running(p: *OwnedSubprocess) bool
```

false: no owner here ever holds a child

### wait

```mach
pub fun wait(a: *std.allocator.Allocator, p: *OwnedSubprocess) res[SubprocessTerminal, Error]
```

### wait_any

```mach
pub fun wait_any(a: *std.allocator.Allocator, owners: *OwnedSubprocess, count: u32, interruptible: bool) res[*OwnedSubprocess, Error]
```

### finish_capture

```mach
pub fun finish_capture(a: *std.allocator.Allocator, p: *OwnedSubprocess, retain: bool) res[CaptureIoResult, Error]
```

### abandon_capture

```mach
pub fun abandon_capture(p: *OwnedSubprocess)
```

### capture_bytes

```mach
pub fun capture_bytes(p: *OwnedSubprocess, len: *usize) *u8
```

nil and no bytes: no child wrote a capture

### capture_dnit

```mach
pub fun capture_dnit(a: *std.allocator.Allocator, p: *OwnedSubprocess) err[Error]
```

### timeout

```mach
pub fun timeout(a: *std.allocator.Allocator, p: *OwnedSubprocess, grace: std.chrono.duration.Duration) res[SubprocessTerminal, Error]
```

### cancel

```mach
pub fun cancel(a: *std.allocator.Allocator, p: *OwnedSubprocess, grace: std.chrono.duration.Duration) res[SubprocessTerminal, Error]
```

