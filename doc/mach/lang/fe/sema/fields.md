# mach.lang.fe.sema.fields

## rec Node

```mach
pub rec Node;
```

## rec Roots

```mach
pub rec Roots;
```

## def CaptureMode

```mach
pub def CaptureMode: u8
```

## val RECIPES

```mach
pub val RECIPES:      CaptureMode = 0
```

## val MATERIALIZED

```mach
pub val MATERIALIZED: CaptureMode = 1
```

## rec Graph

```mach
pub rec Graph;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Graph;
```

## fun dnit

```mach
pub fun dnit(graph: *Graph);
```

## fun capture

```mach
pub fun capture[T](s: *session.Session, a: *A.Allocator, roots: *Roots, count: usize, mode: CaptureMode, defs: *isa.TargetDefs, ctx: *T,
prepare: fun(*T, type.TypeId) err[fail.Fail]) res[Graph, fail.Fail];
```

## fun install

```mach
pub fun install(s: *session.Session, graph: *Graph) err[fail.Fail];
```

