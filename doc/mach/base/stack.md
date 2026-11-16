[mach](../README.md) / [base](README.md) / **stack**

# mach.base.stack

the stack the compiler's recursive readers run on

a reader whose depth follows its input asks exhausted() on each step and
refuses with its located diagnostic once less than MARGIN is left, so no fixed
nesting bound or linker reserve has to be sized for the deepest input. where
the host has threads the driver runs on a thread whose stack it sizes itself
(run), so how deep an input may nest does not follow the platform's main-thread
reserve or ulimit -s. a threadless host runs on the calling thread and the
margin alone holds the bound.

<sub>source: [`src/base/stack.mach`](../../../src/base/stack.mach) · imported by 29 modules</sub>

| Functions | |
| :--- | :--- |
| [`exhausted`](#exhausted) | whether less than MARGIN is left below the caller's frame |
| [`run`](#run) | run f(arg) to completion on a thread with a WORKER_RESERVE stack |
| [`run_reserved`](#run_reserved) | run f(arg) to completion on a thread with reserve bytes of stack, or on the calling thread where the host has no threads or refuses one |

| Constants | |
| :--- | :--- |
| [`WORKER_RESERVE`](#constants) | the stack run gives its thread |

---

## Functions

### exhausted

```mach
pub fun exhausted() bool
```

whether less than MARGIN is left below the caller's frame. a thread whose
stack std cannot bound answers false, and the reader's fixed bound holds

### run

```mach
pub fun run(f: fun(*u8), arg: *u8)
```

run f(arg) to completion on a thread with a WORKER_RESERVE stack

### run_reserved

```mach
pub fun run_reserved(f: fun(*u8), arg: *u8, reserve: usize)
```

run f(arg) to completion on a thread with reserve bytes of stack, or on the
calling thread where the host has no threads or refuses one. total: a thread
it spawned is always joinable, so a refused join stops the compiler

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `WORKER_RESERVE` | `usize` | `67108864` | the stack run gives its thread |

