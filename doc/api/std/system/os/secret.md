# std.system.os.secret

## fun allocate

```mach
pub fun allocate(size: usize) *^u8;
```

allocate one zero-initialized secret-welded byte region

size: number of addressable bytes
ret: logical span owner, or nil for zero size or failure

## fun deallocate

```mach
pub fun deallocate(data: *^u8, size: usize) i64;
```

release one secret-welded byte region

data: allocation returned by allocate
size: original nonzero allocation size
ret: zero after wipe and release, or a negative error with zeroed ownership retained

## fun allocate_typed

```mach
pub fun allocate_typed[T](count: usize) *T;
```

allocate a zero-initialized typed region without changing T's secrecy shape

count: number of T values
ret: aligned typed owner, or nil for zero count, overflow, or failure

## fun deallocate_typed

```mach
pub fun deallocate_typed[T](data: *T, count: usize) i64;
```

wipe and release a typed region without changing T's secrecy shape

data: allocation returned by allocate_typed[T]
count: original element count
ret: zero after release, or a negative error with wiped ownership retained

## fun random_fill

```mach
pub fun random_fill(data: *^u8, len: usize) i64;
```

initialize every requested byte from the operating-system csprng

data: destination secret storage
len: number of bytes to initialize
ret: zero after complete initialization, or a negative error after a complete wipe

## rec Borrow

```mach
pub rec Borrow;
```

## fun borrow_open

```mach
pub fun borrow_open(data: *^u8, size: usize, out: *Borrow) i64;
```

lend welded storage to a handle

data: the storage, which stays owned by and valid for the caller until close
size: nonzero span length
out: the handle
ret: zero, EFAULT for nil or empty storage, ENOMEM when the table cannot grow

## fun borrow_close

```mach
pub fun borrow_close(borrow: Borrow) i64;
```

return a borrow; the storage is neither wiped nor released, it was never owned

ret: zero, or EBADF for a handle that is not live

## fun borrow_size

```mach
pub fun borrow_size(borrow: Borrow) usize;
```

the span length of a live borrow, or zero

## fun borrow_wipe

```mach
pub fun borrow_wipe(borrow: Borrow, offset: usize, len: usize) i64;
```

zero a span of a borrow

ret: zero, EBADF for a dead handle, EINVAL for a span outside the borrow

## fun borrow_fill

```mach
pub fun borrow_fill(borrow: Borrow, offset: usize, source: *^u8, len: usize) i64;
```

copy welded bytes into a span of a borrow

source: welded storage the caller can read
ret: zero, EBADF, EINVAL for a span outside the borrow, EFAULT for a nil source

## fun borrow_drain

```mach
pub fun borrow_drain(borrow: Borrow, offset: usize, destination: *^u8, len: usize) i64;
```

copy welded bytes out of a span of a borrow

destination: welded storage the caller owns
ret: zero, EBADF, EINVAL for a span outside the borrow, EFAULT for a nil destination

## fun borrow_copy

```mach
pub fun borrow_copy(destination: Borrow, destination_offset: usize, source: Borrow, source_offset: usize, len: usize) i64;
```

copy between two borrows without either pointer leaving the boundary

ret: zero, EBADF for a dead handle, EINVAL for a span outside either borrow

## fun borrow_read_at

```mach
pub fun borrow_read_at(descriptor: i32, borrow: Borrow, offset: usize, len: usize, file_offset: u64) i64;
```

read from a descriptor into a span of a borrow: the kernel writes the welded
storage through this call and nothing else

ret: bytes delivered (zero at end of file), or a negative error with the span unspecified

## fun borrow_write_at

```mach
pub fun borrow_write_at(descriptor: i32, borrow: Borrow, offset: usize, len: usize, file_offset: u64) i64;
```

write a span of a borrow to a descriptor: the kernel reads the welded storage
through this call and nothing else

ret: bytes persisted, or a negative error with nothing about the span changed

