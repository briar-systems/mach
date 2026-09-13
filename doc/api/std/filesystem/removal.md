# std.filesystem.removal

## rec Error

```mach
pub rec Error;
```

## fun message

```mach
pub fun message(e: Error) str;
```

## fun with_cleanup

```mach
pub fun with_cleanup(result: err[Error], code: i64) err[Error];
```

keep the primary failure when releasing a cursor or descriptor also fails

## fun tree

```mach
pub fun tree(dirfd: i32, name: str, removed: *usize, max_depth: usize) err[Error];
```

count is usable only when the complete operation succeeds

## fun private_tree

```mach
pub fun private_tree(dirfd: i32, name: str, removed: *usize, max_depth: usize) err[Error];
```

only for transaction-owned private trees under an exclusive cooperative root claim

