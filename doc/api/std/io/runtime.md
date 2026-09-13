# std.io.runtime

## def Kind

```mach
pub def Kind: u8
```

## val ACCEPT

```mach
pub val ACCEPT:       Kind = 0
```

## val CONNECT

```mach
pub val CONNECT:      Kind = 1
```

## val READ

```mach
pub val READ:         Kind = 2
```

## val WRITE

```mach
pub val WRITE:        Kind = 3
```

## val RECEIVE_FROM

```mach
pub val RECEIVE_FROM: Kind = 4
```

## val SEND_TO

```mach
pub val SEND_TO:      Kind = 5
```

## val FILE_READ

```mach
pub val FILE_READ:    Kind = 6
```

## val FILE_WRITE

```mach
pub val FILE_WRITE:   Kind = 7
```

## val TIMER

```mach
pub val TIMER:        Kind = 8
```

## val PROCESS_WAIT

```mach
pub val PROCESS_WAIT: Kind = 9
```

## val USER_WAKE

```mach
pub val USER_WAKE:    Kind = 10
```

## val SHUTDOWN

```mach
pub val SHUTDOWN:     Kind = 11
```

## val CLOSE

```mach
pub val CLOSE:        Kind = 12
```

## val BIND

```mach
pub val BIND:         Kind = 13
```

## val LISTEN

```mach
pub val LISTEN:       Kind = 14
```

## val RESOLVE

```mach
pub val RESOLVE:      Kind = 15
```

## rec Token

```mach
pub rec Token;
```

## rec Operation

```mach
pub rec Operation;
```

buffer is borrowed from successful submission until its completion is dequeued

## rec Completion

```mach
pub rec Completion;
```

## rec NativeEvent

```mach
pub rec NativeEvent;
```

## rec SourceToken

```mach
pub rec SourceToken;
```

## def DispatchFun

```mach
pub def DispatchFun: fun(ptr, usize, u32, i64, u64) bool
```

## def CancelFun

```mach
pub def CancelFun:   fun(ptr, Token, io_error.Error) i64
```

## def ReleaseFun

```mach
pub def ReleaseFun:  fun(ptr, *Completion)
```

## val CANCELLATION_PENDING

```mach
pub val CANCELLATION_PENDING: i64 = 1
```

## rec WaitPlan

```mach
pub rec WaitPlan;
```

## val OPEN

```mach
pub val OPEN:      u8 = lifecycle.OPEN
```

## val CLOSING

```mach
pub val CLOSING:   u8 = lifecycle.CLOSING
```

## val CLOSED

```mach
pub val CLOSED:    u8 = lifecycle.CLOSED
```

## val DESTROYED

```mach
pub val DESTROYED: u8 = 3
```

## rec Runtime

```mach
pub rec Runtime;
```

one thread owns wait and dequeue, while submission and resolution may be concurrent

## fun aliases

```mach
pub fun aliases(runtime: *Runtime, data: *u8, len: usize) bool;
```

the caller excludes concurrent destruction while querying stable ownership

## fun make

```mach
pub fun make(runtime: *Runtime, capacity: usize) err[io_error.Error];
```

## fun register_source

```mach
pub fun register_source(
runtime: *Runtime,
context: ptr,
dispatch: DispatchFun,
cancel_operation: CancelFun,
release: ReleaseFun,
) res[SourceToken, io_error.Error];
```

## fun retire_source

```mach
pub fun retire_source(runtime: *Runtime, token: SourceToken) res[bool, io_error.Error];
```

## fun release_source

```mach
pub fun release_source(runtime: *Runtime, token: SourceToken) err[io_error.Error];
```

## fun unregister_source

```mach
pub fun unregister_source(runtime: *Runtime, token: SourceToken) err[io_error.Error];
```

## fun source_id

```mach
pub fun source_id(token: SourceToken) u16;
```

## fun submit

```mach
pub fun submit(runtime: *Runtime, operation: Operation) res[Token, io_error.Error];
```

## fun submit_owned

```mach
pub fun submit_owned(runtime: *Runtime, source_token: SourceToken, operation: Operation) res[Token, io_error.Error];
```

