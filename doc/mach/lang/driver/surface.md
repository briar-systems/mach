# mach.lang.driver.surface

## fun omitted

```mach
pub fun omitted(a: *ast.Ast, text: str, out: *Vector[token.Span]) err[fail.Fail];
```

the omitted bodies of `a`, in source order

## fun tests

```mach
pub fun tests(a: *ast.Ast, text: str, out: *Vector[token.Span]) err[fail.Fail];
```

the test declarations of `a`, whole, in source order

## fun write

```mach
pub fun write(fb: *dq.FpBuf, text: str, spans: *Vector[token.Span]) err[fail.Fail];
```

the surface's bytes: the text with every omitted span removed

## fun digest

```mach
pub fun digest(text: str, spans: *Vector[token.Span], placed: bool, out: *[32]u8);
```

the digest of the surface. `placed` also takes each omitted span's line
count and the length of its last line, so the digest moves whenever a
retained declaration moves to another line or column: debug information an
importer generates from a retained body names where that body is

