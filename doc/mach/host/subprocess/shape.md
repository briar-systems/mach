[mach](../../README.md) / [host](../README.md) / [subprocess](README.md) / **shape**

# mach.host.subprocess.shape

what a supervised child looks like to its supervisor, whatever the host: where an owner is in its
lifecycle, how a child ended, what became of its capture, and how a spawn is asked for. the
members of mach.host.subprocess share these, so a caller reads one shape on every host

Source: [`src/host/subprocess/shape.mach`](../../../../src/host/subprocess/shape.mach)

| Type | | |
| :--- | :--- | :--- |
| [`SubprocessState`](#subprocessstate) | `def` | where an owner is in its lifecycle |
| [`SubprocessTerminal`](#subprocessterminal) | `rec` | how a child ended |
| [`CaptureIoResult`](#captureioresult) | `rec` | what happened to a capture file |
| [`SpawnOptions`](#spawnoptions) | `rec` | how spawn starts a child, spawn_options giving the defaults |

| On `SubprocessState` | |
| :--- | :--- |
| [`STATE_UNSTARTED`](#subprocessstate) | fresh from init, nothing spawned |
| [`STATE_SPAWN_FAILURE`](#subprocessstate) | the one spawn attempt started no child |
| [`STATE_RUNNING`](#subprocessstate) | a child is running and not yet reaped |
| [`STATE_EXITED`](#subprocessstate) | the child exited with a code, and as a terminal kind, ended that way |
| [`STATE_SIGNALED`](#subprocessstate) | the child was ended by a signal, and as a terminal kind, ended that way |
| [`STATE_REAPED`](#subprocessstate) | the terminal was observed and the child reaped, so the owner is done |

| On `SpawnOptions` | |
| :--- | :--- |
| [`spawn_options`](#spawn_options) | the options of a plain spawn: this process's directory, its streams and its group, with nothing captured |

---

## SubprocessState

```mach
pub def SubprocessState: u8
```

where an owner is in its lifecycle

| Name | Value | |
| :--- | :--- | :--- |
| `STATE_UNSTARTED` | `0` | fresh from init, nothing spawned |
| `STATE_SPAWN_FAILURE` | `1` | the one spawn attempt started no child |
| `STATE_RUNNING` | `2` | a child is running and not yet reaped |
| `STATE_EXITED` | `3` | the child exited with a code, and as a terminal kind, ended that way |
| `STATE_SIGNALED` | `4` | the child was ended by a signal, and as a terminal kind, ended that way |
| `STATE_REAPED` | `7` | the terminal was observed and the child reaped, so the owner is done |

## SubprocessTerminal

```mach
pub rec SubprocessTerminal {
    kind: SubprocessState;
    code: i32;
}
```

how a child ended

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `SubprocessState` | STATE_EXITED or STATE_SIGNALED once observed, else the owner's state |
| `code` | `i32` | the exit code, or the signal number |

## CaptureIoResult

```mach
pub rec CaptureIoResult {
    bytes:        usize;
    truncated:    bool;
    short_read:   bool;
    read_failed:  bool;
    write_failed: bool;
    close_failed: bool;
    closed:       bool;
    owned:        bool;
}
```

what happened to a capture file

| Field | Type | |
| :--- | :--- | :--- |
| `bytes` | `usize` | captured bytes read back, kept only when owned |
| `truncated` | `bool` | the child wrote more than the capture limit |
| `short_read` | `bool` | the read back ended before the bytes the file held |
| `read_failed` | `bool` | the read back failed, or the capture was abandoned |
| `write_failed` | `bool` | the capture file could not be opened |
| `close_failed` | `bool` | closing the capture file failed |
| `closed` | `bool` | the capture file is closed |
| `owned` | `bool` | the owner still holds the captured bytes |

## SpawnOptions

```mach
pub rec SpawnOptions {
    cwd:            str;
    stdin:          usize;
    grouped:        bool;
    capture:        str;
    capture_stderr: bool;
    capture_limit:  usize;
}
```

how spawn starts a child, spawn_options giving the defaults

| Field | Type | |
| :--- | :--- | :--- |
| `cwd` | `str` | directory the child starts in, or empty to inherit this process's |
| `stdin` | `usize` | handle for the child's stdin, or system_os.INVALID_HANDLE to inherit |
| `grouped` | `bool` | place the child as the leader of its own process group, so a termination reaches every descendant that stays in it |
| `capture` | `str` | path of a file, created or truncated, that receives the child's stdout, or empty to inherit stdout |
| `capture_stderr` | `bool` | send the child's stderr to the capture file too |
| `capture_limit` | `usize` | the most captured bytes finish_capture keeps, and wait_any cancels a child whose capture grows past it |

### spawn_options

```mach
pub fun spawn_options() SpawnOptions
```

the options of a plain spawn: this process's directory, its streams and its
group, with nothing captured

