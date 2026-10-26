# mach.provider.git.pack

checked pack decoding with bounded zlib streams and delta reconstruction

## val MAX_OBJECT

```mach
pub val MAX_OBJECT:      usize = 67108864
```

## val MAX_TOTAL

```mach
pub val MAX_TOTAL:       usize = 536870912
```

## val MAX_OBJECTS

```mach
pub val MAX_OBJECTS:     usize = 1000000
```

## val MAX_DELTA_DEPTH

```mach
pub val MAX_DELTA_DEPTH: usize = 64
```

## rec Object

```mach
pub rec Object;
```

## rec Pack

```mach
pub rec Pack;
```

## fun dnit

```mach
pub fun dnit(p: *Pack);
```

## fun kind_name

```mach
pub fun kind_name(kind: u8) str;
```

## fun read_be32

```mach
pub fun read_be32(data: *u8) u32;
```

## fun find

```mach
pub fun find(p: *Pack, id: *object.Id) *Object;
```

## fun decode

```mach
pub fun decode(a: *A.Allocator, data: *u8, len: usize, hash_len: usize) res[Pack, str];
```

decode a self-contained pack; upload-pack requests must not negotiate thin-pack

## rec IndexedPack

```mach
pub rec IndexedPack;
```

## fun open_indexed

```mach
pub fun open_indexed(data: *u8, len: usize,
index_data: *u8, index_len: usize, h: usize) res[IndexedPack, str];
```

verify immutable pack and index buffers once before repeated object reads

## fun read_opened

```mach
pub fun read_opened(a: *A.Allocator, p: *IndexedPack, id: *object.Id) res[Object, str];
```

the owner keeps both verified buffers immutable for this view's entire lifetime

## fun read_indexed

```mach
pub fun read_indexed(a: *A.Allocator, data: *u8, len: usize, index_data: *u8, index_len: usize, id: *object.Id) res[Object, str];
```

