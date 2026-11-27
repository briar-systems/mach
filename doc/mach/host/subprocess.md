[mach](../README.md) / [host](README.md) / **subprocess**

# mach.host.subprocess

supervised child processes, where the host can start one. both members export one surface over
the shapes in mach.host.subprocess.shape. where the host has no process to start, every spawn is
refused, so a command that needs a child ends with a diagnostic that says so

Source: [`src/host/subprocess.mach`](../../../src/host/subprocess.mach)

| Forwards | |
| :--- | :--- |
| [`STATE_UNSTARTED`](#state_unstarted), [`STATE_SPAWN_FAILURE`](#state_spawn_failure), [`STATE_EXITED`](#state_exited), [`STATE_SIGNALED`](#state_signaled), [`SubprocessTerminal`](#subprocessterminal), [`CaptureIoResult`](#captureioresult), [`SpawnOptions`](#spawnoptions), [`spawn_options`](#spawn_options), [`Error`](#error), [`refusal`](#refusal), [`refusal_of`](#refusal_of), [`reason`](#reason), [`unable`](#unable), [`resolve`](#resolve), [`OwnedSubprocess`](#ownedsubprocess), [`init`](#init), [`bound`](#bound), [`expired`](#expired), [`dnit`](#dnit), [`spawn`](#spawn), [`spawn_native`](#spawn_native), [`running`](#running), [`wait`](#wait), [`wait_any`](#wait_any), [`finish_capture`](#finish_capture), [`abandon_capture`](#abandon_capture), [`capture_bytes`](#capture_bytes), [`capture_dnit`](#capture_dnit), [`timeout`](#timeout), [`cancel`](#cancel) | |

---

## Forwards

### STATE_UNSTARTED

Forwards [`mach.host.subprocess.shape.STATE_UNSTARTED`](subprocess/shape.md#subprocessstate).

```mach
pub val STATE_UNSTARTED: SubprocessState = 0
```

fresh from init, nothing spawned

### STATE_SPAWN_FAILURE

Forwards [`mach.host.subprocess.shape.STATE_SPAWN_FAILURE`](subprocess/shape.md#subprocessstate).

```mach
pub val STATE_SPAWN_FAILURE: SubprocessState = 1
```

the one spawn attempt started no child

### STATE_EXITED

Forwards [`mach.host.subprocess.shape.STATE_EXITED`](subprocess/shape.md#subprocessstate).

```mach
pub val STATE_EXITED: SubprocessState = 3
```

the child exited with a code, and as a terminal kind, ended that way

### STATE_SIGNALED

Forwards [`mach.host.subprocess.shape.STATE_SIGNALED`](subprocess/shape.md#subprocessstate).

```mach
pub val STATE_SIGNALED: SubprocessState = 4
```

the child was ended by a signal, and as a terminal kind, ended that way

### SubprocessTerminal

Forwards [`mach.host.subprocess.shape.SubprocessTerminal`](subprocess/shape.md#subprocessterminal).

```mach
pub rec SubprocessTerminal {
    kind: SubprocessState;
    code: i32;
}
```

how a child ended

### CaptureIoResult

Forwards [`mach.host.subprocess.shape.CaptureIoResult`](subprocess/shape.md#captureioresult).

```mach
pub rec CaptureIoResult {
    bytes:        usize;
    truncated:    bool;
    short_read:   bool;
    read_failed:  bool;
    write_failed: bool;
    close_failed: bool;
    closed:       bool;
    owned:        bool;
}
```

what happened to a capture file

### SpawnOptions

Forwards [`mach.host.subprocess.shape.SpawnOptions`](subprocess/shape.md#spawnoptions).

```mach
pub rec SpawnOptions {
    cwd:            str;
    stdin:          usize;
    grouped:        bool;
    capture:        str;
    capture_stderr: bool;
    capture_limit:  usize;
}
```

how spawn starts a child, spawn_options giving the defaults

### spawn_options

Forwards [`mach.host.subprocess.shape.spawn_options`](subprocess/shape.md#spawn_options).

```mach
pub fun spawn_options() SpawnOptions
```

the options of a plain spawn: this process's directory, its streams and its
group, with nothing captured

### Error

Forwards the first of these whose condition holds, and they differ.

Where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`: [`mach.host.subprocess.native.Error`](subprocess/native.md#error).

```mach
pub rec Error {
    identity: str;
    cause:    Cause;
}
```

Otherwise: [`mach.host.subprocess.absent.Error`](subprocess/absent.md#error).

```mach
pub rec Error {
    identity: str;
}
```

a refusal of this module

### refusal

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.refusal`](subprocess/native.md#refusal), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.refusal`](subprocess/absent.md#refusal), otherwise

```mach
pub fun refusal(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, verb: str, e: Error, cwd: str) mach.base.fail.Fail
```

the machine's refusal `e` to `verb` the child, an environment failure under `k`
naming the child and, when `cwd` is given, the directory it runs in; an
allocation refusal is internal

### refusal_of

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.refusal_of`](subprocess/native.md#refusal_of), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.refusal_of`](subprocess/absent.md#refusal_of), otherwise

```mach
pub fun refusal_of(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, op: mach.base.fail.Op, e: Error) mach.base.fail.Fail
```

the machine's refusal `e` of `op`, which names the child as its caller knows it
(a runner, a build step); an allocation refusal is internal

### reason

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.reason`](subprocess/native.md#reason), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.reason`](subprocess/absent.md#reason), otherwise

```mach
pub fun reason(e: Error) str
```

what refused in `e`, static, for a line that names the child itself (a test that could not be
launched); a failure goes through `refusal`

### unable

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.unable`](subprocess/native.md#unable), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.unable`](subprocess/absent.md#unable), otherwise

```mach
pub fun unable(identity: str) opt[Error]
```

none: this host starts processes, so nothing is refused before a spawn is tried

### resolve

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.resolve`](subprocess/native.md#resolve), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.resolve`](subprocess/absent.md#resolve), otherwise

```mach
pub fun resolve(a: *std.allocator.Allocator, name: str) res[str, Error]
```

the program `name` names on PATH, as a path a spawn takes. the current directory is never searched

### OwnedSubprocess

Forwards the first of these whose condition holds, and they differ.

Where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`: [`mach.host.subprocess.native.OwnedSubprocess`](subprocess/native.md#ownedsubprocess).

```mach
pub rec OwnedSubprocess {
    child:           std.process.exec.Child;
    state:           SubprocessState;
    terminal:        SubprocessTerminal;
    requested:       Request;
    capture:         OwnedCapture;
    has_capture:     bool;
    spawn_attempted: bool;
    reap_count:      u8;
    pathname:        *u8;
    has_deadline:    bool;
    deadline:        std.chrono.time.Instant;
    timeout:         std.chrono.duration.Duration;
    watch:           std.process.exec.ExitWatch;
}
```

Otherwise: [`mach.host.subprocess.absent.OwnedSubprocess`](subprocess/absent.md#ownedsubprocess).

```mach
pub rec OwnedSubprocess {
    terminal: SubprocessTerminal;
    pathname: str;
    timeout:  std.chrono.duration.Duration;
}
```

the owner of one supervised child, made by init and released by dnit

### init

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.init`](subprocess/native.md#init), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.init`](subprocess/absent.md#init), otherwise

```mach
pub fun init(p: *OwnedSubprocess)
```

make `p` a fresh owner with nothing spawned

### bound

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.bound`](subprocess/native.md#bound), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.bound`](subprocess/absent.md#bound), otherwise

```mach
pub fun bound(p: *OwnedSubprocess, timeout: std.chrono.duration.Duration) err[Error]
```

give `p` a deadline `timeout` from now, which wait_any enforces

### expired

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.expired`](subprocess/native.md#expired), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.expired`](subprocess/absent.md#expired), otherwise

```mach
pub fun expired(p: *OwnedSubprocess) bool
```

whether the child was terminated for outliving its deadline

### dnit

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.dnit`](subprocess/native.md#dnit), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.dnit`](subprocess/absent.md#dnit), otherwise

```mach
pub fun dnit(a: *std.allocator.Allocator, p: *OwnedSubprocess)
```

release what `p` owns: its pathname, and its capture file and identity. a
second call releases nothing. it does not reap a running child

### spawn

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.spawn`](subprocess/native.md#spawn), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.spawn`](subprocess/absent.md#spawn), otherwise

```mach
pub fun spawn(a: *std.allocator.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: std.system.os.Environment,
    options: SpawnOptions) err[Error]
```

start a child under `p`, which must be fresh from init: an owner spawns at
most once, even when the spawn fails

the pathname, argv and envp are copied, so the caller's buffers may change
as soon as this returns. the owner keeps the pathname to name the child in
its errors. on failure the owner is in STATE_SPAWN_FAILURE and owns nothing.

### spawn_native

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.spawn_native`](subprocess/native.md#spawn_native), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.spawn_native`](subprocess/absent.md#spawn_native), otherwise

```mach
pub fun spawn_native(a: *std.allocator.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: std.system.os.NativeEnvironment,
    options: SpawnOptions) err[Error]
```

spawn with the child's environment in native units, so an inherited variable
with no UTF-8 spelling reaches it unit for unit. the pathname, argv and
working directory are UTF-8 as spawn takes them, converted once; the
environment is read only during the call. everything else is as spawn

### running

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.running`](subprocess/native.md#running), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.running`](subprocess/absent.md#running), otherwise

```mach
pub fun running(p: *OwnedSubprocess) bool
```

whether `p` holds a child that has not ended, which a cancel or a timeout can stop

### wait

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.wait`](subprocess/native.md#wait), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.wait`](subprocess/absent.md#wait), otherwise

```mach
pub fun wait(a: *std.allocator.Allocator, p: *OwnedSubprocess) res[SubprocessTerminal, Error]
```

block until the running child of `p` ends, and reap it

### wait_any

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.wait_any`](subprocess/native.md#wait_any), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.wait_any`](subprocess/absent.md#wait_any), otherwise

```mach
pub fun wait_any(a: *std.allocator.Allocator, owners: *OwnedSubprocess, count: u32, interruptible: bool) res[*OwnedSubprocess, Error]
```

supervise `count` owners until one of them reaches its terminal, and return
that one reaped

the wait blocks, with no polling, until a running owner's child ends, an
ending event arrives while the process holds the interrupt (mach.host.interrupt,
whose queue the wait blocks on then), the nearest deadline passes, or a
capture is due its check. each pass looks at the interrupt, then every running
owner: a taken interrupt cancels the first running owner where the caller asked
for that, an owner past its own deadline is timed out, and one whose capture
outgrew its limit is cancelled, each with GRACE to stop.

### finish_capture

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.finish_capture`](subprocess/native.md#finish_capture), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.finish_capture`](subprocess/absent.md#finish_capture), otherwise

```mach
pub fun finish_capture(a: *std.allocator.Allocator, p: *OwnedSubprocess, retain: bool) res[CaptureIoResult, Error]
```

read the capture of a finished child back and close it, once

### abandon_capture

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.abandon_capture`](subprocess/native.md#abandon_capture), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.abandon_capture`](subprocess/absent.md#abandon_capture), otherwise

```mach
pub fun abandon_capture(p: *OwnedSubprocess)
```

close the capture without reading it back, marking it failed

### capture_bytes

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.capture_bytes`](subprocess/native.md#capture_bytes), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.capture_bytes`](subprocess/absent.md#capture_bytes), otherwise

```mach
pub fun capture_bytes(p: *OwnedSubprocess, len: *usize) *u8
```

the captured bytes retained by finish_capture, borrowed from `p`

### capture_dnit

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.capture_dnit`](subprocess/native.md#capture_dnit), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.capture_dnit`](subprocess/absent.md#capture_dnit), otherwise

```mach
pub fun capture_dnit(a: *std.allocator.Allocator, p: *OwnedSubprocess) err[Error]
```

free the captured bytes and the capture identity of a finished capture, once

### timeout

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.timeout`](subprocess/native.md#timeout), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.timeout`](subprocess/absent.md#timeout), otherwise

```mach
pub fun timeout(a: *std.allocator.Allocator, p: *OwnedSubprocess, grace: std.chrono.duration.Duration) res[SubprocessTerminal, Error]
```

terminate the running child for outliving its deadline and reap it

the child is asked to stop, its whole group when it has one, and given
`grace` to end before it is killed. a grouped child's group gets the whole
grace even when the child ends sooner, and what is left of it then is
killed as a group.

### cancel

Forwards the first of these whose condition holds:

- [`mach.host.subprocess.native.cancel`](subprocess/native.md#cancel), where `capability.HAS_CLOCK && capability.HAS_FILES && capability.HAS_PROCESS && capability.HAS_THREADS`
- [`mach.host.subprocess.absent.cancel`](subprocess/absent.md#cancel), otherwise

```mach
pub fun cancel(a: *std.allocator.Allocator, p: *OwnedSubprocess, grace: std.chrono.duration.Duration) res[SubprocessTerminal, Error]
```

terminate the running child as cancelled and reap it, as timeout does

