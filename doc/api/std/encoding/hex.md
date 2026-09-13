# std.encoding.hex

## fun encoded_len

```mach
pub fun encoded_len(src_len: usize) usize;
```

return the buffer size required to encode src_len bytes (including null terminator)

src_len: number of source bytes
ret: required destination buffer size

## fun decoded_len

```mach
pub fun decoded_len(src_len: usize) usize;
```

return the maximum number of decoded bytes for a hex string of src_len chars

src_len: length of the hex string
ret: maximum decoded byte count

## fun encode

```mach
pub fun encode(src: *u8, src_len: usize, dst: *u8, dst_len: usize) usize;
```

encode bytes as lowercase hexadecimal

src: pointer to source bytes
src_len: number of source bytes
dst: pointer to destination buffer
dst_len: size of destination buffer (must be >= encoded_len(src_len))
ret: number of bytes written (not counting null terminator), 0 if dst too small

## fun encode_upper

```mach
pub fun encode_upper(src: *u8, src_len: usize, dst: *u8, dst_len: usize) usize;
```

encode bytes as uppercase hexadecimal

src: pointer to source bytes
src_len: number of source bytes
dst: pointer to destination buffer
dst_len: size of destination buffer (must be >= encoded_len(src_len))
ret: number of bytes written (not counting null terminator), 0 if dst too small

## fun decode

```mach
pub fun decode(src: *u8, src_len: usize, dst: *u8, dst_len: usize) usize;
```

decode a hexadecimal string to bytes

accepts both uppercase and lowercase hex characters.
returns 0 if the input has odd length or contains invalid characters.

src: pointer to hex string
src_len: length of the hex string
dst: pointer to destination buffer
dst_len: size of destination buffer (must be >= decoded_len(src_len))
ret: number of bytes written, 0 on invalid input or dst too small

