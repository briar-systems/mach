[mach](../README.md) / [host](README.md) / **subprocess**

# mach.host.subprocess

supervised child processes

an OwnedSubprocess owns one child from spawn to reap. it moves through
STATE_UNSTARTED, then STATE_RUNNING or STATE_SPAWN_FAILURE, then
STATE_EXITED or STATE_SIGNALED as the terminal is observed, and ends in
STATE_REAPED. each step happens at most once: an owner spawns once, observes
one terminal and reaps once, so a child is never waited on twice and a pid
is never reused under a stale owner. wait, wait_any, timeout and cancel are
the ways to reach the terminal. a capture, when the spawn asked for one, is
a file the child writes and the owner reads back after the child is done.

<sub>source: [`src/host/subprocess.mach`](../../../src/host/subprocess.mach) · imported by 5 modules</sub>

<sub>`Cause` is private · `A` is `std.allocator` · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `SubprocessState` is private · `exec` is `std.process.exec` · `OwnedCapture` is private · `time` is `std.chrono.time` · `chrono_duration` is `std.chrono.duration` · `sync_cancel` is `std.sync.cancel` · `events` is `std.process.events`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Request`](#request) | `def` | the termination a supervisor asked for, mirrored from the cancellation scope's reason, so the codes are the scope's 1.x reason codes and never change |
| [`Error`](#error) | `rec` | a refusal of this module |
| [`SubprocessTerminal`](#subprocessterminal) | `rec` | how a child ended |
| [`CaptureIoResult`](#captureioresult) | `rec` | what happened to a capture file |
| [`OwnedSubprocess`](#ownedsubprocess) | `rec` | the owner of one supervised child, made by init and released by dnit |
| [`SpawnOptions`](#spawnoptions) | `rec` | how spawn starts a child, spawn_options giving the defaults |

| On `Error` | |
| :--- | :--- |
| [`failure`](#failure) | the refusal `cause`, named by `identity` |
| [`refusal`](#refusal) | the machine's refusal `e` to `verb` the child, an environment failure under `k` naming the child and, when `cwd` is given, the directory it runs in; an allocation refusal is internal |
| [`refusal_of`](#refusal_of) | the machine's refusal `e` of `op`, which names the child as its caller knows it (a runner, a build step); an allocation refusal is internal |

| On `OwnedSubprocess` | |
| :--- | :--- |
| [`init`](#init) | make `p` a fresh owner with nothing spawned |
| [`bound`](#bound) | give `p` a deadline `timeout` from now, which wait_any enforces |
| [`expired`](#expired) | whether the child was terminated for outliving its deadline |
| [`dnit`](#dnit) | release what `p` owns: its pathname, and its capture file and identity |
| [`spawn`](#spawn) | start a child under `p`, which must be fresh from init: an owner spawns at most once, even when the spawn fails |
| [`spawn_native`](#spawn_native) | spawn with the child's environment in native units, so an inherited variable with no UTF-8 spelling reaches it unit for unit |
| [`running`](#running) | whether `p` holds a child that has not ended, which a cancel or a timeout can stop |
| [`wait`](#wait) | block until the running child of `p` ends, and reap it |
| [`wait_any`](#wait_any) | supervise `count` owners until one of them reaches its terminal, and return that one reaped |
| [`finish_capture`](#finish_capture) | read the capture of a finished child back and close it, once |
| [`abandon_capture`](#abandon_capture) | close the capture without reading it back, marking it failed |
| [`capture_bytes`](#capture_bytes) | the captured bytes retained by finish_capture, borrowed from `p` |
| [`capture_dnit`](#capture_dnit) | free the captured bytes and the capture identity of a finished capture, once |
| [`timeout`](#timeout) | terminate the running child for outliving its deadline and reap it |
| [`cancel`](#cancel) | terminate the running child as cancelled and reap it, as timeout does |

| On `SpawnOptions` | |
| :--- | :--- |
| [`spawn_options`](#spawn_options) | the options of a plain spawn: this process's directory, its streams and its group, with nothing captured |

| Constants | |
| :--- | :--- |
| [`STATE_UNSTARTED`](#constants) | fresh from init, nothing spawned |
| [`STATE_SPAWN_FAILURE`](#constants) | the one spawn attempt started no child |
| [`STATE_EXITED`](#constants) | the child exited with a code, and as a terminal kind, ended that way |
| [`STATE_SIGNALED`](#constants) | the child was ended by a signal, and as a terminal kind, ended that way |

---

## Request

```mach
pub def Request: u8
```

the termination a supervisor asked for, mirrored from the cancellation
scope's reason, so the codes are the scope's 1.x reason codes and never change

## Error

```mach
pub rec Error {
    identity: str;
    cause:    Cause;
}
```

a refusal of this module

| Field | Type | |
| :--- | :--- | :--- |
| `identity` | `str` | the pathname of the child, or "subprocess" before one is known, borrowed from the owner or the caller |
| `cause` | `Cause` | what refused |

### failure

```mach
pub fun failure(identity: str, cause: Cause) Error
```

the refusal `cause`, named by `identity`

### refusal

```mach
pub fun refusal(a: *A.Allocator, k: diagnostic_kind.Kind, verb: str, e: Error, cwd: str) fail.Fail
```

the machine's refusal `e` to `verb` the child, an environment failure under `k`
naming the child and, when `cwd` is given, the directory it runs in; an
allocation refusal is internal

### refusal_of

```mach
pub fun refusal_of(a: *A.Allocator, k: diagnostic_kind.Kind, op: fail.Op, e: Error) fail.Fail
```

the machine's refusal `e` of `op`, which names the child as its caller knows it
(a runner, a build step); an allocation refusal is internal

## SubprocessTerminal

```mach
pub rec SubprocessTerminal {
    kind: SubprocessState;
    code: i32;
}
```

how a child ended

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `SubprocessState` | STATE_EXITED or STATE_SIGNALED once observed, else the owner's state |
| `code` | `i32` | the exit code, or the signal number |

## CaptureIoResult

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

| Field | Type | |
| :--- | :--- | :--- |
| `bytes` | `usize` | captured bytes read back and kept |
| `truncated` | `bool` | the child wrote more than the capture limit |
| `short_read` | `bool` | the read back ended before the bytes the file held |
| `read_failed` | `bool` | the read back failed, or the capture was abandoned |
| `write_failed` | `bool` | the capture file could not be opened |
| `close_failed` | `bool` | closing the capture file failed |
| `closed` | `bool` | the capture file is closed |
| `owned` | `bool` | the owner still holds the captured bytes |

## OwnedSubprocess

```mach
pub rec OwnedSubprocess {
    child:           exec.Child;
    state:           SubprocessState;
    terminal:        SubprocessTerminal;
    requested:       Request;
    capture:         OwnedCapture;
    has_capture:     bool;
    spawn_attempted: bool;
    reap_count:      u8;
    pathname:        *u8;
    has_deadline:    bool;
    deadline:        time.Instant;
    timeout:         chrono_duration.Duration;
}
```

the owner of one supervised child, made by init and released by dnit

| Field | Type | |
| :--- | :--- | :--- |
| `child` | `exec.Child` | the child's handle while it runs |
| `state` | `SubprocessState` | where the owner is in its lifecycle (see STATE_UNSTARTED) |
| `terminal` | `SubprocessTerminal` | how the child ended, once observed |
| `requested` | `Request` | the termination a supervisor asked for |
| `capture` | `OwnedCapture` | the capture file and what was read back from it |
| `has_capture` | `bool` | the spawn asked for a capture |
| `spawn_attempted` | `bool` | spawn has run, so it cannot run again |
| `reap_count` | `u8` | times the child was reaped, never more than one |
| `pathname` | `*u8` | the owned copy of the child's pathname, naming its errors |
| `has_deadline` | `bool` | bound set a deadline |
| `deadline` | `time.Instant` | the instant wait_any times the child out |
| `timeout` | `chrono_duration.Duration` | the bound the deadline came from |

### init

```mach
pub fun init(p: *OwnedSubprocess)
```

make `p` a fresh owner with nothing spawned

### bound

```mach
pub fun bound(p: *OwnedSubprocess, timeout: chrono_duration.Duration) err[Error]
```

give `p` a deadline `timeout` from now, which wait_any enforces

| Parameter | |
| :--- | :--- |
| **returns** | nothing, or the refusal when the timeout is not positive, the clock fails, or the owner is done |

### expired

```mach
pub fun expired(p: *OwnedSubprocess) bool
```

whether the child was terminated for outliving its deadline

### dnit

```mach
pub fun dnit(a: *A.Allocator, p: *OwnedSubprocess)
```

release what `p` owns: its pathname, and its capture file and identity. a
second call releases nothing. it does not reap a running child

### spawn

```mach
pub fun spawn(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: exec.Environment,
    options: SpawnOptions) err[Error]
