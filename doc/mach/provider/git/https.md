# mach.provider.git.https

synchronous dependency requests over Mach HTTP and authenticated Mach TLS

## val MAX_BODY

```mach
pub val MAX_BODY: usize = 536870912
```

## fun exchange

```mach
pub fun exchange(a: *A.Allocator, text: str, post: bool, body: *u8, body_len: usize,
expected_type: str, limit: usize) res[Vector[u8], str];
```

one HTTP/1.1 exchange, with bounded framing and no transparent content encoding

