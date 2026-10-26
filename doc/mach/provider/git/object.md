# mach.provider.git.object

public Git object IDs; SHA-1 is used only for Git format compatibility

## rec Id

```mach
pub rec Id;
```

## fun parse_id

```mach
pub fun parse_id(data: *u8, len: usize) res[Id, str];
```

## fun format_id

```mach
pub fun format_id(id: *Id, out: *u8) bool;
```

write a lowercase, nul-terminated ID to caller storage (at least 65 bytes)

## fun equal

```mach
pub fun equal(a: *Id, b: *Id) bool;
```

## fun digest

```mach
pub fun digest(data: *u8, len: usize, hash_len: usize) res[Id, str];
```

## fun object_id

```mach
pub fun object_id(kind: str, data: *u8, len: usize, hash_len: usize) res[Id, str];
```

hash the canonical type, decimal length, nul and payload

