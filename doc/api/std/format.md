# std.format

## tag FormatError

```mach
pub tag FormatError: u8 {
    syntax:     usize;
    few_holes:  usize;
    many_holes: usize;
    write:      WriteError;
    alloc:      A.Error;
}
```

every way a format can fail beyond the writer's own outcome

syntax: the format string is malformed; payload is the byte offset of
            the offending brace or spec
few_holes: more arguments than holes; payload is the offset where the scan
            ended with an argument still unrendered
many_holes: more holes than arguments; payload is the offset of the hole
            no argument fills
write: the sink failed; payload carries the persisted prefix and cause
alloc: sprint's result buffer was refused; nothing was written

## fun write_bytes

```mach
pub fun write_bytes(w: *writer.Writer, buf: *u8, len: usize) res[usize, WriteError];
```

write raw bytes to a writer

w: target writer
buf: source buffer
len: number of bytes to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_str

```mach
pub fun write_str(w: *writer.Writer, s: str) res[usize, WriteError];
```

write a null-terminated string to a writer

w: target writer
s: string to write (nil treated as empty)
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_byte

```mach
pub fun write_byte(w: *writer.Writer, b: u8) res[usize, WriteError];
```

write a single byte to a writer

w: target writer
b: byte to write
ret: 1 on success, or the writer's failure

## fun write_newline

```mach
pub fun write_newline(w: *writer.Writer) res[usize, WriteError];
```

write a linefeed to a writer

w: target writer
ret: 1 on success, or the writer's failure

## fun write_u64

```mach
pub fun write_u64(w: *writer.Writer, n: u64) res[usize, WriteError];
```

write an unsigned integer in decimal

w: target writer
n: value to format
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_i64

```mach
pub fun write_i64(w: *writer.Writer, n: i64) res[usize, WriteError];
```

write a signed integer in decimal

w: target writer
n: value to format
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_hex_u64

```mach
pub fun write_hex_u64(w: *writer.Writer, n: u64, upper: bool) res[usize, WriteError];
```

write a 64-bit value in hexadecimal

w: target writer
n: value to format
upper: true for uppercase (A-F), false for lowercase (a-f)
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_ptr

```mach
pub fun write_ptr(w: *writer.Writer, p: ptr) res[usize, WriteError];
```

write a pointer as 0x followed by hex digits

w: target writer
p: pointer to format
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_f64

```mach
pub fun write_f64(w: *writer.Writer, n: f64) res[usize, WriteError];
```

write a 64-bit float in shortest round-trippable decimal

w: target writer
n: value to format
ret: bytes written, or the writer's failure with the persisted prefix

## fun write_value

```mach
pub fun write_value(w: *writer.Writer, va: ...) res[usize, WriteError];
```

write values in their default rendering, the `{}` of vformat without a
format string

the same type-directed dispatch as vformat's holes: signed/unsigned integers
in decimal, str as text, ptr as 0x-hex, f64 and f32 as the shortest
round-trippable decimal. each value of the pack is rendered in order with
nothing between them; the common call passes one. a type vformat would
refuse at runtime is refused here at instantiation, since the pack's types
are known.

w: target writer
va: the values to render
ret: bytes written, or the writer's failure with the persisted prefix

## fun vformat

```mach
pub fun vformat(w: *writer.Writer, fmt: str, va: ...) res[usize, FormatError];
```

format a comptime-variadic pack into a writer (v1.7 packs)

holes are type-directed `{}` matched to args in order: signed/unsigned int ->
decimal, str -> text, ptr -> 0xHEX, f64 -> shortest round-trip decimal. specs:
`{:x}`/`{:X}` hex, `{:c}` low-byte char, `{:N}`/`{:0N}` minimum width (right-
align), `{:<N}` left-align. `{{` and
`}}` are literal-brace escapes. the arg sequence is comptime-unrolled and
monomorphized per call; the format scan stays at runtime.

w: target writer
fmt: format string
va: comptime variadic argument pack
ret: total bytes written, or the failure; a write failure carries the bytes
     persisted before it

## fun format

```mach
pub fun format(w: *writer.Writer, fmt: str, va: ...) res[usize, FormatError];
```

format a comptime-variadic pack into a writer

thin wrapper that forwards the whole pack to vformat.

w: target writer
fmt: format string
va: comptime variadic argument pack
ret: total bytes written, or the failure

## fun sprint

```mach
pub fun sprint(a: *A.Allocator, fmt: str, va: ...) res[str, FormatError];
```

format a comptime-variadic pack into a freshly allocated string

the allocating counterpart to `format`/`vformat`: renders the same `{}` holes
with identical type-directed dispatch and spec vocabulary, but returns a new
str owned by `a` rather than writing to a Writer. the result is an exact-size,
null-terminated allocation (str_len + 1 bytes), matching the std.types.string
allocating constructors and released with text.string.str_free. formats
twice: a measuring pass over vformat sizes the buffer, then a filling pass
renders into it, so no growth or trimming is needed. a format error is
reported before any allocation.

a: allocator for the result
fmt: format string
va: comptime variadic argument pack
ret: the formatted string, or the format's failure, or alloc when the
     allocator refuses the buffer

