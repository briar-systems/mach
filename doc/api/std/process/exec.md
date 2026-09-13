# std.process.exec

## rec Child

```mach
pub rec Child;
```

handle to a spawned child process

pid: process id of the child
pgid: 0 if the child was not placed in its own group (see spawn_grouped /
      spawn_redirected_grouped); otherwise the group token needed to signal
      the whole group - on linux/darwin this is the posix process group id
      (pgid == pid, since the child is its own group's leader); on windows
      it is a tracked group token backed by the retained child process

## rec Failure

```mach
pub rec Failure;
```

a failed native process operation

code: the negative mapped OS error
native_code: the exact windows GetLastError; zero on posix, whose errno is `code`
operation: the os.PROCESS_OP_* stage that failed
child: the unreaped child transferred to the caller, pid zero for none

## rec OutputFailure

```mach
pub rec OutputFailure;
```

the drain of a captured child's output failed

read: the reader's failure, with the bytes delivered before it
child: the unreaped child transferred to the caller when the wait that
       followed the drain failed too, pid zero when the wait reaped it
wait: that wait's native failure, absent when the wait succeeded

## tag Error

```mach
pub tag Error: u8 {
    native: Failure;
    output: OutputFailure;
    alloc:  A.Error;
    env:    env.EnvError;
    empty_name;
    unset;
    not_found;
    ungrouped;
    unsupported;
    query: io_error.Error;
}
```

why a process operation failed

native: a spawn, pipe, wait, status query, close or termination refused
             by the platform; the child it retains is the caller's to wait
             for again, or to terminate first, before releasing ownership
output: the capture drain failed (the wait's outcome is inside)
alloc: an owned path copy was refused
env: the PATH read failed
empty_name: the program name is empty
unset: PATH is not set
not_found: the name is on no directory of the search list
ungrouped: the child was not spawned into a group, so no group can be signalled
unsupported: process groups do not exist on this platform
query: the name was found nowhere, and at least one candidate could not
             be examined (the last such failure); the search continues past
             a refused query the way execvp does, so a hit still wins

## fun retained

```mach
pub fun retained(error: Error) Child;
```

the child an error still owns: pid zero when none was retained. do not
discard an error while it owns a child; wait for it or terminate it first

## fun error_message

```mach
pub fun error_message(error: Error) str;
```

a rendering of the error for diagnostics

## rec Reaped

```mach
pub rec Reaped;
```

a child reaped by wait_any

child: the reaped child's handle
status: its exit status

## rec Output

```mach
pub rec Output;
```

result of running a child process and collecting its stdout

status: exit status of the child process
bytes: the child's complete stdout, owned by the caller's allocator

## fun run

```mach
pub fun run(pathname: str, argv: **u8, envp: **u8) res[ExitStatus, Error];
```

spawn a process and wait for it to exit

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: exit status, or an error retaining an unreaped child

## fun run_shell

```mach
pub fun run_shell(command: str, cwd: str, envp: **u8) res[ExitStatus, Error];
```

run a command through the host command interpreter, from `cwd`

The interpreter and its command-line conventions are the platform's, not the
caller's: posix hands `sh -c <command>` straight to execve, while windows
resolves %ComSpec% and emits `"<comspec>" /s /c "<command verbatim>"`. That
split matters because a command line's encoding is a property of the program
being spawned - the CRT argv convention every other spawn here uses is not
what cmd.exe parses, so a caller assembling its own shell invocation would
have to know which convention applies and would get it wrong.

`cwd` is applied in the child, so the caller's own working directory is
untouched and the directory never enters the command line.

command: the command line to run
cwd: working directory for the child, or nil to inherit
envp: null-terminated environment array, or nil to inherit
ret: exit status, or an error retaining an unreaped child

## fun output

```mach
pub fun output(a: *A.Allocator, pathname: str, argv: **u8, envp: **u8) res[Output, Error];
```

run a process and collect its complete standard output

spawns the child with stdout bound to a pipe and drains it to end-of-file
through io.read_all over the pipe's read end, so output of any length is
captured and the child never blocks on a full pipe. a wait is attempted even
when the drain fails, and a failed wait returns the remaining child owner.

a: allocator for the captured output
pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: the exit status and captured stdout, or an error

## fun spawn

```mach
pub fun spawn(pathname: str, argv: **u8, envp: **u8) res[Child, Error];
```

start a process without waiting

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: child handle, or the spawn failure

## fun spawn_redirected

```mach
pub fun spawn_redirected(pathname: str, argv: **u8, envp: **u8, stdout_fd: i32, stderr_fd: i32) res[Child, Error];
```

start a process without waiting, with its stdout and/or
stderr bound to caller-supplied descriptors.

the parent keeps ownership of the passed descriptors (close them after the
child exits, or after the spawn to let a pipe's read end see EOF); -1 leaves
that stream inherited. stdin is always inherited.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
stdout_fd: descriptor for the child's stdout, or -1 to inherit
stderr_fd: descriptor for the child's stderr, or -1 to inherit
ret: child handle, or the spawn failure

## fun spawn_grouped

```mach
pub fun spawn_grouped(pathname: str, argv: **u8, envp: **u8) res[Child, Error];
```

start a process without waiting, placed as the leader of its own new
process group

a caller that spawns a tree of children (a test runner, a build-step
executor) uses this instead of spawn so terminate_group can later reach the
whole tree at once: any descendant that does not itself call setpgid
inherits this group from its parent.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
ret: child handle, or the spawn failure

## fun spawn_redirected_grouped

```mach
pub fun spawn_redirected_grouped(pathname: str, argv: **u8, envp: **u8, stdout_fd: i32, stderr_fd: i32) res[Child, Error];
```

start a process without waiting, with its stdout and/or stderr bound to
caller-supplied descriptors, placed as the leader of its own new process
group (see spawn_grouped)

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
stdout_fd: descriptor for the child's stdout, or -1 to inherit
stderr_fd: descriptor for the child's stderr, or -1 to inherit
ret: child handle, or the spawn failure

## fun spawn_in

```mach
pub fun spawn_in(pathname: str, argv: **u8, envp: **u8, cwd: str) res[Child, Error];
```

start a process in a given working directory without waiting

the directory is applied in the CHILD after the fork and before the exec, so
this process's own working directory is never disturbed and two spawns into
different directories cannot race each other.

pathname: path to executable
argv: null-terminated argument array
envp: null-terminated environment array
cwd: directory to run in, or empty to inherit this process's
ret: child handle, or the spawn failure

## fun spawn_redirected_in

```mach
pub fun spawn_redirected_in(pathname: str, argv: **u8, envp: **u8, cwd: str,
stdin_fd: i32, stdout_fd: i32,
stderr_fd: i32) res[Child, Error];
```

start a process in a given working directory with its standard streams bound
to caller-supplied descriptors

cwd: directory to run in, or empty to inherit this process's
stdin_fd: descriptor for the child's stdin, or -1 to inherit
stdout_fd: descriptor for the child's stdout, or -1 to inherit
stderr_fd: descriptor for the child's stderr, or -1 to inherit
ret: child handle, or the spawn failure

## fun spawn_redirected_in_grouped

```mach
pub fun spawn_redirected_in_grouped(pathname: str, argv: **u8, envp: **u8, cwd: str,
stdin_fd: i32, stdout_fd: i32,
stderr_fd: i32) res[Child, Error];
```

start a process in a given working directory, with redirected streams, as the
leader of its own group

this is the form to reach for when the child may spawn children of its own: a
build step running a shell that runs a compiler. terminate_group then reaches
the whole tree, where terminate_child would leave the grandchildren orphaned
and still running.

ret: child handle whose pgid identifies the group, or the spawn failure

## fun terminate_child

```mach
pub fun terminate_child(child: Child) err[Error];
```

forcefully stop a child without reaping it

the child remains waitable after this succeeds. callers must still pass it
to wait (or collect it through wait_any) to obtain its status and release
retained platform resources.

terminate_child, wait, and wait_any must not run concurrently for the same
child. once one of the wait operations begins reaping a child, its numeric
process ID can be released for reuse.

child: child handle from spawn
ret: the native failure, which keeps the child with the caller

## fun terminate_group

```mach
pub fun terminate_group(child: Child) err[Error];
```

signal every process in a child's group, reaching descendants that spawned
their own children in turn

`ungrouped` when `child` was not created through spawn_grouped /
spawn_redirected_grouped (child.pgid == 0): there is no group to signal.
member processes remain individually waitable afterward, exactly as
terminate_child leaves a single child waitable.

child: child handle from spawn_grouped or spawn_redirected_grouped
ret: the native failure, which keeps the child with the caller

## fun wait

```mach
pub fun wait(child: Child) res[ExitStatus, Error];
```

wait for a child process to exit

a signal that interrupts the wait is not a failure and is not reported as
one: the wait is reissued until the child is reaped. the os layer bounds its
own retries and surfaces exhaustion as EINTR, so a wrapper that treated that
sentinel as terminal would turn a transient interruption into a permanent
error and leave the child unreaped.

child: child handle from spawn
ret: exit status, or a typed error retaining any unreaped child

## fun try_wait

```mach
pub fun try_wait(child: Child) res[opt[ExitStatus], Error];
```

reap a child if it has already exited, without blocking

this is the piece a caller needs to enforce a deadline: blocking in wait
gives up the ability to notice that a scope expired, so a supervisor polls
with this instead and keeps its own timing.

a child that is still running is not an error; the result is simply absent.
an interrupted probe is likewise not an error: it is reissued, so absent
always means still running and never means interrupted.

child: child handle from any spawn
ret: the exit status if it has terminated, absent if it is still running,
       or a typed error retaining any unreaped child

## fun wait_any

```mach
pub fun wait_any() res[Reaped, Error];
```

block until any child of this process exits

POSIX wait(-1) semantics on every platform (the windows layer emulates the
-1 pid by waiting across its tracked child handles). the returned pair
identifies which child was reaped and how it exited, so a caller juggling
several children can map the exit back to its work item. errs when there is
no child to wait for.

serialize waits on the same child, and wait_any with all waits and termination
an error without a selected child leaves every existing child owner with its caller

an interrupting signal is retried rather than reported, as in wait.

ret: the reaped child and its exit status, or a typed wait error

## rec Supervised

```mach
pub rec Supervised;
```

how a supervised child ended

## fun wait_within

```mach
pub fun wait_within(child: Child, scope: *cancel.Scope) res[Supervised, Error];
```

run a child to completion under a cancellation scope, terminating its whole
group if the scope ends first

this is the piece that makes a deadline mean something for a process. a scope
with a deadline is only a promise until somebody enforces it, and enforcing it
on a single child is not enough when that child spawned children of its own: a
build step that runs a shell that runs a compiler leaves the compiler running
if only the shell is signalled. so the group is what gets terminated, which is
why the child should come from one of the grouped spawns.

successful supervision reaps the child. a wait or termination failure returns
its retained child for explicit retry or cleanup, alongside the original cause.

a child that is not in a group is still supervised; expiry then terminates
just that child, and any grandchildren it started survive. that is a weaker
guarantee, not a silent one: prefer a grouped spawn when the child may spawn.

child: child handle, ideally from a grouped spawn
scope: the scope whose deadline or cancellation bounds the child
ret: how it ended, or a typed error retaining any unreaped child

## fun exited

```mach
pub fun exited(status: ExitStatus) bool;
```

check if the process exited normally

status: exit status to inspect
ret: true if the process called exit

## fun code

```mach
pub fun code(status: ExitStatus) u32;
```

get the exit code of a normally exited process

status: exit status to inspect
ret: full u32 exit code, only valid if exited() is true

## fun signaled

```mach
pub fun signaled(status: ExitStatus) bool;
```

check if the process was killed by a signal

status: exit status to inspect
ret: true if terminated by signal

## fun signal

```mach
pub fun signal(status: ExitStatus) i32;
```

get the signal number that killed the process

status: exit status to inspect
ret: signal number

## fun exit

```mach
pub fun exit(code: i64);
```

terminate the current process

code: exit code

## fun resolve

```mach
pub fun resolve(a: *A.Allocator, name: str) res[str, Error];
```

resolve a user-named program to a spawnable path using PATH

none of the spawn entry points searches PATH: execve resolves nothing, and
the windows layer passes a non-NULL lpApplicationName. a caller spawning a
program the USER names rather than a fixed path resolves it here first.

PATH is the only source. the current directory is never consulted, so a
binary committed into a working tree cannot shadow the real tool.

a: allocator for the result
name: program name, bare or already pathed
ret: an owned spawnable path (extent is its length plus the terminator,
      released with text.string.str_free), or the failure

## fun resolve_in

```mach
pub fun resolve_in(a: *A.Allocator, name: str, search: str) res[str, Error];
```

resolve a program name against a caller-supplied search list

the search policy, with the environment read left to `resolve`. a caller with
its own list - a configured toolchain directory, a sandbox - searches it here
without going through PATH, and the policy stays testable without a way to
set an environment variable.

`search` is PATH-shaped and is NOT modified: entries separated by `;` on
windows and `:` elsewhere. an EMPTY entry is skipped rather than treated as
the current directory, which is the whole point of this function existing -
letting the OS search would put the working directory ahead of the real tool
on windows.

a name that already spells a path is returned unresolved and unprobed. the
result is owned by the caller in BOTH cases, so the free is unconditional.
returning the borrowed argument for pathed names would make ownership depend
on the value, which is how a caller ends up leaking one path and double
freeing the other.

a refused copy anywhere in the walk is the allocator's error, never a
directory silently skipped: a search that could not look everywhere does
not get to answer `not_found`.

a: allocator for the result
name: program name, bare or already pathed
search: PATH-shaped list of directories, searched in order
ret: an owned spawnable path, or the failure

