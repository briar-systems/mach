[mach](../README.md) / [compile](README.md) / **demand**

# mach.compile.demand

the instances one module's code reaches, typed and reported. the module asks for the instances
its own code calls, and every instance for those its body calls; its demand is the closure of the
first through the second, each instance typed by itself, once per build. nothing the demand holds
depends on the order instances were typed in: the closure of a set is the same however it is
walked, a report its body places goes out once, in the order of the instances' symbols, and a
report placed nowhere goes out at the module's first call site leading there

Source: [`src/compile/demand.mach`](../../../src/compile/demand.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.front.sema.instance`](../front/sema/instance.md), [`mach.front.sema.product`](../front/sema/product.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Registry`](#registry) | `rec` | every instance a build has met, by its symbol: its identity as whoever asked for it last described it, which the instance's typing reads |
| [`GrowthWatch`](#growthwatch) | `rec` | who hears of a growing cycle's expansion: the module that declares it, the templates on it, and how many instances of it the build has met so far |
| [`Typer`](#typer) | `rec` | how the demand types an instance: the typing of the instance a symbol and its description name |
| [`Entry`](#entry) | `rec` | one instance the closure reached |
| [`Demand`](#demand) | `rec` | the closure, in the order it was met |

| On `Registry` | |
| :--- | :--- |
| [`registry_init`](#registry_init) | a registry holding nothing yet: its set is made on the first instance it keeps |
| [`registry_dnit`](#registry_dnit) | free everything the registry holds |
| [`registered`](#registered) | the symbol of the instance `item` describes, named in `s`'s interner, which a typing registered before: a lowering reads the registry and never writes it, since every instance it reaches was registered by the typing that decided its module's instances, so it names each as that typing did |
| [`described`](#described) | the instance the symbol `name` names, none when nothing described it |

| On `Demand` | |
| :--- | :--- |
| [`init`](#init) | an empty demand |
| [`dnit`](#dnit) | free the demand's own storage, never the typings its entries point to |
| [`offer_module`](#offer_module) | every instance the module's `set` asks for, where its code asks for it |
| [`close`](#close) | the closure: every entry met, in the order met, typed, and every instance its body asks for met in turn |
| [`report`](#report) | each instance's reports, from its typing |

---

## Registry

```mach
pub rec Registry {
    set:    mach.front.sema.instance.Set;
    names:  Map[mach.base.intern.StrId, u32];
    growth: Map[u64, u32];
}
```

every instance a build has met, by its symbol: its identity as whoever asked for it last
described it, which the instance's typing reads. it lives as long as the project, since an
instance is typed again in a later operation of the build when what it reads moved

| Field | Type | |
| :--- | :--- | :--- |
| `growth` | `Map[u64, u32]` | how many instances of each growing cycle the build has met, keyed by the cycle's module and members |

### registry_init

```mach
pub fun registry_init(a: *std.allocator.Allocator) Registry
```

a registry holding nothing yet: its set is made on the first instance it keeps

### registry_dnit

```mach
pub fun registry_dnit(r: *Registry)
```

free everything the registry holds

| Parameter | |
| :--- | :--- |
| `r` | registry to free, unusable after |

### registered

```mach
pub fun registered(r: *Registry, s: *mach.session.Session, item: *mach.front.sema.instance.Instance) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol of the instance `item` describes, named in `s`'s interner, which a typing registered before: a
lowering reads the registry and never writes it, since every instance it reaches was registered by the
typing that decided its module's instances, so it names each as that typing did. an instance no typing
registered is a failure of the lowering

| Parameter | |
| :--- | :--- |
| `r` | registry the typings filled, read only |
| `s` | session as the lowering sees it, whose interner names the symbol |
| `item` | instance the lowering reached |
| **returns** | the instance's symbol, or a failure when no typing registered it |

### described

```mach
pub fun described(r: *Registry, name: mach.base.intern.StrId) opt[*mach.front.sema.instance.Instance]
```

the instance the symbol `name` names, none when nothing described it

## GrowthWatch

```mach
pub rec GrowthWatch {
    ctx:      ptr;
    expanded: fun(ptr, mach.base.intern.StrId, mach.base.intern.StrId, u32);
}
```

who hears of a growing cycle's expansion: the module that declares it, the templates on it,
and how many instances of it the build has met so far. a nil `expanded` hears nothing

## Typer

```mach
pub rec Typer {
    ctx:   ptr;
    typed: fun(ptr, mach.base.intern.StrId, *mach.front.sema.instance.Instance) res[*mach.front.sema.product.InstanceResult, mach.base.fail.Fail];
}
```

how the demand types an instance: the typing of the instance a symbol and its description name

## Entry

```mach
pub rec Entry {
    name:  mach.base.intern.StrId;
    text:  str;
    typed: *mach.front.sema.product.InstanceResult;
}
```

one instance the closure reached

| Field | Type | |
| :--- | :--- | :--- |
| `typed` | `*mach.front.sema.product.InstanceResult` | its typing this build, nil when it could not be typed |

## Demand

```mach
pub rec Demand {
    entries:  Vector[Entry];
    edges:    Vector[Edge];
    asks:     Vector[Ask];
    index:    Map[mach.base.intern.StrId, u32];
    rejected: bool;
}
```

the closure, in the order it was met

| Field | Type | |
| :--- | :--- | :--- |
| `rejected` | `bool` | an instance in the closure does not type check, or could not be typed |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Demand
```

an empty demand

| Parameter | |
| :--- | :--- |
| `a` | allocator the demand's entries, edges, asks and index grow in |
| **returns** | the demand, which the caller frees with dnit |

### dnit

```mach
pub fun dnit(d: *Demand)
```

free the demand's own storage, never the typings its entries point to

| Parameter | |
| :--- | :--- |
| `d` | demand to free, unusable after |

### offer_module

```mach
pub fun offer_module(d: *Demand, r: *Registry, s: *mach.session.Session, set: *mach.front.sema.instance.Set, watch: GrowthWatch) err[mach.base.fail.Fail]
```

every instance the module's `set` asks for, where its code asks for it

### close

```mach
pub fun close(d: *Demand, r: *Registry, s: *mach.session.Session, t: Typer, watch: GrowthWatch) err[mach.base.fail.Fail]
```

the closure: every entry met, in the order met, typed, and every instance its body asks for met
in turn. an instance that does not type check rejects the demand, and one that could not be
typed, its declaring module rejected, asks for nothing more

### report

```mach
pub fun report(d: *Demand, s: *mach.session.Session, into: *mach.base.diagnostic.DiagnosticStore) err[mach.base.fail.Fail]
```

each instance's reports, from its typing. a report its body places itself goes out once, in the
order of the instances' symbols. a report placed nowhere goes out at the first call site by
offset whose closure reaches its instance, directly or through other instances, and at one site
instances go by their symbols, so the order is the sources' alone

