# std.process.events

## def Kind

```mach
pub def Kind: u8
```

## val SERVICE_STOP

```mach
pub val SERVICE_STOP:  Kind = 0
```

## val CONSOLE_CLOSE

```mach
pub val CONSOLE_CLOSE: Kind = 1
```

## val TERMINATE

```mach
pub val TERMINATE:     Kind = 2
```

## val INTERRUPT

```mach
pub val INTERRUPT:     Kind = 3
```

## val RELOAD

```mach
pub val RELOAD:        Kind = 4
```

## rec Event

```mach
pub rec Event;
```

## rec Capabilities

```mach
pub rec Capabilities;
```

## rec Source

```mach
pub rec Source;
```

## fun make

```mach
pub fun make(source: *Source) err[io_error.Error];
```

claim the process's event source with its own wait queue

ret: EBUSY when another source owns the process, or the native failure

## fun make_runtime

```mach
pub fun make_runtime(source: *Source, runtime: *io_runtime.Runtime) err[io_error.Error];
```

the source must close before the runtime whose queue it borrows

## fun capabilities

```mach
pub fun capabilities() Capabilities;
```

## fun notify_service_stop

```mach
pub fun notify_service_stop(source: *Source) err[io_error.Error];
```

windows service handlers call this for stop, shutdown, or preshutdown control

## fun next

```mach
pub fun next(source: *Source) res[opt[Event], io_error.Error];
```

events are coalesced by kind and drained in service-to-reload priority order

ret: the next pending event, absent when none is pending, or EBADF for a
     source that is not the open owner

## fun begin_runtime_drain

```mach
pub fun begin_runtime_drain(
source: *Source,
runtime: *io_runtime.Runtime,
event: Event,
) res[lifecycle.CloseResult, io_error.Error];
```

## fun wait

```mach
pub fun wait(source: *Source, timeout_ms: i32) res[bool, io_error.Error];
```

standalone sources may wait directly, runtime-attached sources use runtime wait

ret: whether an event is pending after the wait, or the failure

## fun close

```mach
pub fun close(source: *Source) err[io_error.Error];
```

release the process's event source: the native handlers are restored, the
owned queue closed and the runtime registration released; the first
refusal is reported after every step has run

