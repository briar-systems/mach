# mach.lang.build.cache.key

## rec Builder

```mach
pub rec Builder;
```

## fun begin

```mach
pub fun begin(domain: str) Builder;
```

## fun number

```mach
pub fun number(b: *Builder, value: u64);
```

## fun bytes

```mach
pub fun bytes(b: *Builder, data: *u8, len: usize);
```

## fun text

```mach
pub fun text(b: *Builder, value: str);
```

## fun name

```mach
pub fun name(b: *Builder, itn: *intern.Interner, id: intern.StrId);
```

## fun finish

```mach
pub fun finish(b: *Builder, digest: *[32]u8) err[fail.Fail];
```

## fun equal

```mach
pub fun equal(left: *[32]u8, right: *[32]u8) bool;
```

## fun object

```mach
pub fun object(snapshot: *[32]u8, module: str, out: *[32]u8) err[fail.Fail];
```

## fun test_object

```mach
pub fun test_object(object_key: *[32]u8, source: str, out: *[32]u8) err[fail.Fail];
```

the key of a module's test object: the key of the normal object it builds on,
and the module's source, which holds the test bodies the normal key need not cover

## fun step

```mach
pub fun step(prior: *[32]u8, owner: str, name: str, inputs: *[32]u8) err[fail.Fail];
```

