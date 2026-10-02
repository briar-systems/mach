# mach.lang.alloc

the compiler's allocation layer: the std allocator surface under one import,
and the refusal rendered once. an allocation refusal is not something the
compiler recovers from at the site that met it; it is reported through the
failure domains (fail.refused, outcome.refused) and the build stops. the
text is presentation only and no site branches on it.

## fwd std_allocator.Allocator

```mach
fwd std_allocator.Allocator
```

forwards `std.allocator.Allocator`

## fwd std_allocator.Error

```mach
fwd std_allocator.Error
```

forwards `std.allocator.Error`

## fwd std_allocator.allocate

```mach
fwd std_allocator.allocate
```

forwards `std.allocator.allocate`

## fwd std_allocator.zallocate

```mach
fwd std_allocator.zallocate
```

forwards `std.allocator.zallocate`

## fwd std_allocator.reallocate

```mach
fwd std_allocator.reallocate
```

forwards `std.allocator.reallocate`

## fwd std_allocator.deallocate

```mach
fwd std_allocator.deallocate
```

forwards `std.allocator.deallocate`

## fun text

```mach
pub fun text(e: std_allocator.Error) str;
```

the refusal as text: `exhausted` is the message the compiler has always
reported, the other cases name the contract fault they are

