# mach.provider.git.remote

smart HTTP v0/v1 reference discovery and self-contained upload-pack requests

## val MAX_REFS

```mach
pub val MAX_REFS:      usize = 100000
```

## val MAX_DISCOVERY

```mach
pub val MAX_DISCOVERY: usize = 16777216
```

## rec Ref

```mach
pub rec Ref;
```

name borrows the advertisement's input bytes

## rec Advertisement

```mach
pub rec Advertisement;
```

## fun valid_ref

```mach
pub fun valid_ref(data: *u8, len: usize) bool;
```

## fun discover

```mach
pub fun discover(a: *A.Allocator, data: *u8, len: usize) res[Advertisement, str];
```

## fun select

```mach
pub fun select(ad: *Advertisement, selector: str) res[object.Id, str];
```

## fun request

```mach
pub fun request(a: *A.Allocator, ad: *Advertisement, wanted: *object.Id) res[Vector[u8], str];
```

request a complete object closure: no have lines, depth, filters or thin-pack

## fun response

```mach
pub fun response(a: *A.Allocator, data: *u8, len: usize, sideband: bool, limit: usize) res[Vector[u8], str];
```

## fun validate_url

```mach
pub fun validate_url(text: str) err[str];
```

validate the public HTTPS URL before any transport or filesystem operation

