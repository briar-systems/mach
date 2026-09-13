# std.sync.mutex

## def Mutex

```mach
pub def Mutex: i64
```

## fun make

```mach
pub fun make() Mutex;
```

mutexes are unfair and may admit a new caller before a woken waiter
waits may wake spuriously, so acquisition always rechecks the state
mutexes are not poisoned when a protected operation fails
mutexes own no target handle and may be discarded only without waiters

## fun lock

```mach
pub fun lock(m: *Mutex);
```

## fun unlock

```mach
pub fun unlock(m: *Mutex);
```

## fun try_lock

```mach
pub fun try_lock(m: *Mutex) bool;
```

## fun is_locked

```mach
pub fun is_locked(m: *Mutex) bool;
```

this diagnostic result is stale as soon as it is returned

