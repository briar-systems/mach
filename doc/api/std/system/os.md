# std.system.os

## fun secret_allocate

```mach
pub fun secret_allocate(size: usize) *^u8;
```

## fun secret_deallocate

```mach
pub fun secret_deallocate(data: *^u8, size: usize) i64;
```

## fun secret_allocate_typed

```mach
pub fun secret_allocate_typed[T](count: usize) *T;
```

## fun secret_deallocate_typed

```mach
pub fun secret_deallocate_typed[T](data: *T, count: usize) i64;
```

## fun secret_borrow_open

```mach
pub fun secret_borrow_open(data: *^u8, size: usize, out: *SecretBorrow) i64;
```

## fun secret_borrow_close

```mach
pub fun secret_borrow_close(borrow: SecretBorrow) i64;
```

## fun secret_borrow_size

```mach
pub fun secret_borrow_size(borrow: SecretBorrow) usize;
```

## fun secret_borrow_wipe

```mach
pub fun secret_borrow_wipe(borrow: SecretBorrow, offset: usize, len: usize) i64;
```

## fun secret_borrow_fill

```mach
pub fun secret_borrow_fill(borrow: SecretBorrow, offset: usize, source: *^u8, len: usize) i64;
```

## fun secret_borrow_drain

```mach
pub fun secret_borrow_drain(borrow: SecretBorrow, offset: usize, destination: *^u8, len: usize) i64;
```

## fun secret_borrow_copy

```mach
pub fun secret_borrow_copy(destination: SecretBorrow, destination_offset: usize, source: SecretBorrow, source_offset: usize, len: usize) i64;
```

## fun secret_borrow_read_at

```mach
pub fun secret_borrow_read_at(descriptor: i32, borrow: SecretBorrow, offset: usize, len: usize, file_offset: u64) i64;
```

## fun secret_borrow_write_at

```mach
pub fun secret_borrow_write_at(descriptor: i32, borrow: SecretBorrow, offset: usize, len: usize, file_offset: u64) i64;
```

## fun unlink_force

```mach
pub fun unlink_force(dirfd: i32, path: *u8, flags: i32) i64;
```

remove a rooted entry without changing read-only attributes on surviving aliases

## fun secret_random_fill

```mach
pub fun secret_random_fill(data: *^u8, len: usize) i64;
```

## fun temp_dir

```mach
pub fun temp_dir(buf: *u8, cap: usize) i64;
```

resolve the directory designated for temporary files

follows the env.get truncation contract: ret < cap means the path was
copied and null-terminated; ret >= cap signals truncation and ret is the
full path length. lookup order is per-OS:
  windows: GetTempPathA, TMP, then TEMP, then USERPROFILE, then the
           windows directory; the result ends with a path separator
  posix:   the $TMPDIR environment variable, falling back to "/tmp"

buf: destination buffer for the path
cap: buffer capacity in bytes
ret: full length of the path, or negative on error

## fun realtime

```mach
pub fun realtime(out: *Timespec) i64;
```

read the wall-clock time

out: pointer to Timespec to populate
ret: 0 on success, negative errno on failure

## fun monotonic

```mach
pub fun monotonic(out: *Timespec) i64;
```

read the monotonic clock

out: pointer to Timespec to populate
ret: 0 on success, negative errno on failure

## fun message

```mach
pub fun message(code: i64) str;
```

error

