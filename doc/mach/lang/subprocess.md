# mach.lang.subprocess

supervised child processes

an OwnedSubprocess owns one child from spawn to reap. it moves through
STATE_UNSTARTED, then STATE_RUNNING or STATE_SPAWN_FAILURE, then
STATE_EXITED or STATE_SIGNALED as the terminal is observed, and ends in
STATE_REAPED. each step happens at most once: an owner spawns once, observes
one terminal and reaps once, so a child is never waited on twice and a pid
is never reused under a stale owner. wait, wait_any, timeout and cancel are
the ways to reach the terminal. a capture, when the spawn asked for one, is
a file the child writes and the owner reads back after the child is done.

## val STATE_UNSTARTED

```mach
pub val STATE_UNSTARTED: SubprocessState = 0
```

fresh from init, nothing spawned

## val STATE_SPAWN_FAILURE

```mach
pub val STATE_SPAWN_FAILURE: SubprocessState = 1
```

the one spawn attempt started no child

## val STATE_RUNNING

```mach
pub val STATE_RUNNING: SubprocessState = 2
```

a child is running and not yet reaped

## val STATE_EXITED

```mach
pub val STATE_EXITED: SubprocessState = 3
```

the child exited with a code, and as a terminal kind, ended that way

## val STATE_SIGNALED

```mach
pub val STATE_SIGNALED: SubprocessState = 4
```

the child was ended by a signal, and as a terminal kind, ended that way

## val STATE_REAPED

```mach
pub val STATE_REAPED: SubprocessState = 7
```

the terminal was observed and the child reaped, so the owner is done

## def Request

```mach
pub def Request: u8
```

the termination a supervisor asked for, mirrored from the cancellation
scope's reason, so the codes are the scope's 1.x reason codes and never change

## val REQUEST_ACTIVE

```mach
pub val REQUEST_ACTIVE: Request = 0
```

no termination was asked for

## val REQUEST_CANCELLED

```mach
pub val REQUEST_CANCELLED: Request = 1
```

the supervisor cancelled the child

## val REQUEST_TIMED_OUT

```mach
pub val REQUEST_TIMED_OUT: Request = 2
```

the child outlived its deadline

## val REQUEST_DESTROYED

```mach
pub val REQUEST_DESTROYED: Request = 3
```

the cancellation scope was destroyed

## val REQUEST_INVALID

```mach
pub val REQUEST_INVALID: Request = 4
```

the scope reported a reason this module does not know

## rec Error

```mach
pub rec Error;
```

a refusal of this module

identity: the pathname of the child, or "subprocess" before one is known,
          borrowed from the owner or the caller
cause: what refused

## fun failure

```mach
pub fun failure(identity: str, cause: Cause) Error;
```

the refusal `cause`, named by `identity`

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

how a child ended

kind: STATE_EXITED or STATE_SIGNALED once observed, else the owner's state
code: the exit code, or the signal number

## rec CaptureIoResult

```mach
pub rec CaptureIoResult;
```

what happened to a capture file

bytes: captured bytes read back and kept
truncated: the child wrote more than the capture limit
short_read: the read back ended before the bytes the file held
read_failed: the read back failed, or the capture was abandoned
write_failed: the capture file could not be opened
close_failed: closing the capture file failed
closed: the capture file is closed
owned: the owner still holds the captured bytes

## rec OwnedSubprocess

```mach
pub rec OwnedSubprocess;
```

the owner of one supervised child, made by init and released by dnit

child: the child's handle while it runs
state: where the owner is in its lifecycle (see STATE_UNSTARTED)
terminal: how the child ended, once observed
requested: the termination a supervisor asked for
capture: the capture file and what was read back from it
has_capture: the spawn asked for a capture
spawn_attempted: spawn has run, so it cannot run again
reap_count: times the child was reaped, never more than one
pathname: the owned copy of the child's pathname, naming its errors
has_deadline: bound set a deadline
deadline: the instant wait_any times the child out
timeout: the bound the deadline came from

## fun init

```mach
pub fun init(p: *OwnedSubprocess);
```

make `p` a fresh owner with nothing spawned

## fun bound

```mach
pub fun bound(p: *OwnedSubprocess, timeout: chrono_duration.Duration) bool;
```

give `p` a deadline `timeout` from now, which wait_any enforces

ret: false when the timeout is not positive, the clock fails, or the owner
     is done

## fun expired

```mach
pub fun expired(p: *OwnedSubprocess) bool;
```

whether the child was terminated for outliving its deadline

## fun dnit

```mach
pub fun dnit(a: *A.Allocator, p: *OwnedSubprocess);
```

release what `p` owns: its pathname, and its capture file and identity. a
second call releases nothing. it does not reap a running child

