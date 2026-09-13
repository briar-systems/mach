# std.encoding.binary

## def Order

```mach
pub def Order: u8
```

byte order for multi-byte values

## val LITTLE

```mach
pub val LITTLE: Order = 0
```

## val BIG

```mach
pub val BIG:    Order = 1
```

## def Checkpoint

```mach
pub def Checkpoint: usize
```

a logical position captured for explicit rollback

## tag EncodeError

```mach
pub tag EncodeError: u8 {
    alloc: A.Error;
    overflow;
    invalid;
    full:      usize;
    alignment: usize;
    reserved;
    bounds;
    inactive;
    stale;
    busy;
}
```

every way a write, patch, alignment or reservation can fail

alloc: the encoder's growth was refused; the encoder is unchanged
overflow: a length or epoch would not fit usize
invalid: the encoder or builder is inconsistent (length past capacity,
           storage missing for a nonzero width, a nil source for nonzero
           bytes)
full: the fixed builder lacks room; payload is the bytes available
alignment: the boundary is not a power of two; payload is the boundary
reserved: the builder is locked by an active reservation
bounds: a patch or checkpoint lies outside the published bytes
inactive: the reservation was already resolved, or never made
stale: the reservation no longer matches its parent
busy: the reservation's child still holds a reservation of its own

## tag DecodeError

```mach
pub tag DecodeError: u8 {
    short: usize;
    invalid;
    alignment: usize;
    bounds;
    unterminated;
}
```

every way a read, skip, alignment or checkpoint can fail

short: the request exceeds the bytes remaining; payload is the
              remaining count
invalid: the cursor is inconsistent (position past the end, storage
              missing for pending bytes)
alignment: the boundary is not a power of two; payload is the boundary
bounds: a checkpoint lies outside the cursor's past
unterminated: no null terminator before the end

## fun put_uint

```mach
pub fun put_uint(dst: *u8, v: u64, sz: usize, order: Order);
```

write the low sz bytes of v into dst in the given order

dst: destination for sz bytes
v: value whose low sz bytes are written
sz: number of bytes to write (1..8)
order: byte order

## fun get_uint

```mach
pub fun get_uint(src: *u8, sz: usize, order: Order) u64;
```

read sz bytes from src in the given order into a u64

src: source of sz bytes
sz: number of bytes to read (1..8)
order: byte order
ret: the assembled value in the low sz bytes

## rec Encoder

```mach
pub rec Encoder;
```

growable byte sequence with a fixed byte order

alloc: allocator for buffer memory
order: byte order for multi-byte writes
data: backing byte array
len: bytes written
cap: allocated capacity

## fun encoder

```mach
pub fun encoder(a: *A.Allocator, order: Order) Encoder;
```

create an empty encoder

a: allocator for buffer memory
order: byte order for multi-byte writes
ret: a new empty Encoder

## fun encoder_dnit

```mach
pub fun encoder_dnit(e: *Encoder) err[A.Error];
```

release the encoder's buffer and reset it

an encoder that owns no buffer is reset without asking the allocator. a
refused release leaves the encoder owning its buffer.

e: encoder to deinitialize
ret: ok, or the allocator's refusal

## fun reserve

```mach
pub fun reserve(e: *Encoder, additional: usize) res[usize, EncodeError];
```

ensure capacity for `additional` more bytes

e: encoder to reserve in
additional: number of additional bytes needed
ret: new capacity, or the failure; a refused growth leaves the
            encoder as it was

## fun write_u8

```mach
pub fun write_u8(e: *Encoder, v: u8) res[usize, EncodeError];
```

append a u8

e: encoder to write into
v: value to append
ret: new length, or the failure

## fun write_u16

```mach
pub fun write_u16(e: *Encoder, v: u16) res[usize, EncodeError];
```

append a u16 in the encoder's byte order

e: encoder to write into
v: value to append
ret: new length, or the failure

## fun write_u32

```mach
pub fun write_u32(e: *Encoder, v: u32) res[usize, EncodeError];
```

append a u32 in the encoder's byte order

e: encoder to write into
v: value to append
ret: new length, or the failure

## fun write_u64

```mach
pub fun write_u64(e: *Encoder, v: u64) res[usize, EncodeError];
```

append a u64 in the encoder's byte order

e: encoder to write into
v: value to append
ret: new length, or the failure

## fun write_bytes

```mach
pub fun write_bytes(e: *Encoder, p: *u8, len: usize) res[usize, EncodeError];
```

