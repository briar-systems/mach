# mach.lang.me.scratch

## rec Function

```mach
pub rec Function;
```

## rec Workspace

```mach
pub rec Workspace;
```

## fun init

```mach
pub fun init(work: *Workspace) err[fail.Fail];
```

## fun dnit

```mach
pub fun dnit(work: *Workspace);
```

## fun run

```mach
pub fun run[C, T](work: *Workspace, owner: *A.Allocator, context: *C, action: fun(*C, *A.Allocator) res[T, fail.Fail]) res[T, fail.Fail];
```

successful results must not borrow workspace storage

## fun run_unit

```mach
pub fun run_unit[C](work: *Workspace, owner: *A.Allocator, context: *C, action: fun(*C, *A.Allocator) err[fail.Fail]) err[fail.Fail];
```

the same for an action with no value

