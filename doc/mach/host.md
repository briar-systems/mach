[mach](README.md) / **host**

# mach.host

the host the compiler runs on: every primitive whose spelling differs by the
operating system the compiler itself was built for. a compiler module calls
these and holds no host conditional of its own. the target a build produces
is the target module's concern, not this one's

<sub>source: [`src/host.mach`](../../src/host.mach) · imported by 13 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`NativeTextError`](#nativetexterror) | `tag` | why text could not be put in native units |
| [`ProbeKind`](#probekind) | `def` | what a component is, read without following it |
| [`Probe`](#probe) | `rec` | cause: what refused the probe, read only for PROBE_ABSENT and PROBE_FAILED |
| [`Cursor`](#cursor) | `rec` | a contained walk's hold on a path: on posix a descriptor for the directory the next component is read beneath, opened one component at a time; windows reads each prefix by path, beneath the same root |
| [`Fault`](#fault) | `rec` |  |

| On `NativeTextError` | |
| :--- | :--- |
| [`native_text`](#native_text) | `text` in std.runtime.native units, owned and terminated: converted from UTF-8 once on windows, the bytes as they are elsewhere |
| [`native_text_message`](#native_text_message) | the refusal of a native text conversion as text |

| On `ProbeKind` | |
| :--- | :--- |
| [`PROBE_ABSENT`](#probekind) · [`PROBE_FAILED`](#probekind) · [`PROBE_LINK`](#probekind) · [`PROBE_DIR`](#probekind) · [`PROBE_FILE`](#probekind) · [`PROBE_OTHER`](#probekind) | |

| On `Cursor` | |
| :--- | :--- |
| [`cursor_open`](#cursor_open) · [`cursor_close`](#cursor_close) | |
| [`cursor_probe`](#cursor_probe) | the component's kind; windows resolves the prefix against the root |
| [`cursor_make`](#cursor_make) | a directory for a missing component; one a concurrent writer made first is as good |
| [`cursor_unlink`](#cursor_unlink) | remove a stale non-directory component, never following it |
| [`cursor_enter`](#cursor_enter) | step beneath a component already read as a real directory, checking that what was opened is still one enter:   the refusal of a component that cannot be opened, formatted with label, rel, prefix and the native detail invalid: the refusal of one that opened as something else, formatted with label, rel and prefix |
| [`cursor_root`](#cursor_root) | the root itself, which posix opened as a directory; windows reads it by path, so a symlinked root is refused there as any component is |

| On `Fault` | |
| :--- | :--- |
| [`image_digest`](#image_digest) | the sha-256 of the running image's file, read through a handle checked to be the file the process maps |

| Functions | |
| :--- | :--- |
| [`terminate_command`](#terminate_command) | the program that ends a process the host cannot signal directly, and its arguments for the process named by pid, written to argv and ended by nil |
| [`native_text_dnit`](#native_text_dnit) | release a native text; an empty one owns no units |
| [`compare_env_names`](#compare_env_names) | compare two environment variable names in native units by the host's identity, returning -1, 0 or 1, as std.process.env.compare_names_native orders them: ordinal case-insensitive UTF-16 order on windows, byte order elsewhere |
| [`program_resolve`](#program_resolve) | locate an executable the way a shell would: a name with a path separator is used directly and must be an existing executable file; any other name is searched on PATH |

| Constants | |
| :--- | :--- |
| [`OPEN_NOFOLLOW`](#constants) | the open flag that refuses a symlink at the leaf, and the one that keeps an open from blocking on a fifo; 0 where the host's open has neither |
| [`OPEN_NONBLOCK`](#constants) | the open flag that refuses a symlink at the leaf, and the one that keeps an open from blocking on a fifo; 0 where the host's open has neither |
| [`PERMISSIONS`](#constants) | whether the host keeps posix permission bits on a file |
| [`FAULT_UNAVAILABLE`](#constants) | why the running image's identity could not be read: unavailable on this host or in this process, or an internal fault of the reader |
| [`FAULT_INTERNAL`](#constants) | |

---

## NativeTextError

```mach
pub tag NativeTextError: u8 {
    native: i64;
    alloc:  A.Error;
}
```

why text could not be put in native units

| Case | Payload | |
| :--- | :--- | :--- |
| `native` | `i64` | the conversion refused it, malformed UTF-8 on windows; the native code |
| `alloc` | `A.Error` | the owned copy could not be acquired |

### native_text

```mach
pub fun native_text(a: *A.Allocator, text: str) res[NativeName, NativeTextError]
```

`text` in std.runtime.native units, owned and terminated: converted from
UTF-8 once on windows, the bytes as they are elsewhere

| Parameter | |
| :--- | :--- |
| `a` | allocator for the units, which native_text_dnit releases |
| `text` | the UTF-8 text, nil read as empty |
| **returns** | the units and their length, or the refusal |

### native_text_message

```mach
pub fun native_text_message(error: NativeTextError) str
```

the refusal of a native text conversion as text

## ProbeKind

```mach
pub def ProbeKind: u8
```

what a component is, read without following it

| Name | Value | |
| :--- | :--- | :--- |
| `PROBE_ABSENT` | `0` |  |
| `PROBE_FAILED` | `1` |  |
| `PROBE_LINK` | `2` |  |
| `PROBE_DIR` | `3` |  |
| `PROBE_FILE` | `4` |  |
| `PROBE_OTHER` | `5` |  |

## Probe

```mach
pub rec Probe {
    kind:  ProbeKind;
    cause: fail.Cause;
}
```

cause: what refused the probe, read only for PROBE_ABSENT and PROBE_FAILED

## Cursor

```mach
pub rec Cursor {
    alloc: *A.Allocator;
    root:  str;
    fd:    usize;
}
```

a contained walk's hold on a path: on posix a descriptor for the directory the next
component is read beneath, opened one component at a time; windows reads
each prefix by path, beneath the same root

### cursor_open

```mach
pub fun cursor_open(c: *Cursor, alloc: *A.Allocator, root: str, label: str) err[fail.Fail]
```

### cursor_close

```mach
pub fun cursor_close(c: *Cursor) err[fail.Fail]
```

### cursor_probe

```mach
pub fun cursor_probe(c: *Cursor, component: str, prefix: str) res[Probe, fail.Fail]
```

the component's kind; windows resolves the prefix against the root

### cursor_make

```mach
pub fun cursor_make(c: *Cursor, component: str, prefix: str) opt[fail.Cause]
```

a directory for a missing component; one a concurrent writer made first is as good

### cursor_unlink

```mach
pub fun cursor_unlink(c: *Cursor, component: str, prefix: str) opt[fail.Cause]
```

remove a stale non-directory component, never following it

### cursor_enter

```mach
pub fun cursor_enter(c: *Cursor, enter: str, invalid: str, label: str, rel: str, component: str, prefix: str) err[fail.Fail]
```

step beneath a component already read as a real directory, checking that
what was opened is still one
enter:   the refusal of a component that cannot be opened, formatted with label, rel, prefix and the native detail
invalid: the refusal of one that opened as something else, formatted with label, rel and prefix

### cursor_root

```mach
pub fun cursor_root(c: *Cursor) Probe
```

the root itself, which posix opened as a directory; windows reads it by path,
so a symlinked root is refused there as any component is

## Fault

```mach
pub rec Fault { kind: u8; message: str; }
```

### image_digest

```mach
pub fun image_digest(alloc: *A.Allocator, out: *u8) err[Fault]
```

the sha-256 of the running image's file, read through a handle checked to be
the file the process maps

## Functions

### terminate_command

```mach
pub fun terminate_command(pid: *u8, argv: **u8) *u8
```

the program that ends a process the host cannot signal directly, and its
arguments for the process named by pid, written to argv and ended by nil

| Parameter | |
| :--- | :--- |
| `pid` | the process id as decimal text |
| `argv` | room for four entries |
| **returns** | the program's path, which argv[0] also names |

### native_text_dnit

```mach
pub fun native_text_dnit(a: *A.Allocator, name: *NativeName)
```

release a native text; an empty one owns no units

### compare_env_names

```mach
pub fun compare_env_names(a: *A.Allocator, left: *Unit, left_length: usize, right: *Unit, right_length: usize) res[i32, fail.Fail]
```

compare two environment variable names in native units by the host's
identity, returning -1, 0 or 1, as std.process.env.compare_names_native
orders them: ordinal case-insensitive UTF-16 order on windows, byte order
elsewhere

| Parameter | |
| :--- | :--- |
| **returns** | the order, or the refusal of a name past the platform's length limit |

### program_resolve

```mach
pub fun program_resolve(a: *A.Allocator, name: str) res[str, fail.Fail]
```

locate an executable the way a shell would: a name with a path separator is used directly
and must be an existing executable file; any other name is searched on PATH

| Parameter | |
| :--- | :--- |
| `a` | allocator for the returned path |
| `name` | the program name or path |
| **returns** | the resolved path, owned by the caller; "PATH unset", "not found on PATH", or the direct-path error |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `OPEN_NOFOLLOW` | `i32` | `native_os.O_NOFOLLOW` where `$mach.build.sys == $mach.sys.linux \|\| $mach.build.sys == $mach.sys.darwin`<br>`0` where not `$mach.build.sys == $mach.sys.linux \|\| $mach.build.sys == $mach.sys.darwin` | the open flag that refuses a symlink at the leaf, and the one that keeps an open from blocking on a fifo; 0 where the host's open has neither |
| `OPEN_NONBLOCK` | `i32` | `native_os.O_NONBLOCK` where `$mach.build.sys == $mach.sys.linux \|\| $mach.build.sys == $mach.sys.darwin`<br>`0` where not `$mach.build.sys == $mach.sys.linux \|\| $mach.build.sys == $mach.sys.darwin` | the open flag that refuses a symlink at the leaf, and the one that keeps an open from blocking on a fifo; 0 where the host's open has neither |
| `PERMISSIONS` | `bool` | `false` where `$mach.build.sys == $mach.sys.windows`<br>`true` where not `$mach.build.sys == $mach.sys.windows` | whether the host keeps posix permission bits on a file |
| `FAULT_UNAVAILABLE` | `u8` | `1` | why the running image's identity could not be read: unavailable on this host or in this process, or an internal fault of the reader |
| `FAULT_INTERNAL` | `u8` | `2` |  |

