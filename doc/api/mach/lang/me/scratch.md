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

## fun init_direct

```mach
pub fun init_direct(work: *Workspace, alloc: *A.Allocator) err[fail.Fail];
```

a workspace whose requests go straight to `alloc` and are never reclaimed by
`run`: the fail-at-N probes count every scratch request through it and hold
the transform to releasing each one itself

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

