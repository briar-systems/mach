# std.io.handle

## rec FileHandle

```mach
pub rec FileHandle;
```

file handle owned by the filesystem layer

## rec SocketHandle

```mach
pub rec SocketHandle;
```

socket handle owned by the networking layer

## val INVALID_VALUE

```mach
pub val INVALID_VALUE: usize = (-1)::isize::usize
```

## fun invalid_file

```mach
pub fun invalid_file() FileHandle;
```

## fun invalid_socket

```mach
pub fun invalid_socket() SocketHandle;
```

## fun file_is_valid

```mach
pub fun file_is_valid(handle: FileHandle) bool;
```

## fun socket_is_valid

```mach
pub fun socket_is_valid(handle: SocketHandle) bool;
```

