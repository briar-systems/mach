# mach.lang.publication.testing

## rec Output

```mach
pub rec Output;
```

owns an output file and its private directory until dnit.

## fun create

```mach
pub fun create(alloc: *A.Allocator, prefix: str) res[Output, outcome.Fail];
```

## fun path

```mach
pub fun path(file: *Output) str;
```

borrows the output path until dnit.

## fun close

```mach
pub fun close(file: *Output) err[outcome.Fail];
```

## fun dnit

```mach
pub fun dnit(file: *Output) err[outcome.Fail];
```

## fun retain_bytes

```mach
pub fun retain_bytes(alloc: *A.Allocator, label: str, bytes: *u8, len: usize) err[outcome.Fail];
```

## fun control_free

```mach
pub fun control_free(alloc: *A.Allocator, dir: str) bool;
```

true when no entry beneath `dir`, walked recursively, is a transaction
control file: the lock sentinel or anything in the `.machtxn.` namespace

