# mach.lang.type.query.placement

where each parameter of a generic sits in the storage its instances lay out, answered
over the declarations so a generic that grows through a pointer still has a finite answer

a parameter is laid when its argument's bytes sit inside an aggregate beside other
storage, so its size and alignment shape the layout, and it is a pointee when its
argument is the whole target of a pointer, through arrays only. either may sit under `^`.
a parameter that only flows into parameters that sit nowhere reaches no byte at all, so
two instances that differ only there lay out and classify the same storage. the places
are a least fixpoint over the declaration graph: every set starts empty and only grows,
so passes repeat until one adds nothing. beside its places, a parameter is reached when its
argument appears anywhere an instance's fields lead, an empty array's element included,
which is what a question about what a type contains reads

## def Place

```mach
pub def Place: u8
```

one bit of a parameter's places

## val LAID

```mach
pub val LAID:           Place = 1
```

## val LAID_SECRET

```mach
pub val LAID_SECRET:    Place = 2
```

## val POINTEE

```mach
pub val POINTEE:        Place = 4
```

## val PLACE_COUNT

```mach
pub val PLACE_COUNT: u32 = 4
```

## fun place_at

```mach
pub fun place_at(i: u32) Place;
```

## rec Places

```mach
pub rec Places;
```

## fun init

```mach
pub fun init(q: *type_query.Query) Places;
```

## fun dnit

```mach
pub fun dnit(p: *Places);
```

## fun settle

```mach
pub fun settle(p: *Places, generic: type_kind.TypeId) bool;
```

settles the places of `generic` and every generic it reaches; false when they cannot be
known, because a generic it reaches has no fields yet or the walk failed

## fun bits

```mach
pub fun bits(p: *Places, generic: type_kind.TypeId, param: u32) u8;
```

the places parameter `param` of a settled generic takes

## fun reached

```mach
pub fun reached(p: *Places, generic: type_kind.TypeId, param: u32) bool;
```

whether parameter `param` of a settled generic is reached, through storage or not

## fun via

```mach
pub fun via(p: *Places, generic: type_kind.TypeId, param: u32, place: Place) u32;
```

the field of `generic` through which `param` first took place `place`, which it must take

