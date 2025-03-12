- [Mach Memory Management](#mach-memory-management)
  - [References](#references)
  - [Access](#access)


# Mach Memory Management

Memory management in Mach, while inspired by languages like C, is quite unique in its representation.

## References

A reference in mach uses the `@` symbol before a type to signify that it is a reference type.
Note that a reference type is effectively identical to a pointer in languages like C on the back end of things.

Example reference type:
```mach
var foo: u32  = 1     # u32 with a value of 1
var bar: @u32 = &foo  # reference to a u32 from the address of foo
var baz: u32  = ?bar  # dereference bar and assign the value to baz
```

foo = 2  # also changes bar
bar = 3  # also changes foob
baz = 4  # does not change foo or bar
```

Because of the nature in which typecasting and references work, more complicated syntax can be used when necessary for highly granular access to memory:
```mach
var foo: @u32 = 0xDEADBEEF::@u32  # reference to a u32 at location 0xDEADBEEF
vol foo: @u32 = 0xDEADBEEF::@u32  # volatile reference to a u32 at location 0xDEADBEEF
```


## Access

Accessing memory stored at an offset from a reference location is done with `<>`.

Here is an example of some advanced memory manipulation:
```mach
var foo: @u32 = 0xDEADBEEF::@u32  # reference to a u32 at location 0xDEADBEEF
var bar: @u32 = foo<2>            # reference to a u32 at location 0xDEADBEEF + (size_of(u32) * 2 * 8)
var baz: u32  = ?bar              # dereference bar and assign the value to baz
#               this can also be written as &foo<2>

foo<2> = 0xF00F::u32  # write to the same memory location that `bar` is pointing to
```
