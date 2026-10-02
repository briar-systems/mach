# mach.lang.stack

the stack the compiler's recursive readers run on

a reader whose depth follows its input asks exhausted() on each step and
refuses with its located diagnostic once less than MARGIN is left, so no fixed
nesting bound or linker reserve has to be sized for the deepest input. where
the host has threads the driver runs on a thread whose stack it sizes itself
(run), so how deep an input may nest does not follow the platform's main-thread
reserve or ulimit -s. a threadless host runs on the calling thread and the
margin alone holds the bound.

## val WORKER_RESERVE

```mach
pub val WORKER_RESERVE: usize = 67108864
```

the stack run gives its thread. it is address space, committed only as used

## fun exhausted

```mach
pub fun exhausted() bool;
```

whether less than MARGIN is left below the caller's frame. a thread whose
stack std cannot bound answers false, and the reader's fixed bound holds

## fun run

```mach
pub fun run(f: fun(*u8), arg: *u8);
```

run f(arg) to completion on a thread with a WORKER_RESERVE stack

## fun run_reserved

```mach
pub fun run_reserved(f: fun(*u8), arg: *u8, reserve: usize);
```

run f(arg) to completion on a thread with reserve bytes of stack, or on the
calling thread where the host has no threads or refuses one