append raw bytes verbatim

e: encoder to write into
p: source bytes
len: number of bytes
ret: new length, or the failure

## fun write_str

```mach
pub fun write_str(e: *Encoder, s: str) res[usize, EncodeError];
```

append a string and its null terminator

e: encoder to write into
s: string to append
ret: new length, or the failure

## fun align

```mach
pub fun align(e: *Encoder, boundary: usize) res[usize, EncodeError];
```

pad with zero bytes up to an alignment boundary

e: encoder to align
boundary: alignment boundary (a power of two)
ret: new length, or the failure

## fun patch_u8

```mach
pub fun patch_u8(e: *Encoder, offset: usize, v: u8) res[usize, EncodeError];
```

overwrite a u8 at an earlier offset without changing length

## fun patch_u16

```mach
pub fun patch_u16(e: *Encoder, offset: usize, v: u16) res[usize, EncodeError];
```

overwrite a u16 at an earlier offset in the encoder's byte order

e: encoder to patch
offset: byte offset to write at
v: value to write
ret: bytes written, or the failure

## fun patch_u32

```mach
pub fun patch_u32(e: *Encoder, offset: usize, v: u32) res[usize, EncodeError];
```

overwrite a u32 at an earlier offset in the encoder's byte order

e: encoder to patch
offset: byte offset to write at
v: value to write
ret: bytes written, or the failure

## fun patch_u64

```mach
pub fun patch_u64(e: *Encoder, offset: usize, v: u64) res[usize, EncodeError];
```

overwrite a u64 at an earlier offset in the encoder's byte order

e: encoder to patch
offset: byte offset to write at
v: value to write
ret: bytes written, or the failure

## rec Builder

```mach
pub rec Builder;
```

fixed-capacity byte builder with a transactional reservation lock

order: byte order for multi-byte writes
data: caller-owned backing bytes
len: logically published bytes
cap: backing capacity
reserved: whether a child reservation owns the unpublished tail
epoch: monotonically increasing reservation identity

## rec Reservation

```mach
pub rec Reservation;
```

unpublished child builder over a reserved parent tail

parent: parent whose logical length remains unchanged until commit
body: fixed child builder bounded to the reservation
start: parent's logical length when the reservation began
limit: reserved byte capacity
epoch: identity used to reject stale reservation handles
active: whether the reservation can still commit or roll back

an active reservation and its parent must stay at stable addresses. resolve
nested reservations before their parents and do not copy active handles.

## fun builder

```mach
pub fun builder(data: *u8, cap: usize, order: Order) Builder;
```

wrap caller-owned storage as an empty fixed builder

## fun builder_remaining

```mach
pub fun builder_remaining(b: *Builder) usize;
```

bytes available after the builder's published prefix

## fun builder_write_u8

```mach
pub fun builder_write_u8(b: *Builder, v: u8) res[usize, EncodeError];
```

append a u8 atomically

## fun builder_write_u16

```mach
pub fun builder_write_u16(b: *Builder, v: u16) res[usize, EncodeError];
```

append a u16 in the builder's byte order atomically

## fun builder_write_u32

```mach
pub fun builder_write_u32(b: *Builder, v: u32) res[usize, EncodeError];
```

append a u32 in the builder's byte order atomically

## fun builder_write_u64

```mach
pub fun builder_write_u64(b: *Builder, v: u64) res[usize, EncodeError];
```

append a u64 in the builder's byte order atomically

## fun builder_write_bytes

```mach
pub fun builder_write_bytes(b: *Builder, data: *u8, len: usize) res[usize, EncodeError];
```

append raw bytes atomically

## fun builder_write_str

```mach
pub fun builder_write_str(b: *Builder, s: str) res[usize, EncodeError];
```

append a string and null terminator atomically

## fun builder_align

```mach
pub fun builder_align(b: *Builder, boundary: usize) res[usize, EncodeError];
```

append zero padding to a power-of-two boundary atomically

## fun builder_patch_u8

```mach
pub fun builder_patch_u8(b: *Builder, offset: usize, v: u8) res[usize, EncodeError];
```

patch one published byte without changing length

## fun builder_patch_u16

```mach
pub fun builder_patch_u16(b: *Builder, offset: usize, v: u16) res[usize, EncodeError];
```

patch a published u16 without changing length

## fun builder_patch_u32