## rec SpawnOptions

```mach
pub rec SpawnOptions;
```

how spawn starts a child, spawn_options giving the defaults

cwd: directory the child starts in, or empty to inherit this
                process's
stdin: handle for the child's stdin, or system_os.INVALID_HANDLE to
                inherit
grouped: place the child as the leader of its own process group, so a
                termination reaches every descendant that stays in it
capture: path of a file, created or truncated, that receives the
                child's stdout, or empty to inherit stdout
capture_stderr: send the child's stderr to the capture file too
capture_limit: the most captured bytes finish_capture keeps, and wait_any
                cancels a child whose capture grows past it

## fun spawn_options

```mach
pub fun spawn_options() SpawnOptions;
```

the options of a plain spawn: this process's directory, its streams and its
group, with nothing captured

## fun spawn

```mach
pub fun spawn(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: exec.Environment,
options: SpawnOptions) err[Error];
```

start a child under `p`, which must be fresh from init: an owner spawns at
most once, even when the spawn fails

the pathname, argv and envp are copied, so the caller's buffers may change
as soon as this returns. the owner keeps the pathname to name the child in
its errors. on failure the owner is in STATE_SPAWN_FAILURE and owns nothing.

a: allocator for the owner's copies, which dnit releases
p: the owner, from init
pathname: path to the executable
argv: null-terminated argument array
envp: the child's environment, given entries or this process's inherited
options: directory, stdin, grouping and capture (see SpawnOptions)
ret: nothing, or the refusal named by the pathname

## fun spawn_native

```mach
pub fun spawn_native(a: *A.Allocator, p: *OwnedSubprocess, pathname: str, argv: **u8, envp: exec.NativeEnvironment,
options: SpawnOptions) err[Error];
```

spawn with the child's environment in native units, so an inherited variable
with no UTF-8 spelling reaches it unit for unit. the pathname, argv and
working directory are UTF-8 as spawn takes them, converted once; the
environment is read only during the call. everything else is as spawn

envp: the child's environment, given native entries or this process's inherited

## fun wait

```mach
pub fun wait(a: *A.Allocator, p: *OwnedSubprocess) res[SubprocessTerminal, Error];
```

block until the running child of `p` ends, and reap it

ret: how the child ended, or the refusal when no child is running

## fun wait_any

```mach
pub fun wait_any(a: *A.Allocator, owners: *OwnedSubprocess, count: u32, scope: *sync_cancel.Scope, source: *events.Source) res[*OwnedSubprocess, Error];
```

supervise `count` owners until one of them reaches its terminal, and return
that one reaped

each pass polls the scope, then every running owner: a cancelled or expired
scope cancels or times out the first running owner, an owner past its own
deadline is timed out, and one whose capture outgrew its limit is
cancelled. a terminate, interrupt, console-close or service-stop event from
`source` cancels the scope.

owners: array of `count` owners, some of which may not be running
scope: cancellation scope over all of them, or nil
source: process events that cancel the scope, or nil
ret: the owner that reached its terminal, or the refusal when none is
        running

## fun finish_capture

```mach
pub fun finish_capture(a: *A.Allocator, p: *OwnedSubprocess, retain: bool) res[CaptureIoResult, Error];
```

read the capture of a finished child back and close it, once

retain: keep the captured bytes for capture_bytes, else only count them
ret: what happened to the capture, or the refusal while the child runs or
        after release_capture

## fun abandon_capture

```mach
pub fun abandon_capture(p: *OwnedSubprocess);
```

close the capture without reading it back, marking it failed

## fun capture_bytes

```mach
pub fun capture_bytes(p: *OwnedSubprocess, len: *usize) *u8;
```

the captured bytes retained by finish_capture, borrowed from `p`

len: receives the byte count, 0 when nothing is retained
ret: the bytes, or nil when nothing is retained

## fun release_capture

```mach
pub fun release_capture(a: *A.Allocator, p: *OwnedSubprocess) err[Error];
```

free the captured bytes and the capture identity of a finished capture, once

## fun timeout

```mach
pub fun timeout(a: *A.Allocator, p: *OwnedSubprocess, grace: chrono_duration.Duration) res[SubprocessTerminal, Error];
```

terminate the running child for outliving its deadline and reap it

the child is asked to stop, its whole group when it has one, and given
`grace` to end before it is killed.

grace: how long the child has to stop, rounded up to a millisecond
ret: how the child ended, or the refusal. a refused termination keeps the
       child running and owned, so the request can be retried

## fun cancel

```mach
pub fun cancel(a: *A.Allocator, p: *OwnedSubprocess, grace: chrono_duration.Duration) res[SubprocessTerminal, Error];
```

terminate the running child as cancelled and reap it, as timeout does

