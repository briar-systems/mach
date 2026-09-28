# mach.lang.alloc

the compiler's allocation layer: the std allocator surface under one import,
and the refusal rendered once. an allocation refusal is not something the
compiler recovers from at the site that met it; it is reported through the
failure domains (fail.refused, outcome.refused) and the build stops. the
text is presentation only and no site branches on it.

## fwd A.Allocator

```mach
fwd A.Allocator
```

forwards `std.allocator.Allocator`

## fwd A.Error

```mach
fwd A.Error
```

forwards `std.allocator.Error`

## fwd A.allocate

```mach
fwd A.allocate
```

forwards `std.allocator.allocate`

## fwd A.zallocate

```mach
fwd A.zallocate
```

forwards `std.allocator.zallocate`

## fwd A.reallocate

```mach
fwd A.reallocate
```

forwards `std.allocator.reallocate`

## fwd A.deallocate

```mach
fwd A.deallocate
```

forwards `std.allocator.deallocate`

## fun text

```mach
pub fun text(e: A.Error) str;
```

the refusal as text: `exhausted` is the message the compiler has always
reported, the other cases name the contract fault they are

