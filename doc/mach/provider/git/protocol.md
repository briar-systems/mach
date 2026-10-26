# mach.provider.git.protocol

bounded packet-line framing and Git delta reconstruction

## val MAX_PACKET

```mach
pub val MAX_PACKET: usize = 65520
```

## rec Packet

```mach
pub rec Packet;
```

## rec Reader

```mach
pub rec Reader;
```

## fun hex

```mach
pub fun hex(c: u8) i32;
```

## fun next

```mach
pub fun next(r: *Reader) res[Packet, str];
```

consume exactly one complete v0/v1 packet, rejecting reserved control lengths

## fun append_bytes

```mach
pub fun append_bytes(out: *Vector[u8], data: *u8, len: usize) err[str];
```

## fun append_packet

```mach
pub fun append_packet(out: *Vector[u8], data: *u8, len: usize) err[str];
```

append a packet transactionally: allocation failure restores the prior length

## fun append_flush

```mach
pub fun append_flush(out: *Vector[u8]) err[str];
```

## fun apply_delta

```mach
pub fun apply_delta(a: *A.Allocator, base: *u8, base_len: usize,
delta: *u8, delta_len: usize, limit: usize) res[Vector[u8], str];
```

reconstruct into a new owned vector, checking every source and output boundary

