[mach](../../README.md) / [type](../README.md) / [query](README.md) / **placement**

# mach.type.query.placement

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

<sub>source: [`src/type/query/placement.mach`](../../../../src/type/query/placement.mach) · imported by 2 modules</sub>

<sub>`type_query` is [`mach.type.query`](../query.md) · `map` is `std.collections.map` · `type_kind` is [`mach.type.kind`](../kind.md) · `Entry` is private · `Param` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Place`](#place) | `def` | one bit of a parameter's places |
| [`Places`](#places) | `rec` |  |

| On `Place` | |
| :--- | :--- |
| [`LAID`](#place) · [`LAID_SECRET`](#place) · [`POINTEE`](#place) · [`place_at`](#place_at) | |

| On `Places` | |
| :--- | :--- |
| [`init`](#init) · [`dnit`](#dnit) | |
| [`settle`](#settle) | settles the places of `generic` and every generic it reaches; false when they cannot be known, because a generic it reaches has no fields yet or the walk failed |
| [`bits`](#bits) | the places parameter `param` of a settled generic takes |
| [`reached`](#reached) | whether parameter `param` of a settled generic is reached, through storage or not |
| [`via`](#via) | the field of `generic` through which `param` first took place `place`, which it must take |

| Constants | |
| :--- | :--- |
| [`PLACE_COUNT`](#constants) | |

---

## Place

```mach
pub def Place: u8
```

one bit of a parameter's places

| Name | Value | |
| :--- | :--- | :--- |
| `LAID` | `1` |  |
| `LAID_SECRET` | `2` |  |
| `POINTEE` | `4` |  |

### place_at

```mach
pub fun place_at(i: u32) Place
```

## Places

```mach
pub rec Places {
    q:       *type_query.Query;
    index:   map.Map[type_kind.TypeId, u32];
    entries: Vector[Entry];
    params:  Vector[Param];
    # a generic met had no field table, so no answer is final
    incomplete: bool;
    grew:       bool;
}
```

### init

```mach
pub fun init(q: *type_query.Query) Places
```

### dnit

```mach
pub fun dnit(p: *Places)
```

### settle

```mach
pub fun settle(p: *Places, generic: type_kind.TypeId) bool
```

settles the places of `generic` and every generic it reaches; false when they cannot be
known, because a generic it reaches has no fields yet or the walk failed

### bits

```mach
pub fun bits(p: *Places, generic: type_kind.TypeId, param: u32) u8
```

the places parameter `param` of a settled generic takes

### reached

```mach
pub fun reached(p: *Places, generic: type_kind.TypeId, param: u32) bool
```

whether parameter `param` of a settled generic is reached, through storage or not

### via

```mach
pub fun via(p: *Places, generic: type_kind.TypeId, param: u32, place: Place) u32
```

the field of `generic` through which `param` first took place `place`, which it must take

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `PLACE_COUNT` | `u32` | `4` |  |

