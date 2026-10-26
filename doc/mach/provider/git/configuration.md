# mach.provider.git.configuration

the Git configuration subset used for repository and dependency metadata

## rec Pair

```mach
pub rec Pair;
```

## rec Config

```mach
pub rec Config;
```

## fun dnit

```mach
pub fun dnit(a: *A.Allocator, c: *Config);
```

## fun copy

```mach
pub fun copy(a: *A.Allocator, text: str) res[str, str];
```

## fun parse

```mach
pub fun parse(a: *A.Allocator, data: *u8, len: usize) res[Config, str];
```

## fun name

```mach
pub fun name(a: *A.Allocator, p: *Pair) res[str, str];
```

## fun get

```mach
pub fun get(a: *A.Allocator, c: *Config, key: str, all: bool) res[str, str];
```

## fun quoted

```mach
pub fun quoted(out: *Vector[u8], text: str) err[str];
```

## fun encode

```mach
pub fun encode(a: *A.Allocator, c: *Config) res[Vector[u8], str];
```