```

start a child under `p`, which must be fresh from init: an owner spawns at
most once, even when the spawn fails

the pathname, argv and envp are copied, so the caller's buffers may change
as soon as this returns. the owner keeps the pathname to name the child in
its errors. on failure the owner is in STATE_SPAWN_FAILURE and owns nothing.

| Parameter | |
| :--- | :--- |
| `a` | allocator for the owner's copies, which dnit releases |
| `p` | the owner, from init |
| `pathname` | path to the executable |
| `argv` | null-terminated argument array |
| `envp` | the child's environment, given entries or this process's inherited |
| `options` | directory, stdin, grouping and capture (see SpawnOptions) |
| **returns** | nothing, or the refusal named by the pathname |

### spawn_native

```mach
pub fun spawn_native(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: exec.NativeEnvironment,
    options: SpawnOptions) err[Error]
```

spawn with the child's environment in native units, so an inherited variable
with no UTF-8 spelling reaches it unit for unit. the pathname, argv and
working directory are UTF-8 as spawn takes them, converted once; the
environment is read only during the call. everything else is as spawn

| Parameter | |
| :--- | :--- |
| `envp` | the child's environment, given native entries or this process's inherited |

### running

```mach
pub fun running(p: *OwnedSubprocess) bool
```

whether `p` holds a child that has not ended, which a cancel or a timeout can stop

### wait

```mach
pub fun wait(a: *A.Allocator, p: *OwnedSubprocess) res[SubprocessTerminal, Error]
```

block until the running child of `p` ends, and reap it

| Parameter | |
| :--- | :--- |
| **returns** | how the child ended, or the refusal when no child is running |

### wait_any

```mach
pub fun wait_any(a: *A.Allocator, owners: *OwnedSubprocess, count: u32, scope: *sync_cancel.Scope, source: *events.Source) res[*OwnedSubprocess, Error]
```

supervise `count` owners until one of them reaches its terminal, and return
that one reaped

each pass polls the scope, then every running owner: a cancelled or expired
scope cancels or times out the first running owner, an owner past its own
deadline is timed out, and one whose capture outgrew its limit is
cancelled. a terminate, interrupt, console-close or service-stop event from
`source` cancels the scope.

| Parameter | |
| :--- | :--- |
| `owners` | array of `count` owners, some of which may not be running |
| `scope` | cancellation scope over all of them, or nil |
| `source` | process events that cancel the scope, or nil |
| **returns** | the owner that reached its terminal, or the refusal when none is running |

### finish_capture

```mach
pub fun finish_capture(a: *A.Allocator, p: *OwnedSubprocess, retain: bool) res[CaptureIoResult, Error]
```

read the capture of a finished child back and close it, once

| Parameter | |
| :--- | :--- |
| `retain` | keep the captured bytes for capture_bytes, else only count them |
| **returns** | what happened to the capture, or the refusal while the child runs or after capture_dnit |

### abandon_capture

```mach
pub fun abandon_capture(p: *OwnedSubprocess)
```

close the capture without reading it back, marking it failed

### capture_bytes

```mach
pub fun capture_bytes(p: *OwnedSubprocess, len: *usize) *u8
```

the captured bytes retained by finish_capture, borrowed from `p`

| Parameter | |
| :--- | :--- |
| `len` | receives the byte count, 0 when nothing is retained |
| **returns** | the bytes, or nil when nothing is retained |

### capture_dnit

```mach
pub fun capture_dnit(a: *A.Allocator, p: *OwnedSubprocess) err[Error]
```

free the captured bytes and the capture identity of a finished capture, once

### timeout

```mach
pub fun timeout(a: *A.Allocator, p: *OwnedSubprocess, grace: chrono_duration.Duration) res[SubprocessTerminal, Error]
```

terminate the running child for outliving its deadline and reap it

the child is asked to stop, its whole group when it has one, and given
`grace` to end before it is killed.

| Parameter | |
| :--- | :--- |
| `grace` | how long the child has to stop, rounded up to a millisecond |
| **returns** | how the child ended, or the refusal. a refused termination keeps the child running and owned, so the request can be retried |

### cancel

```mach
pub fun cancel(a: *A.Allocator, p: *OwnedSubprocess, grace: chrono_duration.Duration) res[SubprocessTerminal, Error]
```

terminate the running child as cancelled and reap it, as timeout does

## SpawnOptions

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

| Field | Type | |
| :--- | :--- | :--- |
| `cwd` | `str` | directory the child starts in, or empty to inherit this process's |
| `stdin` | `usize` | handle for the child's stdin, or system_os.INVALID_HANDLE to inherit |
| `grouped` | `bool` | place the child as the leader of its own process group, so a termination reaches every descendant that stays in it |
| `capture` | `str` | path of a file, created or truncated, that receives the child's stdout, or empty to inherit stdout |
| `capture_stderr` | `bool` | send the child's stderr to the capture file too |
| `capture_limit` | `usize` | the most captured bytes finish_capture keeps, and wait_any cancels a child whose capture grows past it |

### spawn_options

```mach
pub fun spawn_options() SpawnOptions
```

the options of a plain spawn: this process's directory, its streams and its
group, with nothing captured

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `STATE_UNSTARTED` | `SubprocessState` | `0` | fresh from init, nothing spawned |
| `STATE_SPAWN_FAILURE` | `SubprocessState` | `1` | the one spawn attempt started no child |
| `STATE_EXITED` | `SubprocessState` | `3` | the child exited with a code, and as a terminal kind, ended that way |
| `STATE_SIGNALED` | `SubprocessState` | `4` | the child was ended by a signal, and as a terminal kind, ended that way |

