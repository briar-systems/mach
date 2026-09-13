# mach.subprocess

## def SubprocessState

```mach
pub def SubprocessState: u8
```

## val STATE_UNSTARTED

```mach
pub val STATE_UNSTARTED:     SubprocessState = 0
```

## val STATE_SPAWN_FAILURE

```mach
pub val STATE_SPAWN_FAILURE: SubprocessState = 1
```

## val STATE_RUNNING

```mach
pub val STATE_RUNNING:       SubprocessState = 2
```

## val STATE_EXITED

```mach
pub val STATE_EXITED:        SubprocessState = 3
```

## val STATE_SIGNALED

```mach
pub val STATE_SIGNALED:      SubprocessState = 4
```

## val STATE_REAPED

```mach
pub val STATE_REAPED:        SubprocessState = 7
```

## def Request

```mach
pub def Request: u8
```

the termination a supervisor asked for, mirrored from the cancellation
scope's reason: the oracle in mach.lang.validation records this kind, so
the codes are the scope's 1.x reason codes and never change

## val REQUEST_ACTIVE

```mach
pub val REQUEST_ACTIVE:    Request = 0
```

## val REQUEST_CANCELLED

```mach
pub val REQUEST_CANCELLED: Request = 1
```

## val REQUEST_TIMED_OUT

```mach
pub val REQUEST_TIMED_OUT: Request = 2
```

## val REQUEST_DESTROYED

```mach
pub val REQUEST_DESTROYED: Request = 3
```

## val REQUEST_INVALID

```mach
pub val REQUEST_INVALID:   Request = 4
```

## fun request_of

```mach
pub fun request_of(reason: cnc.Reason) Request;
```

## tag Cause

```mach
pub tag Cause: u8 {
    alloc:  A.Error;
    exec:   exec.Error;
    io:     io_error.Error;
    native: i64;
    attempted;
    idle;
    not_running;
    invalid_status;
    transition;
    request;
    incomplete;
    capture_released;
    capture_running;
    capture_unfinished;
    capture_release;
    termination;
    reap;
    grace_negative;
    grace_large;
}
```

a supervised process's refusal, named with the identity the supervisor
knows it by (its pathname, or "subprocess" before one is recorded): the
allocator's, the process layer's, a capture file's, or a lifecycle the
caller drove out of order

## rec Error

```mach
pub rec Error;
```

## fun failure

```mach
pub fun failure(identity: str, cause: Cause) Error;
```

## fun cause_text

```mach
pub fun cause_text(c: Cause) str;
```

## fun text

```mach
pub fun text(a: *A.Allocator, e: Error) str;
```

"identity: cause", owned by `a` (released with str_free); the static
fallback when even that cannot be formatted

## rec SubprocessTerminal

```mach
pub rec SubprocessTerminal;
```

## rec CaptureIoResult

```mach
pub rec CaptureIoResult;
```

## rec OwnedCapture

```mach
pub rec OwnedCapture;
```

## rec OwnedSubprocess

```mach
pub rec OwnedSubprocess;
```

## fun init

```mach
pub fun init(p: *OwnedSubprocess);
```

## fun set_deadline

```mach
pub fun set_deadline(p: *OwnedSubprocess, deadline: tm.Time) bool;
```

## fun bound_seconds

```mach
pub fun bound_seconds(p: *OwnedSubprocess, seconds: i64) bool;
```

## fun deadline_reached

```mach
pub fun deadline_reached(p: *OwnedSubprocess, now: tm.Time) bool;
```

## fun expired

```mach
pub fun expired(p: *OwnedSubprocess) bool;
```

## fun dnit

```mach
pub fun dnit(a: *A.Allocator, p: *OwnedSubprocess);
```

## fun spawn

```mach
pub fun spawn(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8) err[Error];
```

## fun spawn_in

```mach
pub fun spawn_in(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8,
cwd: str) err[Error];
```

## fun spawn_captured

```mach
pub fun spawn_captured(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8,
identity: str, limit: usize) err[Error];
```

## fun spawn_captured_input

```mach
pub fun spawn_captured_input(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8,
stdin_fd: i32, capture_stderr: bool, identity: str, limit: usize) err[Error];
```

## fun spawn_grouped

```mach
pub fun spawn_grouped(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8) err[Error];
```

## fun spawn_in_grouped

```mach
pub fun spawn_in_grouped(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8,
cwd: str) err[Error];
```

## fun spawn_captured_grouped

```mach
pub fun spawn_captured_grouped(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: **u8,
identity: str, limit: usize) err[Error];
```

## fun running

```mach
pub fun running(p: *OwnedSubprocess) bool;
```

## fun pid

```mach
pub fun pid(p: *OwnedSubprocess) i64;
```

## fun wait

```mach
pub fun wait(a: *A.Allocator, p: *OwnedSubprocess) res[SubprocessTerminal, Error];
```

## fun wait_any

```mach
pub fun wait_any(a: *A.Allocator, owners: *OwnedSubprocess, count: u32, scope: *cnc.Scope, source: *events.Source) res[*OwnedSubprocess, Error];
```

## fun finish_capture

```mach
pub fun finish_capture(a: *A.Allocator, p: *OwnedSubprocess, retain: bool) res[CaptureIoResult, Error];
```

## fun abandon_capture

```mach
pub fun abandon_capture(p: *OwnedSubprocess);
```

## fun capture_bytes

```mach
pub fun capture_bytes(p: *OwnedSubprocess, len: *usize) *u8;
```

## fun release_capture

```mach
pub fun release_capture(a: *A.Allocator, p: *OwnedSubprocess) err[Error];
```

## fun timeout

```mach
pub fun timeout(a: *A.Allocator, p: *OwnedSubprocess, grace: duration.Duration) res[SubprocessTerminal, Error];
```

## fun cancel

```mach
pub fun cancel(a: *A.Allocator, p: *OwnedSubprocess, grace: duration.Duration) res[SubprocessTerminal, Error];
```

