# std.io.error

## def Kind

```mach
pub def Kind: u8
```

## val INTERRUPTED

```mach
pub val INTERRUPTED:        Kind = 0
```

## val WOULD_BLOCK

```mach
pub val WOULD_BLOCK:        Kind = 1
```

## val TIMEOUT

```mach
pub val TIMEOUT:            Kind = 2
```

## val CANCELLED

```mach
pub val CANCELLED:          Kind = 3
```

## val REFUSED

```mach
pub val REFUSED:            Kind = 4
```

## val RESET

```mach
pub val RESET:              Kind = 5
```

## val ABORTED

```mach
pub val ABORTED:            Kind = 6
```

## val CLOSED

```mach
pub val CLOSED:             Kind = 7
```

## val ADDRESS_IN_USE

```mach
pub val ADDRESS_IN_USE:     Kind = 8
```

## val UNREACHABLE

```mach
pub val UNREACHABLE:        Kind = 9
```

## val PERMISSION

```mach
pub val PERMISSION:         Kind = 10
```

## val RESOURCE_EXHAUSTED

```mach
pub val RESOURCE_EXHAUSTED: Kind = 11
```

## val INVALID

```mach
pub val INVALID:            Kind = 12
```

## val UNSUPPORTED

```mach
pub val UNSUPPORTED:        Kind = 13
```

## val OTHER

```mach
pub val OTHER:              Kind = 14
```

## def Operation

```mach
pub def Operation: u8
```

## val OP_UNKNOWN

```mach
pub val OP_UNKNOWN:  Operation = 0
```

## val OP_OPEN

```mach
pub val OP_OPEN:     Operation = 1
```

## val OP_CLOSE

```mach
pub val OP_CLOSE:    Operation = 2
```

## val OP_READ

```mach
pub val OP_READ:     Operation = 3
```

## val OP_WRITE

```mach
pub val OP_WRITE:    Operation = 4
```

## val OP_CREATE

```mach
pub val OP_CREATE:   Operation = 5
```

## val OP_BIND

```mach
pub val OP_BIND:     Operation = 6
```

## val OP_LISTEN

```mach
pub val OP_LISTEN:   Operation = 7
```

## val OP_ACCEPT

```mach
pub val OP_ACCEPT:   Operation = 8
```

## val OP_CONNECT

```mach
pub val OP_CONNECT:  Operation = 9
```

## val OP_RECEIVE

```mach
pub val OP_RECEIVE:  Operation = 10
```

## val OP_SEND

```mach
pub val OP_SEND:     Operation = 11
```

## val OP_SHUTDOWN

```mach
pub val OP_SHUTDOWN: Operation = 12
```

## val OP_OPTION

```mach
pub val OP_OPTION:   Operation = 13
```

## val OP_SEEK

```mach
pub val OP_SEEK:     Operation = 14
```

## val OP_SYNC

```mach
pub val OP_SYNC:     Operation = 15
```

## val OP_SUBMIT

```mach
pub val OP_SUBMIT:   Operation = 16
```

## val OP_WAIT

```mach
pub val OP_WAIT:     Operation = 17
```

## val OP_WAKE

```mach
pub val OP_WAKE:     Operation = 18
```

## rec Error

```mach
pub rec Error;
```

## fun from_code

```mach
pub fun from_code(code: i64, operation: Operation) Error;
```

## fun message

```mach
pub fun message(error: Error) str;
```

## fun retryable

```mach
pub fun retryable(error: Error) bool;
```

