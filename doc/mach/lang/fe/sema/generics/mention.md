# mach.lang.fe.sema.generics.mention

## rec Memo

```mach
pub rec Memo;
```

whether each nominal's fields mention a type parameter, as decided under one
type projection; a lookup under a later projection finds the memo empty

## fun init

```mach
pub fun init(a: *A.Allocator) Memo;
```

## fun dnit

```mach
pub fun dnit(m: *Memo);
```

## fun get

```mach
pub fun get(m: *Memo, generation: u64, tid: type.TypeId) opt[bool];
```

## fun set

```mach
pub fun set(m: *Memo, generation: u64, tid: type.TypeId, mentions: bool) err[fail.Fail];
```

