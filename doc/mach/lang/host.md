# mach.lang.host

the host the compiler runs on: every primitive whose spelling differs by the
operating system the compiler itself was built for. a compiler module calls
these and holds no host conditional of its own. the target a build produces
is the target module's concern, not this one's

## val OPEN_NOFOLLOW

```mach
pub val OPEN_NOFOLLOW: i32 = native_os.O_NOFOLLOW
```

## val OPEN_NONBLOCK

```mach
pub val OPEN_NONBLOCK: i32 = native_os.O_NONBLOCK
```

## val OPEN_NOFOLLOW

```mach
pub val OPEN_NOFOLLOW: i32 = 0
```

## val OPEN_NONBLOCK

```mach
pub val OPEN_NONBLOCK: i32 = 0
```

## val PERMISSIONS

```mach
pub val PERMISSIONS: bool = false
```

## val PERMISSIONS

```mach
pub val PERMISSIONS: bool = true
```

## fun terminate_command

```mach
pub fun terminate_command(pid: *u8, argv: **u8) *u8;
```

the program that ends a process the host cannot signal directly, and its
arguments for the process named by pid, written to argv and ended by nil

pid: the process id as decimal text
argv: room for four entries
ret: the program's path, which argv[0] also names

## tag NativeTextError

```mach
pub tag NativeTextError: u8 {
    native: i64;
    alloc:  A.Error;
}
```

why text could not be put in native units

native: the conversion refused it, malformed UTF-8 on windows; the native code
alloc: the owned copy could not be acquired

## fun native_text

```mach
pub fun native_text(a: *A.Allocator, text: str) res[NativeName, NativeTextError];
```

`text` in std.runtime.native units, owned and terminated: converted from
UTF-8 once on windows, the bytes as they are elsewhere

a: allocator for the units, which native_text_free releases
text: the UTF-8 text, nil read as empty
ret: the units and their length, or the refusal

## fun native_text_free

```mach
pub fun native_text_free(a: *A.Allocator, name: *NativeName);
```

## fun native_text_message

```mach
pub fun native_text_message(error: NativeTextError) str;
```

the refusal of a native text conversion as text

## fun compare_env_names

```mach
pub fun compare_env_names(left: *Unit, left_length: usize, right: *Unit, right_length: usize) res[i32, fail.Fail];
```

compare two environment variable names in native units by the host's
identity, returning -1, 0 or 1, as std.process.env.compare_names_native
orders them: ordinal case-insensitive UTF-16 order on windows, byte order
elsewhere

ret: the order, or the refusal of a name past the platform's length limit

## fun program_resolve

```mach
pub fun program_resolve(a: *A.Allocator, name: str) res[str, fail.Fail];
```

locate an executable the way a shell would: a name with a path separator is used directly
and must be an existing executable file; any other name is searched on PATH

a: allocator for the returned path
name: the program name or path
ret: the resolved path, owned by the caller; "PATH unset", "not found on PATH", or the
      direct-path error

## def ProbeKind

```mach
pub def ProbeKind: u8
```

what a component is, read without following it

## val PROBE_ABSENT

```mach
pub val PROBE_ABSENT: ProbeKind = 0
```

## val PROBE_FAILED

```mach
pub val PROBE_FAILED: ProbeKind = 1
```

## val PROBE_LINK

```mach
pub val PROBE_LINK:   ProbeKind = 2
```

## val PROBE_DIR

```mach
pub val PROBE_DIR:    ProbeKind = 3
```

## val PROBE_FILE

```mach
pub val PROBE_FILE:   ProbeKind = 4
```

## val PROBE_OTHER

```mach
pub val PROBE_OTHER:  ProbeKind = 5
```

## rec Probe

```mach
pub rec Probe;
```

## rec Cursor

```mach
pub rec Cursor;
```

a contained walk's hold on a path: on posix a descriptor for the directory the next
component is read beneath, opened one component at a time; windows reads
each prefix by path, beneath the same root

## fun cursor_open

```mach
pub fun cursor_open(c: *Cursor, alloc: *A.Allocator, root: str, label: str) err[fail.Fail];
```

## fun cursor_close

```mach
pub fun cursor_close(c: *Cursor) err[fail.Fail];
```

## fun cursor_probe

```mach
pub fun cursor_probe(c: *Cursor, component: str, prefix: str) res[Probe, fail.Fail];
```

the component's kind; windows resolves the prefix against the root

## fun cursor_make

```mach
pub fun cursor_make(c: *Cursor, component: str, prefix: str) opt[str];
```

a directory for a missing component; one a concurrent writer made first is as good

## fun cursor_unlink

```mach
pub fun cursor_unlink(c: *Cursor, component: str, prefix: str) opt[str];
```

remove a stale non-directory component, never following it

## fun cursor_enter

```mach
pub fun cursor_enter(c: *Cursor, enter: str, invalid: str, label: str, rel: str, component: str, prefix: str) err[fail.Fail];
```

step beneath a component already read as a real directory, checking that
what was opened is still one
enter:   the refusal of a component that cannot be opened, formatted with label, rel, prefix and the native detail
invalid: the refusal of one that opened as something else, formatted with label, rel and prefix

## fun cursor_root

```mach
pub fun cursor_root(c: *Cursor) Probe;
```

the root itself, which posix opened as a directory; windows reads it by path,
so a symlinked root is refused there as any component is

## val FAULT_UNAVAILABLE

```mach
pub val FAULT_UNAVAILABLE: u8 = 1
```

why the running image's identity could not be read: unavailable on this host
or in this process, or an internal fault of the reader

## val FAULT_INTERNAL

```mach
pub val FAULT_INTERNAL:    u8 = 2
```

## rec Fault

```mach
pub rec Fault;
```

## fun image_digest

```mach
pub fun image_digest(alloc: *A.Allocator, out: *u8) err[Fault];
```

the sha-256 of the running image's file, read through a handle checked to be
the file the process maps

## fun image_build_id

```mach
pub fun image_build_id(out: *u8) usize;
```

the build id the linker wrote into the running image, read from its mapped
headers; 0 when the image carries none (a compiler linked by a seed that
predates the note) or the host has no reader

