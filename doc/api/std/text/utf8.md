# std.text.utf8

## def Codepoint

```mach
pub def Codepoint: u32
```

## val REPLACEMENT

```mach
pub val REPLACEMENT:   Codepoint = 0xFFFD
```

## val MAX_CODEPOINT

```mach
pub val MAX_CODEPOINT: Codepoint = 0x10FFFF
```

## fun byte_len

```mach
pub fun byte_len(first_byte: u8) usize;
```

return the sequence length from a UTF-8 leading byte

first_byte: the first byte of a UTF-8 sequence
ret: 1-4 for valid leading bytes, 0 for invalid

## fun codepoint_len

```mach
pub fun codepoint_len(cp: Codepoint) usize;
```

return how many bytes are needed to encode a codepoint

cp: the codepoint to measure
ret: 1-4 for valid codepoints, 0 if > U+10FFFF

## fun is_continuation

```mach
pub fun is_continuation(b: u8) bool;
```

true if the byte is a UTF-8 continuation byte (0x80-0xBF)

b: byte to test
ret: true if continuation byte

## fun encode

```mach
pub fun encode(cp: Codepoint, buf: *u8, len: usize) usize;
```

encode a codepoint into a byte buffer

cp: codepoint to encode
buf: destination buffer
len: size of buf in bytes
ret: bytes written (0 if buf too small or invalid codepoint)

## fun decode

```mach
pub fun decode(buf: *u8, len: usize, cp: *Codepoint) usize;
```

decode one codepoint from a byte buffer

buf: source buffer
len: number of bytes available
cp: pointer to write the decoded codepoint
ret: bytes consumed (0 if invalid or empty; sets *cp to REPLACEMENT on error)

## fun validate

```mach
pub fun validate(buf: *u8, len: usize) bool;
```

check if a byte buffer contains valid UTF-8

buf: source buffer
len: number of bytes
ret: true if the entire buffer is valid UTF-8

## fun count

```mach
pub fun count(buf: *u8, len: usize) usize;
```

count the number of codepoints in a UTF-8 byte buffer

buf: source buffer
len: number of bytes
ret: number of codepoints (counts sequence starts, not bytes)

