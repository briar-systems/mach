# std.print

## fun print

```mach
pub fun print(s: str) res[usize, WriteError];
```

write s to stdout

s: string to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun println

```mach
pub fun println(s: str) res[usize, WriteError];
```

write s followed by a newline to stdout

s: string to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun eprint

```mach
pub fun eprint(s: str) res[usize, WriteError];
```

write s to stderr

s: string to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun eprintln

```mach
pub fun eprintln(s: str) res[usize, WriteError];
```

write s followed by a newline to stderr

s: string to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun u64

```mach
pub fun u64(n: u64) res[usize, WriteError];
```

write unsigned 64-bit integer to stdout

n: value to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun eu64

```mach
pub fun eu64(n: u64) res[usize, WriteError];
```

write unsigned 64-bit integer to stderr

n: value to write
ret: bytes written, or the writer's failure with the persisted prefix

## fun printf

```mach
pub fun printf(fmt: str, va: ...) res[usize, FormatError];
```

formatted output to stdout

fmt: format string (see std.format.vformat for hole syntax)
va: comptime variadic argument pack
ret: bytes written, or the format's failure

## fun eprintf

```mach
pub fun eprintf(fmt: str, va: ...) res[usize, FormatError];
```

formatted output to stderr

fmt: format string (see std.format.vformat for hole syntax)
va: comptime variadic argument pack
ret: bytes written, or the format's failure

## fun printlnf

```mach
pub fun printlnf(fmt: str, va: ...) res[usize, FormatError];
```

formatted output to stdout followed by a newline

fmt: format string (see std.format.vformat for hole syntax)
va: comptime variadic argument pack
ret: bytes written, or the format's failure

## fun eprintlnf

```mach
pub fun eprintlnf(fmt: str, va: ...) res[usize, FormatError];
```

formatted output to stderr followed by a newline

fmt: format string (see std.format.vformat for hole syntax)
va: comptime variadic argument pack
ret: bytes written, or the format's failure