```mach
pub fun builder_patch_u32(b: *Builder, offset: usize, v: u32) res[usize, EncodeError];
```

patch a published u32 without changing length

## fun builder_patch_u64

```mach
pub fun builder_patch_u64(b: *Builder, offset: usize, v: u64) res[usize, EncodeError];
```

patch a published u64 without changing length

## fun builder_checkpoint

```mach
pub fun builder_checkpoint(b: *Builder) res[Checkpoint, EncodeError];
```

capture the current fixed-builder length for explicit rollback

## fun builder_rollback

```mach
pub fun builder_rollback(b: *Builder, mark: Checkpoint) res[usize, EncodeError];
```

restore a prior builder checkpoint without changing backing bytes

## fun builder_reserve

```mach
pub fun builder_reserve(b: *Builder, capacity: usize) res[Reservation, EncodeError];
```

lock capacity for a child field without publishing parent length

## fun builder_commit

```mach
pub fun builder_commit(r: *Reservation) res[usize, EncodeError];
```

publish the completed prefix of a reservation as one logical field

## fun reservation_rollback

```mach
pub fun reservation_rollback(r: *Reservation) res[usize, EncodeError];
```

discard an unpublished reservation and unlock its parent

## rec Decoder

```mach
pub rec Decoder;
```

cursor over existing bytes with a fixed byte order

order: byte order for multi-byte reads
data: byte data
len: total length in bytes
pos: current read position

## def Cursor

```mach
pub def Cursor: Decoder
```

checked borrowed cursor, retained as an alias of Decoder for compatibility

## fun decoder

```mach
pub fun decoder(data: *u8, len: usize, order: Order) Decoder;
```

wrap existing bytes as a decoder positioned at offset 0

data: byte data
len: length in bytes
order: byte order for multi-byte reads
ret: a new Decoder

## fun cursor

```mach
pub fun cursor(data: *u8, len: usize, order: Order) Cursor;
```

wrap existing bytes as a checked cursor positioned at offset 0

## fun read_u8

```mach
pub fun read_u8(d: *Decoder) res[u8, DecodeError];
```

read a u8 and advance

d: decoder to read from
ret: the value, or the failure

## fun read_u16

```mach
pub fun read_u16(d: *Decoder) res[u16, DecodeError];
```

read a u16 in the decoder's byte order and advance

d: decoder to read from
ret: the value, or the failure

## fun read_u32

```mach
pub fun read_u32(d: *Decoder) res[u32, DecodeError];
```

read a u32 in the decoder's byte order and advance

d: decoder to read from
ret: the value, or the failure

## fun read_u64

```mach
pub fun read_u64(d: *Decoder) res[u64, DecodeError];
```

read a u64 in the decoder's byte order and advance

d: decoder to read from
ret: the value, or the failure

## fun read_bytes

```mach
pub fun read_bytes(d: *Decoder, len: usize) res[*u8, DecodeError];
```

return a pointer to the next `len` bytes and advance past them

d: decoder to read from
len: number of bytes to consume
ret: pointer into the decoder's data (nil for a zero-length request), or
     the failure

## fun read_str

```mach
pub fun read_str(d: *Decoder) res[str, DecodeError];
```

read a null-terminated string and advance past the terminator

d: decoder to read from
ret: pointer to the string within the data, or the failure

## fun skip

```mach
pub fun skip(d: *Decoder, n: usize) res[usize, DecodeError];
```

advance the cursor by n bytes

d: decoder to advance
n: number of bytes to skip
ret: new position, or the failure

## fun align_read

```mach
pub fun align_read(d: *Decoder, boundary: usize) res[usize, DecodeError];
```

advance the cursor to an alignment boundary

d: decoder to align
boundary: alignment boundary (a power of two)
ret: new position, or the failure

## fun read_subview

```mach
pub fun read_subview(d: *Decoder, len: usize) res[Cursor, DecodeError];
```

return a bounded child cursor and advance only after validation

## fun cursor_checkpoint

```mach
pub fun cursor_checkpoint(d: *Decoder) res[Checkpoint, DecodeError];
```

capture a cursor position for speculative decoding

## fun cursor_rollback

```mach
pub fun cursor_rollback(d: *Decoder, mark: Checkpoint) res[usize, DecodeError];
```

restore an earlier cursor checkpoint

## fun remaining

```mach
pub fun remaining(d: *Decoder) usize;
```

bytes remaining between the cursor and the end

d: decoder to query
ret: unread byte count

