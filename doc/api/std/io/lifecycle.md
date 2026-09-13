# std.io.lifecycle

## tag StateError

```mach
pub tag StateError: u8 {
    invalid;
    closed;
    exhausted;
    active;
    inactive;
    stale;
    busy;
}
```

every way a state transition can be refused

the domain error tag for state machines: lifecycles and their attachments
here, and the queued sinks, channels, pools and resolvers that later lanes
migrate. a case names the fact that refused the transition; the receiver is
left exactly as it was.

invalid: a nil receiver or an argument outside the closed set
closed: the owner has left the open state and admits no new work
exhausted: a bounded counter or table is full
active: the object is already attached, registered or open
inactive: the object is not attached, registered or open
stale: the object belongs to an earlier generation of its owner
busy: outstanding work forbids the transition

## def State

```mach
pub def State: u8
```

## val OPEN

```mach
pub val OPEN:    State = 0
```

## val CLOSING

```mach
pub val CLOSING: State = 1
```

## val CLOSED

```mach
pub val CLOSED:  State = 2
```

## def Mode

```mach
pub def Mode: u8
```

## val GRACEFUL

```mach
pub val GRACEFUL: Mode = 0
```

## val ABORTIVE

```mach
pub val ABORTIVE: Mode = 1
```

## def Cause

```mach
pub def Cause: u8
```

## val EXPLICIT

```mach
pub val EXPLICIT:            Cause = 0
```

## val PROCESS_TERMINATION

```mach
pub val PROCESS_TERMINATION: Cause = 1
```

## val DEADLINE

```mach
pub val DEADLINE:            Cause = 2
```

## val CANCELLATION

```mach
pub val CANCELLATION:        Cause = 3
```

## val FAILURE

```mach
pub val FAILURE:             Cause = 4
```

## rec Lifecycle

```mach
pub rec Lifecycle;
```

## rec Attachment

```mach
pub rec Attachment;
```

## rec Snapshot

```mach
pub rec Snapshot;
```

## rec CloseResult

```mach
pub rec CloseResult;
```

## fun make

```mach
pub fun make(lifecycle: *Lifecycle) err[StateError];
```

initializes a lifecycle in place; the storage is address-bound from here on

## fun attach

```mach
pub fun attach(lifecycle: *Lifecycle, attachment: *Attachment) err[StateError];
```

initializes an attachment in place against an open lifecycle; the attachment
is address-bound until it settles

## fun settle

```mach
pub fun settle(attachment: *Attachment) res[bool, StateError];
```

settles an attachment; true when this settlement completed the owner's close

## fun begin_close

```mach
pub fun begin_close(lifecycle: *Lifecycle, mode: Mode, cause: Cause) CloseResult;
```

## fun begin_process_drain

```mach
pub fun begin_process_drain(lifecycle: *Lifecycle) CloseResult;
```

## fun fail

```mach
pub fun fail(lifecycle: *Lifecycle) CloseResult;
```

## fun snapshot

```mach
pub fun snapshot(lifecycle: *Lifecycle) Snapshot;
```

## fun settle_closed_child

```mach
pub fun settle_closed_child(attachment: *Attachment, child: *Lifecycle) res[bool, StateError];
```

settles the parent's attachment for a child that has fully closed; a child
still open or draining is busy and the attachment stays active

