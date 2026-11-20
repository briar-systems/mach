[mach](../README.md) / [front](README.md) / **refusal**

# mach.front.refusal

a module refused by a module-scope `$error`. a module in which one fires is refused and
reports each that fired and nothing else, and a module that imports a refused module,
directly or through others, is not analysed and reports nothing

Source: [`src/front/refusal.mach`](../../../src/front/refusal.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`State`](#state) | `def` |  |
| [`Refusal`](#refusal) | `rec` | why a module is refused, if it is |

| On `State` | |
| :--- | :--- |
| [`NONE`](#state) | |
| [`OWN`](#state) | a module-scope `$error` fired in the module |
| [`IMPORTED`](#state) | a module it imports is refused |

| On `Refusal` | |
| :--- | :--- |
| [`init`](#init) | an unrefused module's state, with an empty site list |
| [`dnit`](#dnit) | free the site list and return the state to NONE |
| [`is_refused`](#is_refused) | whether the module is refused for any reason |
| [`fired`](#fired) | a module-scope `$error` that fired at `offset` |
| [`copy`](#copy) | an owned copy of `from`, from `a` |

---

## State

```mach
pub def State: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` |  |
| `OWN` | `1` | a module-scope `$error` fired in the module |
| `IMPORTED` | `2` | a module it imports is refused |

## Refusal

```mach
pub rec Refusal {
    state: State;
    sites: std.collections.vector.Vector[u32];
}
```

why a module is refused, if it is

| Field | Type | |
| :--- | :--- | :--- |
| `state` | `State` | NONE, OWN or IMPORTED |
| `sites` | `std.collections.vector.Vector[u32]` | where each module-scope `$error` that fired begins in the module's file, for OWN |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Refusal
```

an unrefused module's state, with an empty site list

| Parameter | |
| :--- | :--- |
| `a` | allocator the site list grows from |
| **returns** | a refusal in state NONE, released with `dnit` |

### dnit

```mach
pub fun dnit(r: *Refusal)
```

free the site list and return the state to NONE

| Parameter | |
| :--- | :--- |
| `r` | refusal to release |

### is_refused

```mach
pub fun is_refused(r: *Refusal) bool
```

whether the module is refused for any reason

| Parameter | |
| :--- | :--- |
| `r` | refusal to test |
| **returns** | true when the state is OWN or IMPORTED |

### fired

```mach
pub fun fired(r: *Refusal, offset: usize) err[mach.base.fail.Fail]
```

a module-scope `$error` that fired at `offset`

### copy

```mach
pub fun copy(from: *Refusal, a: *std.allocator.Allocator) res[Refusal, mach.base.fail.Fail]
```

an owned copy of `from`, from `a`

