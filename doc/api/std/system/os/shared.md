# std.system.os.shared

## rec Timespec

```mach
pub rec Timespec;
```

seconds and nanoseconds from a clock source

sec: whole seconds
nsec: nanoseconds within second [0, 999999999]

## rec IoQueue

```mach
pub rec IoQueue;
```

native wait queue state owned by the portable completion runtime

## rec DirectoryEntry

```mach
pub rec DirectoryEntry;
```

the name is borrowed until the next cursor operation, including eof or failure

## rec DirectoryInitResult

```mach
pub rec DirectoryInitResult;
```

initialization failure and failed duplicate cleanup remain independent

## val PUBLICATION_CLAIMS

```mach
pub val PUBLICATION_CLAIMS:              i64 = 1
```

independently qualified cooperative publication capabilities

## val PUBLICATION_RETAIN_REPLACE

```mach
pub val PUBLICATION_RETAIN_REPLACE:      i64 = 2
```

## val PUBLICATION_READ_RETAIN_REPLACE

```mach
pub val PUBLICATION_READ_RETAIN_REPLACE: i64 = 4
```

## val SEEK_SET

```mach
pub val SEEK_SET: i32 = 0
```

seek whence

## val SEEK_CUR

```mach
pub val SEEK_CUR: i32 = 1
```

## val SEEK_END

```mach
pub val SEEK_END: i32 = 2
```

## val PROT_NONE

```mach
pub val PROT_NONE:  u32 = 0b000
```

memory protection flags

## val PROT_READ

```mach
pub val PROT_READ:  u32 = 0b001
```

## val PROT_WRITE

```mach
pub val PROT_WRITE: u32 = 0b010
```

## val PROT_EXEC

```mach
pub val PROT_EXEC:  u32 = 0b100
```

## val ADVISE_NORMAL

```mach
pub val ADVISE_NORMAL:     u32 = 0
```

memory advisory hints

## val ADVISE_RANDOM

```mach
pub val ADVISE_RANDOM:     u32 = 1
```

## val ADVISE_SEQUENTIAL

```mach
pub val ADVISE_SEQUENTIAL: u32 = 2
```

## val ADVISE_WILL_NEED

```mach
pub val ADVISE_WILL_NEED:  u32 = 3
```

## val ADVISE_DONT_NEED

```mach
pub val ADVISE_DONT_NEED:  u32 = 4
```

## val HUGE_2MB

```mach
pub val HUGE_2MB: usize = 2097152
```

standard huge page sizes

## val HUGE_1GB

```mach
pub val HUGE_1GB: usize = 1073741824
```

## val HEAP_RESERVE

```mach
pub val HEAP_RESERVE: usize = 268435456
```

heap region reserve size (256MB)

## val NOT_FOUND

```mach
pub val NOT_FOUND: i64 = -1
```

platform-agnostic sentinels

## val PROCESS_EXITED

```mach
pub val PROCESS_EXITED:    u8 = 1
```

a process state observation, with a full native windows exit code

## val PROCESS_SIGNALED

```mach
pub val PROCESS_SIGNALED:  u8 = 2
```

## val PROCESS_STOPPED

```mach
pub val PROCESS_STOPPED:   u8 = 3
```

## val PROCESS_CONTINUED

```mach
pub val PROCESS_CONTINUED: u8 = 4
```

## rec ProcessStatus

```mach
pub rec ProcessStatus;
```

## val PROCESS_OP_WAIT

```mach
pub val PROCESS_OP_WAIT:      u8 = 1
```

## val PROCESS_OP_STATUS

```mach
pub val PROCESS_OP_STATUS:    u8 = 2
```

## val PROCESS_OP_CLOSE

```mach
pub val PROCESS_OP_CLOSE:     u8 = 3
```

## val PROCESS_OP_SPAWN

```mach
pub val PROCESS_OP_SPAWN:     u8 = 4
```

## val PROCESS_OP_READ

```mach
pub val PROCESS_OP_READ:      u8 = 5
```

## val PROCESS_OP_PIPE

```mach
pub val PROCESS_OP_PIPE:      u8 = 6
```

## val PROCESS_OP_TERMINATE

```mach
pub val PROCESS_OP_TERMINATE: u8 = 7
```

## rec ProcessWaitError

```mach
pub rec ProcessWaitError;
```

## rec WaitResult

```mach
pub rec WaitResult;
```

pid is positive for an observation, zero for no event or an error

## fun posix_status

```mach
pub fun posix_status(raw: i32, continued: bool) ProcessStatus;
```

native producers identify their own continued encoding before decoding

## fun has_exited

```mach
pub fun has_exited(status: ProcessStatus) bool;
```

## fun exit_code

```mach
pub fun exit_code(status: ProcessStatus) u32;
```

## fun was_signaled

```mach
pub fun was_signaled(status: ProcessStatus) bool;
```

## fun term_signal

```mach
pub fun term_signal(status: ProcessStatus) i32;
```

## fun was_stopped

```mach
pub fun was_stopped(status: ProcessStatus) bool;
```

## fun stop_signal

```mach
pub fun stop_signal(status: ProcessStatus) i32;
```

## fun was_continued

```mach
pub fun was_continued(status: ProcessStatus) bool;
```

