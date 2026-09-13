# mach.lang.target.of.ar

## rec Member

```mach
pub rec Member;
```

## rec Step

```mach
pub rec Step;
```

## fun is_archive

```mach
pub fun is_archive(buf: *u8, len: usize) bool;
```

## fun first_member

```mach
pub fun first_member() usize;
```

## fun next_member

```mach
pub fun next_member(buf: *u8, len: usize, pos: usize) Step;
```

## fun is_special

```mach
pub fun is_special(member: Member) bool;
```

## rec ArMember

```mach
pub rec ArMember;
```

## fun write_archive

```mach
pub fun write_archive(alloc: *A.Allocator, members: *ArMember, count: u32, destination: *publication.Destination) err[fail.Fail];
```

