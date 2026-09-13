# mach.lang.build.cache.image

## val MAX_ENTRY_BYTES

```mach
pub val MAX_ENTRY_BYTES: usize = 67108864
```

## val MALFORMED

```mach
pub val MALFORMED:       u8    = 1
```

## val INTERNAL

```mach
pub val INTERNAL:        u8    = 2
```

## val LIMIT

```mach
pub val LIMIT:           u8    = 3
```

## rec Error

```mach
pub rec Error;
```

## rec TestFact

```mach
pub rec TestFact;
```

what the engine reads from a module's lowered ir besides its object: the
scalarization count and the test declarations. they travel with the object so
a restored module never has to lower

## rec Facts

```mach
pub rec Facts;
```

## rec Product

```mach
pub rec Product;
```

## fun facts_dnit

```mach
pub fun facts_dnit(alloc: *A.Allocator, f: *Facts);
```

## fun product_dnit

```mach
pub fun product_dnit(p: *Product);
```

## fun encode

```mach
pub fun encode(alloc: *A.Allocator, image: *of.ObjectImage, facts: *Facts) res[vector.Vector[u8], Error];
```

## fun decode

```mach
pub fun decode(alloc: *A.Allocator, itn: *intern.Interner, bytes: *u8, len: usize) res[Product, Error];
```

final name transfer may retain canonical caller-interner names on allocation refusal.

