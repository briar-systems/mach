# mach.provider.git.native

adapter for the compiler's existing dependency operations; never spawns a process

## rec State

```mach
pub rec State;
```

## fun init

```mach
pub fun init(a: *A.Allocator) State;
```

## fun clear

```mach
pub fun clear(s: *State);
```

## fun dnit

```mach
pub fun dnit(s: *State);
```

## fun run

```mach
pub fun run(s: *State, directory: str, given: *str, count: usize) res[Vector[u8], str];
```

translate only the operations issued by the dependency driver, not a Git CLI

## fun clone_selection

```mach
pub fun clone_selection(s: *State, directory: str, url: str, name: str, selector: str) err[str];
```

the driver knows the selector before acquisition, so a fresh checkout need not
fetch unrelated branch tips to obtain a release tag

## fun add_dependency

```mach
pub fun add_dependency(s: *State, root: str, rel: str, url: str, selector: str, offline: bool) err[str];
```

## fun fetch_selection

```mach
pub fun fetch_selection(s: *State, directory: str, selector: str) err[str];
```

## fun ensure_pin_tag

```mach
pub fun ensure_pin_tag(s: *State, directory: str, pin: str) err[str];
```

