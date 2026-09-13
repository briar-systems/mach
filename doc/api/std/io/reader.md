# std.io.reader

## rec ReadFailure

```mach
pub rec ReadFailure;
```

a native failure and the bytes delivered before it

## tag ReadError

```mach
pub tag ReadError: u8 {
    eof:         usize;
    would_block: usize;
    native:      ReadFailure;
    alloc:       A.Error;
}
```

every way a read can fail

eof: the stream ended; payload is the bytes delivered before the end
would_block: the source stalled (EAGAIN); payload is the bytes delivered first
native: the source failed; payload carries the failure and the bytes
             delivered before it
alloc: growth was refused (read_all, read_str); nothing is delivered
             and the caller's allocator is exactly as it was

## def ReadFun

```mach
pub def ReadFun: fun(ptr, *u8, usize) res[usize, ReadError]
```

read callback type

ptr: opaque context passed to the read callback
buf: buffer to read into
len: maximum number of bytes to read, never zero
ret: bytes delivered (zero at the end of the stream) or the failure

## rec Reader

```mach
pub rec Reader;
```

generic read interface

ctx: opaque context passed to the read callback
f_read: read callback (ctx, buf, len) -> res[usize, ReadError]

## fun native_failure

```mach
pub fun native_failure(delivered: usize, error: io_error.Error) ReadError;
```

## fun advance

```mach
pub fun advance(error: ReadError, delivered: usize) ReadError;
```

add the bytes delivered by earlier calls to the error's progress

## fun read

```mach
pub fun read(r: *Reader, buf: *u8, len: usize) res[usize, ReadError];
```

read up to len bytes into buf

r: reader to read from
buf: buffer to read into
len: maximum number of bytes to read
ret: bytes delivered, or eof, would_block or the native failure; ok{0}
     only for a zero-length request

## fun read_exact

```mach
pub fun read_exact(r: *Reader, buf: *u8, len: usize) err[ReadError];
```

read exactly len bytes into buf

r: reader to read from
buf: buffer to fill
len: number of bytes to read
ret: the failure with the bytes delivered before it, or ok

## fun read_all

```mach
pub fun read_all(a: *A.Allocator, r: *Reader) res[Vector[u8], ReadError];
```

read until EOF into a fresh vector

the caller owns the returned vector. on failure nothing is returned and the
vector's storage has already been released; the error carries the bytes the
source delivered before it failed.

a: allocator to use
r: reader to read from
ret: the content, or the failure

## fun read_str

```mach
pub fun read_str(a: *A.Allocator, r: *Reader) res[OwnedString, ReadError];
```

read until EOF and return the content as an owned, null-terminated string

the vector's buffer is terminated in place and adopted whole: the string's
extent is the buffer's capacity and owned_release returns it all at once.

a: allocator to use
r: reader to read from
ret: the content as an owned string, or the failure

