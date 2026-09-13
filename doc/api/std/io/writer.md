# std.io.writer

## rec WriteFailure

```mach
pub rec WriteFailure;
```

a native failure and the bytes persisted before it

## tag WriteError

```mach
pub tag WriteError: u8 {
    stalled:     usize;
    would_block: usize;
    native:      WriteFailure;
}
```

every way a write can fail

stalled: the sink accepted nothing; payload is the persisted prefix
would_block: the sink stalled natively (EAGAIN); payload is the prefix
native: the sink failed; payload carries the failure and the prefix

## def WriteFun

```mach
pub def WriteFun: fun(ptr, *u8, usize) res[usize, WriteError]
```

write callback type

ptr: opaque context passed to the write callback
buf: buffer to write from
len: number of bytes to write, never zero
ret: bytes accepted (zero when the sink stalls) or the failure

## rec Writer

```mach
pub rec Writer;
```

generic write interface

ctx: opaque context passed to the write callback
f_write: write callback (ctx, buf, len) -> res[usize, WriteError]

## fun native_failure

```mach
pub fun native_failure(written: usize, error: io_error.Error) WriteError;
```

## fun persisted

```mach
pub fun persisted(error: WriteError) usize;
```

the bytes the sink persisted before the failure

## fun advance

```mach
pub fun advance(error: WriteError, written: usize) WriteError;
```

add the bytes persisted by earlier calls to the error's prefix

## fun write

```mach
pub fun write(w: *Writer, buf: *u8, len: usize) res[usize, WriteError];
```

write up to len bytes from buf

w: writer to write to
buf: buffer to write from
len: maximum number of bytes to write
ret: bytes accepted, or stalled, would_block or the native failure; ok{0}
     only for a zero-length request

## fun write_all

```mach
pub fun write_all(w: *Writer, buf: *u8, len: usize) err[WriteError];
```

write exactly len bytes from buf

w: writer to write to
buf: buffer to write from
len: number of bytes to write
ret: the failure with the persisted prefix, or ok

## fun write_str

```mach
pub fun write_str(w: *Writer, s: str) err[WriteError];
```

write a null-terminated string

w: writer to write to
s: string to write
ret: the failure with the persisted prefix, or ok

