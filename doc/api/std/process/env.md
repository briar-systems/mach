# std.process.env

## tag EnvError

```mach
pub tag EnvError: u8 {
    native: i64;
    alloc:  A.Error;
    changed;
}
```

why an environment read failed

native: the platform refused the read; the negative errno
alloc: the owned copy could not be acquired
changed: the variable changed between the probe and the exact read

## fun compare_names

```mach
pub fun compare_names(left: str, right: str) res[i32, EnvError];
```

compare complete variable names, returning -1, 0, or 1
unix uses byte order, windows uses ordinal case-insensitive UTF-16 order
invalid UTF-8 on windows or allocation failure is the native error

## fun get

```mach
pub fun get(name: str, buf: *u8, cap: usize) res[opt[usize], EnvError];
```

read an environment variable into a buffer

when the value fits (length < cap) it is copied and null-terminated.
the full value length is always reported, so length >= cap signals
truncation (buffer contents unspecified) and length + 1 is the capacity
to retry with. an unset variable is absence, not a failure.

name: variable name
buf: destination buffer
cap: buffer capacity in bytes
ret: the full length of the value excluding the terminator, absent when
      the variable is unset, or the native failure

## fun value

```mach
pub fun value(a: *A.Allocator, name: str) res[opt[OwnedString], EnvError];
```

read an environment variable into a freshly allocated string

probes with a small stack buffer and retries once with an exact allocation
when the value is larger, so the common case costs one read and values of
any length resolve correctly. the retry is bounded: a variable that is
unset or longer at the second read is reported as `changed` rather than
read a third time.

a: allocator for the result
name: variable name
ret: the owned value (extent is the length plus the terminator), absent
      when the variable is unset, or the failure

## fun current_dir

```mach
pub fun current_dir(a: *A.Allocator) res[OwnedString, EnvError];
```

read the current working directory into a freshly allocated string

probes with a stack buffer and retries once with an exact allocation when
the path is longer; a path that grew again between the reads is `changed`.

a: allocator for the result
ret: the owned working directory path, or the native or allocation failure

## fun environ

```mach
pub fun environ() **u8;
```

the inherited environment captured at startup on unix or first use on windows
windows omits hidden drive-current-directory entries

ret: null-terminated array of "NAME=value" strings, or nil if the
     environment is unavailable (e.g. before runtime init)

