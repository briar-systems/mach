# mach.lang.driver.surface

what of a module's source text a reader of it depends on, as the spans a walk of its syntax
omits. an importer's object holds none of the module's function bodies: its code calls them,
and an instance's code lives in its unit, so an importer reads the text without any function
or test body. an instance reads its declaring module's template text: the text without the
bodies of tests and of functions that are not templates (generic, comptime-parameter or pack,
growth.fun_is_template), since the instance is that body typed. the same walk also finds a
module's test declarations, which only its test object compiles, so its normal object is keyed
without them

## fun bodies

```mach
pub fun bodies(a: *ast.Ast, text: str, out: *Vector[lang_source.Span]) err[fail.Fail];
```

every function and test body of `a`, in source order: what its importers do not read

## fun plain_bodies

```mach
pub fun plain_bodies(a: *ast.Ast, text: str, out: *Vector[lang_source.Span]) err[fail.Fail];
```

the bodies of `a`'s tests and of its functions that are not templates, in source order: what
an instance of one of its templates does not read

## fun tests

```mach
pub fun tests(a: *ast.Ast, text: str, out: *Vector[lang_source.Span]) err[fail.Fail];
```

the test declarations of `a`, whole, in source order

## fun digest

```mach
pub fun digest(text: str, spans: *Vector[lang_source.Span], placed: bool, out: *[32]u8);
```

the digest of the text without `spans`. `placed` also takes each omitted span's line count and
the length of its last line, so the digest moves whenever retained text moves to another line
or column: debug information an object generates from a retained body names where it is

