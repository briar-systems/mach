[mach](../README.md) / [compile](README.md) / **surface**

# mach.compile.surface

what of a module's source text a reader of it depends on, as the spans a walk of its syntax
omits. an importer's object holds none of the module's plain function bodies: its code calls
them, and an instance's code it holds is held to it by its record, under the instance's key, so
an importer reads the text without any function or test body. an instance reads its declaring module's template text: the text without the
bodies of tests and of functions that are not templates (generic, comptime-parameter or pack,
growth.fun_is_template), since the instance is that body typed. the same walk also finds a
module's test declarations, which only its test object compiles, so its normal object is keyed
without them

Source: [`src/compile/surface.mach`](../../../src/compile/surface.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.front.ast`](../front/ast.md).

| Functions | |
| :--- | :--- |
| [`bodies`](#bodies) | every function and test body of `a`, in source order: what its importers do not read |
| [`plain_bodies`](#plain_bodies) | the bodies of `a`'s tests and of its functions that are not templates, in source order: what an instance of one of its templates does not read |
| [`tests`](#tests) | the test declarations of `a`, whole, in source order |
| [`digest`](#digest) | the digest of the text without `spans` |

---

## Functions

### bodies

```mach
pub fun bodies(a: *mach.front.ast.Ast, text: str, out: *Vector[mach.base.source.Span]) err[mach.base.fail.Fail]
```

every function and test body of `a`, in source order: what its importers do not read

### plain_bodies

```mach
pub fun plain_bodies(a: *mach.front.ast.Ast, text: str, out: *Vector[mach.base.source.Span]) err[mach.base.fail.Fail]
```

the bodies of `a`'s tests and of its functions that are not templates, in source order: what
an instance of one of its templates does not read

### tests

```mach
pub fun tests(a: *mach.front.ast.Ast, text: str, out: *Vector[mach.base.source.Span]) err[mach.base.fail.Fail]
```

the test declarations of `a`, whole, in source order

### digest

```mach
pub fun digest(text: str, spans: *Vector[mach.base.source.Span], placed: bool, out: *[32]u8)
```

the digest of the text without `spans`. `placed` also takes each omitted span's line count and
the length of its last line, so the digest moves whenever retained text moves to another line
or column: debug information an object generates from a retained body names where it is