## fun submit_scoped

```mach
pub fun submit_scoped(runtime: *Runtime, scope: *cancel.Scope, operation: Operation) res[Token, io_error.Error];
```

prior cancellation is observed or the returned token is atomically scope-owned

## fun submit_scoped_owned

```mach
pub fun submit_scoped_owned(
runtime: *Runtime,
source_token: SourceToken,
scope: *cancel.Scope,
operation: Operation,
) res[Token, io_error.Error];
```

## fun complete

```mach
pub fun complete(runtime: *Runtime, token: Token, bytes: usize, resource: usize, end_of_stream: bool) err[io_error.Error];
```

## fun complete_copy

```mach
pub fun complete_copy(runtime: *Runtime, token: Token, source: *u8, bytes: usize, resource: usize, end_of_stream: bool) err[io_error.Error];
```

copy is atomic with terminal publication and loses cleanly to close or cancellation

## fun complete_batch

```mach
pub fun complete_batch(runtime: *Runtime, token: Token, bytes: usize, items: usize, resource: usize) err[io_error.Error];
```

batch completions report byte progress and completed item count separately

## fun fail

```mach
pub fun fail(runtime: *Runtime, token: Token, failure: io_error.Error) err[io_error.Error];
```

## fun complete_cancellation

```mach
pub fun complete_cancellation(runtime: *Runtime, token: Token) err[io_error.Error];
```

asynchronous sources publish only after target ownership has quiesced

## fun submit_timer

```mach
pub fun submit_timer(runtime: *Runtime, deadline: time.Time, context: usize) res[Token, io_error.Error];
```

deadlines are absolute monotonic instants and never expire early
late delivery is limited only by native scheduling and runtime dequeue latency

## fun submit_timer_scoped

```mach
pub fun submit_timer_scoped(runtime: *Runtime, scope: *cancel.Scope, deadline: time.Time, context: usize) res[Token, io_error.Error];
```

## fun cancel_timer

```mach
pub fun cancel_timer(runtime: *Runtime, token: Token) err[io_error.Error];
```

expiry and cancellation race under one lock and publish one terminal result

## fun poll

```mach
pub fun poll(runtime: *Runtime, output: *Completion, output_capacity: usize) res[usize, io_error.Error];
```

drains completions and expires due deadlines without entering the native wait

## fun prepare_wait

```mach
pub fun prepare_wait(runtime: *Runtime, timeout_ms: i32) res[WaitPlan, io_error.Error];
```

prepares another subsystem to wait on the runtime's native queue

## fun begin_native_control

```mach
pub fun begin_native_control(runtime: *Runtime) err[io_error.Error];
```

## fun end_native_control

```mach
pub fun end_native_control(runtime: *Runtime);
```

## fun wait

```mach
pub fun wait(runtime: *Runtime, output: *Completion, output_capacity: usize, timeout_ms: i32) res[usize, io_error.Error];
```

timeout_ms is relative and uses -1 for an unbounded wait
the caller owns output and may reuse it after this call returns

## fun wake

```mach
pub fun wake(runtime: *Runtime) err[io_error.Error];
```

interrupts a blocked wait, with notifications coalesced by the native queue

## fun begin_close

```mach
pub fun begin_close(runtime: *Runtime, mode: lifecycle.Mode, cause: lifecycle.Cause) res[lifecycle.CloseResult, io_error.Error];
```

close rejects new work and queues one terminal completion for every active operation

## fun close

```mach
pub fun close(runtime: *Runtime) res[bool, io_error.Error];
```

## fun destroy

```mach
pub fun destroy(runtime: *Runtime) err[io_error.Error];
```

destroy is valid only after close completions have been dequeued: a runtime
that is not closed or already destroyed is INVALID, one with live operations,
queued completions, a native controller or a registered source is EBUSY;
a refused destroy keeps every allocation

## fun pending

```mach
pub fun pending(runtime: *Runtime) usize;
```

## fun snapshot

```mach
pub fun snapshot(runtime: *Runtime) lifecycle.Snapshot;
```

