# std.sync.cancel

## tag Reason

```mach
pub tag Reason: u8 { active; cancelled; timed_out; destroyed; invalid; }
```

why a scope is no longer active, or that it still is

active: no cancellation has been requested
cancelled: cancelled explicitly through this scope or an ancestor
timed_out: an absolute deadline owned by this scope or an ancestor passed
destroyed: the scope was destroyed, or the call reached a destroyed scope
invalid: the call was refused (nil operands, a registration in use)

## tag StateError

```mach
pub tag StateError: u8 { invalid; destroyed; busy; }
```

a refused scope or registration operation

invalid: nil operands, a deadline outside the target range, a scope
           parented to itself, or a scope that was never made
destroyed: the scope (or the parent) has already been destroyed
busy: destruction refused while children, registrations, waiters,
           running callbacks or in-flight callers remain

## rec Deadline

```mach
pub rec Deadline;
```

the effective absolute monotonic deadline of a scope

at: the earliest deadline along the ancestor chain
owner: the scope that declared it, borrowed from the tree

## def CancelFun

```mach
pub def CancelFun: fun(ptr, Reason)
```

## def Transition

```mach
pub def Transition:    res[bool, StateError]
```

## def Operation

```mach
pub def Operation:     err[StateError]
```

## def DeadlineQuery

```mach
pub def DeadlineQuery: res[opt[Deadline], StateError]
```

## rec Registration

```mach
pub rec Registration;
```

## rec Scope

```mach
pub rec Scope;
```

## fun make_root

```mach
pub fun make_root(scope: *Scope, has_deadline: bool, deadline: time.Time) Operation;
```

scopes are fixed in memory and own no target handle or allocation
callers stop starting new calls before reclaiming a destroyed scope

## fun make_child

```mach
pub fun make_child(scope: *Scope, parent: *Scope, has_deadline: bool, deadline: time.Time) Operation;
```

## fun init_registration

```mach
pub fun init_registration(registration: *Registration) Operation;
```

initialization is valid before first attachment or after terminal completion

## fun attach

```mach
pub fun attach(scope: *Scope, registration: *Registration, callback: CancelFun, context: ptr) Reason;
```

successful registration is the ownership linearization point

## fun attach_with_completion

```mach
pub fun attach_with_completion(scope: *Scope, registration: *Registration, callback: CancelFun, completion: CancelFun, context: ptr) Reason;
```

completion publishes external work and must call finish under its lifetime lock

## fun finish

```mach
pub fun finish(registration: *Registration) Transition;
```

this must be the completion hook's final registration operation

ret: true when this call completed a cancelled registration, false when the
     registration was not awaiting completion, invalid for nil

## fun unregister

```mach
pub fun unregister(registration: *Registration) Transition;
```

completion wins only while the registration remains owned by its scope

ret: true when this call detached the registration, false when cancellation
     already claimed it (the completion hook owns it), invalid for nil

## fun cancel

```mach
pub fun cancel(scope: *Scope) Transition;
```

ret: true when this call cancelled the scope, false when it was already
     cancelled or timed out

## fun timeout

```mach
pub fun timeout(scope: *Scope) Transition;
```

ret: true when this call timed the scope out, false when it was already
     cancelled or timed out

## fun expire

```mach
pub fun expire(scope: *Scope, now: time.Time) Transition;
```

callers without a runtime may drive absolute deadlines explicitly

ret: true when now reached the effective deadline and this call timed its
     owner out, false when there is no deadline, it has not passed, or the
     owner had already left active

## fun get_deadline

```mach
pub fun get_deadline(scope: *Scope) DeadlineQuery;
```

returns the effective deadline and the ancestor that owns it

ret: some{deadline} when the scope or an ancestor declared one, none when
     the scope has no deadline, destroyed once the scope is gone

## fun reason

```mach
pub fun reason(scope: *Scope) Reason;
```

## fun wait

```mach
pub fun wait(scope: *Scope) Reason;
```

## fun waiter_count

```mach
pub fun waiter_count(scope: *Scope) i64;
```

## fun destroy

```mach
pub fun destroy(scope: *Scope) Operation;
```

destruction rejects children, live registrations, waiters, and callbacks

ret: ok once the scope is destroyed, invalid for a nil or never-made scope,
     destroyed when it already was, busy while children, registrations,
     waiters, running callbacks or in-flight callers remain

